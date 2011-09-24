#include <HALOptions.h>
#include <NewtonScript.h>
#include "UserTasks.h"
#include "Logger.h"

#define MAX_MESSAGE 128

class SimpleTask: public TUTaskWorld
{
public:
	TUPort					fPort;
	
	Logger					*fLogger;
	char					*fName;
	
							SimpleTask (Logger *l, char *name): fLogger (l), fName (name) {}

	virtual	ULong			GetSizeOf () { return sizeof (SimpleTask); }
	virtual long 			TaskConstructor ();
	virtual void 			TaskDestructor ();
	virtual void 			TaskMain ();
	
	void					LogAysncMessage (TUAsyncMessage *message);

	static TObjectId		TaskPort (char *name);
};

class SenderTask: public SimpleTask
{
public:
	TUPort					fReceiverPort;
	TUAsyncMessage			fAsyncMessage;
	
							SenderTask (Logger *l, char *name): SimpleTask (l, name) {}

	virtual void 			TaskMain ();
	virtual	ULong			GetSizeOf () { return sizeof (SenderTask); }
	
	void					SendSimpleMessage (ULong type);
	void					SendAsyncMessage (ULong type);
	void					SendAndModifyAsyncMessage (ULong type);
};

class ReceiverTask: public SimpleTask
{
public:
							ReceiverTask (Logger *l, char *name): SimpleTask (l, name) {}

	virtual void 			TaskMain ();
	virtual	ULong			GetSizeOf () { return sizeof (ReceiverTask); }
};

long SimpleTask::TaskConstructor ()
{
	TUNameServer nameServer;
	
	fLogger->Log (0, "SimpleTask::TaskConstructor %s\n", fName);
	fPort.Init ();
	fLogger->Log (0, "  Port: %d\n", fPort.fId);
	nameServer.UnRegisterName (fName, "TUPort");
	nameServer.RegisterName (fName, "TUPort", fPort.fId, 0);
	return noErr;
}

void SimpleTask::TaskDestructor ()
{
	TUNameServer nameServer;

	fLogger->Log (0, "SimpleTask::TaskDestructor %s\n", fName);
	nameServer.UnRegisterName (fName, "TUPort");
}

void SimpleTask::TaskMain ()
{
	fLogger->Log (0, "SimpleTask::TaskMain %s\n", fName);
}

TObjectId SimpleTask::TaskPort (char *name)
{
	TUNameServer nameServer;
	ULong id;
	ULong spec;

	nameServer.Lookup (name, "TUPort", &id, &spec);
	return id;
}

void ReceiverTask::TaskMain ()
{
	UByte message[MAX_MESSAGE];
	ULong n = MAX_MESSAGE;
	ULong type = 0;
	TUMsgToken token;
	long r;
	Boolean done = false;
	
	fLogger->Log (0, "ReceiverTask::TaskMain %s\n", fName);
	while (!done) {
		r = fPort.Receive (&n, message, MAX_MESSAGE, &token, &type);
		if (r == noErr) {
			fLogger->Log (0, "*** SimpleTask: Message received, len: %d, type: %08x\n", n, type);
			fLogger->Log (0, "    [%s]\n", message);
			if (type == 0x00000001) {
				done = true;
			} else if (type == 0x00000002) {
				Sleep (1500 * kMilliseconds);
				fLogger->Log (0, "    Modified message: [%s]\n", message);
			}
		} else {
			fLogger->Log (0, "*** SimpleTask: Error %d while waiting for message\n", r);
		}
	}
}

void SenderTask::SendSimpleMessage (ULong type)
{
	UByte message[11];
	long r;
	
	memcpy (message, "0123456789", 10); message[10] = 0;
	fLogger->Log (0, ">>> Test: Send message to %d\n", fReceiverPort.fId);
	r = fReceiverPort.Send ((void *) message, sizeof (message), kNoTimeout, type);
	fLogger->Log (0, ">>> Test: Message sent, r: %d\n", r);
}

