#ifndef __LOGGER_H
#define __LOGGER_H

void hammer_log (int desiredLevel, int currentLevel, char *format, ...);

class TSerialChip;
class TCircleBuf;
class TULockingSemaphore;
class TUAsyncMessage;
class TUMsgToken;

// ================================================================================
// ¥ Logger
// ================================================================================

class Logger
{
public:
	ULong 					fLogLevel;
	TULockingSemaphore*		fSemaphore;
	TSerialChip* 			fChip;
	
	Boolean					fBufferOutput;
	
	UByte*					fBuffer;
	ULong					fHead;
	ULong					fTail;

	ULong					fLocation;
	ULong					fSpeed;
	
	NewtonErr				Initialize ();
	long					Main ();
	void					Close ();
	
	void 					TxBEmptyIntHandler (void);
	void 					ExtStsIntHandler (void);
	void 					RxCAvailIntHandler (void);
	void 					RxCSpecialIntHandler (void);
	
	void					BufferOutput (Boolean buffer);
	void					StartOutput ();
	void 					Output (UByte*, ULong);

	void 					Log (Long logLevel, char *format, ...);
	void					LogHex (Long logLevel, void *data, int length);
	void					LogAsyncMessage (Long logLevel, TUAsyncMessage *message);
	void					LogMsgToken (Long logLevel, TUMsgToken *token);
};

class TLogFunction
{
public:
	NewtonErr				*returnValue;
	char					*message;
	Logger					*logger;
	
							TLogFunction (Logger *l, char *name, NewtonErr *r);
							TLogFunction (Logger *l, char *name);
							~TLogFunction ();
};

#define HLOG					fLogger->Log
#define HLOGFN(name)			TLogFunction __logfn(fLogger, name);
#define HLOGFNR(name, r)		TLogFunction __logfn(fLogger, name, &r);

#endif
