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

class AsyncReceiverTask: public SimpleTask
{
public:
							AsyncReceiverTask (Logger *l, char *name): SimpleTask (l, name) {}

	virtual void 			TaskMain ();
	virtual	ULong			GetSizeOf () { return sizeof (AsyncReceiverTask); }
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
	void *sMemBuffer;
	ULong n;
	ULong sMemSize;
	ULong type;
	TUMsgToken token;
	TUAsyncMessage asyncMessage;
	long r;
	Boolean done = false;
	
	fLogger->Log (0, "ReceiverTask::TaskMain %s\n", fName);
	while (!done) {
		n = MAX_MESSAGE;
		type = 0;
		memset (message, 0, sizeof (message));
		r = fPort.Receive (&n, message, MAX_MESSAGE, &token, &type);
		if (r == noErr) {
			fLogger->Log (0, "*** SimpleTask: Message received, len: %d, type: %08x\n", n, type);
			asyncMessage = token;
			fLogger->LogMsgToken (0, &token);
			fLogger->LogAsyncMessage (0, &asyncMessage);
			fLogger->Log (0, "    [%s] (%d)\n", message, n);

			if (type == 0x00000001) {
				done = true;
			} else if (type == 0x00000002) {
				Sleep (1500 * kMilliseconds);
				fLogger->Log (0, "    Modified message: [%s] (%d)\n", message, n);
			}

			TUSharedMemMsg sMem(asyncMessage.GetMsgId ());
			r = sMem.CopyFromShared (&n, message, MAX_MESSAGE);
			sMem.GetSize (&sMemSize, &sMemBuffer);
			fLogger->Log (0, "    Copied async data: %d, [%s] (%d)\n", r, message, n);
			fLogger->Log (0, "    Async buffer: %08x %d\n", sMemBuffer, sMemSize);

			r = token.CashMessageToken (&n, message, MAX_MESSAGE);
			fLogger->Log (0, "    Cashed token: %d, [%s] (%d)\n", r, message, n);
			
			r = sMem.MsgDone (noErr, token.GetSignature ());
			fLogger->Log (0, "    Message done: %d\n");
		} else {
			fLogger->Log (0, "*** SimpleTask: Error %d while waiting for message\n", r);
		}
	}
}

void AsyncReceiverTask::TaskMain ()
{
	TUAsyncMessage *message;
	long r;
	Boolean done = false;
	
	fLogger->Log (0, "AsyncReceiverTask::TaskMain %s\n", fName);
	while (!done) {
		message = new TUAsyncMessage ();
		message->Init (false);
		while (!fPort.IsMsgAvailable ()) {
			Sleep (250 * kMilliseconds);
		}
		r = fPort.Receive (message);
		if (r == noErr) {
			fLogger->Log (0, "*** AsyncReceiverTask: Message received\n");
			fLogger->LogAsyncMessage (0, message);
		} else {
			fLogger->Log (0, "*** AsyncReceiverTask: Error %d while waiting for message\n", r);
			done = true;
		}
		delete message;
	}
}

void SenderTask::SendSimpleMessage (ULong type)
{
	UByte message[11];
	long r;
	
	memcpy (message, "0123456789", 10); message[10] = 0;
	fLogger->Log (0, ">>> Test: Send message %08x to %d\n", message, fReceiverPort.fId);
	r = fReceiverPort.Send ((void *) message, sizeof (message), kNoTimeout, type);
	fLogger->Log (0, ">>> Test: Message sent, r: %d\n", r);
}

void SenderTask::SendAsyncMessage (ULong type)
{
	UByte message[11];
	long r;
	ULong n;
	TTime delay = GetGlobalTime () + TTime (1000, kMilliseconds);
	
	memcpy (message, "9876543210", 10); message[10] = 0;
	fLogger->Log (0, ">>> Test: Send aysnc message [%s] (%08x) to %d\n", message, message, fReceiverPort.fId);
	fLogger->LogAsyncMessage (0, &fAsyncMessage);

	TUSharedMemMsg sMem(fAsyncMessage.GetMsgId ());
	/*
	r = sMem.CopyToShared (message, sizeof (message));
	fLogger->Log (0, "    Shared message, copy in r: %d", r);
	r = sMem.CopyFromShared (&n, message, MAX_MESSAGE);
	fLogger->Log (0, " out r: %d, [%s] (%d)\n", r, message, n);
	memcpy (message, "5678943210", 10); message[10] = 0;
	*/

	r = fReceiverPort.Send (&fAsyncMessage, (void *) message, sizeof (message), kNoTimeout, &delay, type);

	// this will turn it into a synchronous message:
	fAsyncMessage.BlockTillDone ();

	fLogger->Log (0, ">>> Test: Async message sent, r: %d\n", r);
	
	// At this point, message goes out of scope, which is not allowed - unless BlockTillDone was used
}

void SenderTask::SendAndModifyAsyncMessage (ULong type)
{
	UByte message[11];
	long r;
	
	memcpy (message, "9876543210", 10); message[10] = 0;
	fLogger->Log (0, ">>> Test: Send aysnc and modify message [%s] to %d\n", message, fReceiverPort.fId);
	fLogger->LogAsyncMessage (0, &fAsyncMessage);
	r = fReceiverPort.Send (&fAsyncMessage, (void *) message, sizeof (message), kNoTimeout, nil, type);
	fLogger->Log (0, ">>> Test: Async message sent, r: %d\n", r);
	Sleep (1000 * kMilliseconds);
	memcpy (message, "0000000000", 10); message[10] = 0;
	fLogger->Log (0, ">>> Test: Async message [%s] modified\n", message);
}

void SenderTask::TaskMain ()
{
	int i;
	long r;
	
	fReceiverPort = TUPort (TaskPort ("Receiver"));

	fLogger->Log (0, "SenderTask::TaskMain %s, receiver port: %d\n", fName, fReceiverPort.fId);

	fAsyncMessage.Init (false);
	TUSharedMemMsg sMem(fAsyncMessage.GetMsgId ());
	r = sMem.SetBuffer (malloc (128), 128, kSMemReadWrite);
	fLogger->Log (0, "Message %d created and buffer set: %d\n", fAsyncMessage.GetMsgId (), r);
	fLogger->LogAsyncMessage (0, &fAsyncMessage);

	SendSimpleMessage (0x08000000);
	SendAsyncMessage (0x00000003);
	Sleep (2 * kSeconds);
	SendAndModifyAsyncMessage (0x00000002);

	Sleep (5 * kSeconds);

	SendSimpleMessage (0x00000001);
}

extern "C" Ref MCreateTasks (RefArg rcvr)
{
	SimpleTask *sender;
	SimpleTask *receiver;
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

	delete (SimpleTask *) RefToInt (GetFrameSlot(rcvr, SYM (sender)));
	delete (SimpleTask *) RefToInt (GetFrameSlot(rcvr, SYM (receiver)));
	logger->Log (0, "\n");

	Sleep (2 * kSeconds);
	logger->Close ();
	delete logger;

	return NILREF;
}