void SenderTask::SendAsyncMessage (ULong type)
{
	UByte message[11];
	long r;
	
	memcpy (message, "9876543210", 10); message[10] = 0;
	fLogger->Log (0, ">>> Test: Send aysnc message to %d\n", fReceiverPort.fId);
	fLogger->LogAsyncMessage (0, &fAsyncMessage);
	r = fReceiverPort.Send (&fAsyncMessage, (void *) message, sizeof (message), kNoTimeout, nil, type);
	fLogger->Log (0, ">>> Test: Async message sent, r: %d\n", r);
}

void SenderTask::SendAndModifyAsyncMessage (ULong type)
{
	UByte message[11];
	long r;
	
	memcpy (message, "9876543210", 10); message[10] = 0;
	fLogger->Log (0, ">>> Test: Send aysnc message to %d\n", fReceiverPort.fId);
	fLogger->LogAsyncMessage (0, &fAsyncMessage);
	r = fReceiverPort.Send (&fAsyncMessage, (void *) message, sizeof (message), kNoTimeout, nil, type);
	fLogger->Log (0, ">>> Test: Async message sent, r: %d\n", r);
	Sleep (1000 * kMilliseconds);
	memcpy (message, "0000000000", 10); message[10] = 0;
	fLogger->Log (0, ">>> Test: Async message modified\n");
}

void SenderTask::TaskMain ()
{
	int i;
	
	fReceiverPort = TUPort (TaskPort ("Receiver"));
	fAsyncMessage.Init ();
	fLogger->Log (0, "SenderTask::TaskMain %s, receiver port: %d\n", fName, fReceiverPort.fId);

	SendSimpleMessage (0x08000000);
	SendAsyncMessage (0x00000003);
	SendAndModifyAsyncMessage (0x00000002);

	Sleep (5 * kSeconds);

	SendSimpleMessage (0x00000001);
}

extern "C" Ref MCreateTasks (RefArg rcvr)
{
	SenderTask *sender;
	ReceiverTask *receiver;
	Logger *logger;

	logger = new Logger ();
	logger->Initialize ();
	logger->Main ();

	sender = new SenderTask (logger, "Sender");
	receiver = new ReceiverTask (logger, "Receiver");

	logger->Log (0, "------------------------------------------------------------------------\n");
	logger->Log (0, "MCreateTasks (sender: %s, receiver: %s)\n", sender->fName, receiver->fName);

	SetFrameSlot (rcvr, SYM (sender), MakeInt ((ULong) sender));
	SetFrameSlot (rcvr, SYM (receiver), MakeInt ((ULong) receiver));
	SetFrameSlot (rcvr, SYM (logger), MakeInt ((ULong) logger));

	return NILREF;
}

extern "C" Ref MStartTasks (RefArg rcvr)
{
	SimpleTask* sender = (SimpleTask *) RefToInt (GetFrameSlot(rcvr, SYM (sender)));
	SimpleTask* receiver = (SimpleTask *) RefToInt (GetFrameSlot(rcvr, SYM (receiver)));
	Logger* logger = (Logger *) RefToInt (GetFrameSlot(rcvr, SYM (logger)));
	logger->Log (0, "MStartTasks\n");

	receiver->StartTask ();
	sender->StartTask ();

	return NILREF;
}

extern "C" Ref MStopTasks (RefArg rcvr)
{
	Logger* logger = (Logger *) RefToInt (GetFrameSlot(rcvr, SYM (logger)));
	logger->Log (0, "MStopTasks\n");

	delete (SenderTask *) RefToInt (GetFrameSlot(rcvr, SYM (sender)));
	delete (ReceiverTask *) RefToInt (GetFrameSlot(rcvr, SYM (receiver)));
	logger->Log (0, "\n");

	Sleep (2 * kSeconds);
	logger->Close ();
	delete logger;

	return NILREF;
}

