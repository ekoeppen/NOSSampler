#include <HALOptions.h>
#include <NewtonScript.h>
#include "UserTasks.h"
#include "Logger.h"

class SimpleTask: public TUTaskWorld
{
public:
	Logger*					fLogger;
	
							SimpleTask (Logger *l): fLogger (l) {}

	void					Initialize ();

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
	fLogger->Log (0, "SimpleTask::TaskConstructor %08x\n", this);
	return noErr;
}

void SimpleTask::TaskDestructor ()
{
	fLogger->Log (0, "SimpleTask::TaskDestructor %08x\n", this);
}

void SimpleTask::TaskMain ()
{
	fLogger->Log (0, "SimpleTask::TaskMain %08x\n", this);
}


extern "C" Ref MCreateTasks (RefArg rcvr)
{
	SimpleTask *task;
	Logger *logger;

	logger = new Logger ();
	logger->Initialize ();
	logger->Main ();

	logger->Log (0, "MCreateTasks (task: %08x logger: %08x)\n", task, logger);

	task = new SimpleTask (logger);

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

extern "C" Ref MStopTasks (RefArg rcvr)
{
	SimpleTask* task = (SimpleTask *) RefToInt (GetFrameSlot(rcvr, SYM (task)));
	Logger* logger = (Logger *) RefToInt (GetFrameSlot(rcvr, SYM (logger)));
	logger->Log (0, "MStopTasks (task: %08x logger: %08x)\n", task, logger);

	Sleep (3 * kSeconds);
	delete task;
	logger->Close ();
	delete logger;

	return NILREF;
}

