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

	virtual	ULong			GetSizeOf ();
	virtual long 			TaskConstructor ();
	virtual void 			TaskDestructor ();
	virtual void 			TaskMain ();

};

ULong SimpleTask::GetSizeOf ()
{
	return sizeof (SimpleTask);
}

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
	UByte message[MAX_MESSAGE];
	ULong n = MAX_MESSAGE;
	ULong type = 0;
	TUMsgToken token;
	long r;
	
	fLogger->Log (0, "SimpleTask::TaskMain %s\n", fName);
	while (true) {
		r = fPort.Receive (&n, message, MAX_MESSAGE, &token, &type);
		if (r == noErr) {
			fLogger->Log (0, "*** SimpleTask: Message received, len: %d, type: %08x\n", n, type);
		} else {
			fLogger->Log (0, "*** SimpleTask: Error %d while waiting for message\n", r);
		}
	}
}

TObjectId TaskPort (char *name)
{
	TUNameServer nameServer;
	ULong id;
	ULong spec;

	nameServer.Lookup (name, "TUPort", &id, &spec);
	return id;
}

extern "C" Ref MCreateTasks (RefArg rcvr)
{
	SimpleTask *task;
	Logger *logger;

	logger = new Logger ();
	logger->Initialize ();
	logger->Main ();

	task = new SimpleTask (logger, "TaskA");

	logger->Log (0, "------------------------------------------------------------------------\n");
	logger->Log (0, "MCreateTasks (task: %s logger: %08x)\n", task->fName, logger);

	SetFrameSlot (rcvr, SYM (task), MakeInt ((ULong) task));
	SetFrameSlot (rcvr, SYM (logger), MakeInt ((ULong) logger));

	return NILREF;
}

extern "C" Ref MStartTasks (RefArg rcvr)
{
	SimpleTask* task = (SimpleTask *) RefToInt (GetFrameSlot(rcvr, SYM (task)));
	Logger* logger = (Logger *) RefToInt (GetFrameSlot(rcvr, SYM (logger)));
	logger->Log (0, "MStartTasks (task: %08x logger: %08x)\n", task, logger);

	task->StartTask ();

	return NILREF;
}

void Test_SendSimpleMessage (Logger *logger)
{
	UByte message[10];
	long r;
	TUPort taskPort (TaskPort ("TaskA"));
	
	logger->Log (0, ">>> Test: Send message to %d\n", taskPort.fId);
	memcpy (message, "0123456789", 10);
	r = taskPort.Send ((void *) message, sizeof (message));
	logger->Log (0, ">>> Test: Message sent, r: %d\n", r);
}

extern "C" Ref MRunTests (RefArg rcvr)
{
	Logger* logger = (Logger *) RefToInt (GetFrameSlot(rcvr, SYM (logger)));
	int i;

	logger->Log (0, "MRunTests\n");

	for (i = 0; i < 10; i++) {
		Test_SendSimpleMessage (logger);
	}

	return NILREF;
}

extern "C" Ref MStopTasks (RefArg rcvr)
{
	long r;
	SimpleTask* task = (SimpleTask *) RefToInt (GetFrameSlot(rcvr, SYM (task)));
	Logger* logger = (Logger *) RefToInt (GetFrameSlot(rcvr, SYM (logger)));
	logger->Log (0, "MStopTasks\n");

	TUTask t (task->GetChildTaskId ());
	t.DestroyObject ();
	delete task;
	logger->Log (0, "\n");

	Sleep (3 * kSeconds);
	logger->Close ();
	delete logger;

	return NILREF;
}

