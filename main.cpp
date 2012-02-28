#include "coroutine.h"
#include "deferred.h"
#include <QtGui>
#include "md5.h"

static int ptcc = 0;

struct PromiseTimerCtx
{
	HANDLE			timer;
	Deferred<>	deferred;

	PromiseTimerCtx() {++ptcc;}
	~PromiseTimerCtx() {--ptcc;}
};

VOID CALLBACK PromiseTimerAPCProc(
  LPVOID lpArgToCompletionRoutine,
  DWORD /*dwTimerLowValue*/,
  DWORD /*dwTimerHighValue*/)
{
	PromiseTimerCtx *ctx = (PromiseTimerCtx*)lpArgToCompletionRoutine;
	if(ctx->timer != NULL) {
		::CloseHandle(ctx->timer);
		ctx->deferred.resolve();
	}
	delete ctx;
}

void PromiseTimerCancelRoutine(void *c)
{
	PromiseTimerCtx *ctx = (PromiseTimerCtx*)c;
	if(ctx != NULL && ctx->timer != NULL) {
		::CloseHandle(ctx->timer);
		ctx->timer = NULL;
	}
	//do NOT delete ctx here, the PromiseTimerAPCProc always got called
}

Promise<> setTimeout(UINT dwMillisecond) {
	PromiseTimerCtx *ctx = new PromiseTimerCtx;
	ctx->timer = ::CreateWaitableTimer(NULL, TRUE, NULL);
	if(ctx->timer) {
		LARGE_INTEGER due;
		due.QuadPart = -10000LL * dwMillisecond;
		if(::SetWaitableTimer(ctx->timer, &due, 0, PromiseTimerAPCProc, (LPVOID)ctx, FALSE)) {
			ctx->deferred.setContext(ctx, PromiseTimerCancelRoutine);
			return ctx->deferred.promise();
		}
		::CloseHandle(ctx->timer);
	}

	//failed, clean up everything
	delete ctx;
	return Deferred<>(false).promise();
}

LRESULT CALLBACK CallWndProc(
  int /*nCode*/,
  WPARAM /*wParam*/,
  LPARAM /*lParam*/)
{
	//poor workaround for win7 move/sizing PREMATURE OPTIMIZATION
	MsgWaitForMultipleObjectsEx(0, NULL, 0, QS_ALLINPUT, MWMO_ALERTABLE);
	return 0;
}

struct CompleteData {
	CompleteData()
		:overlap(), dwErrorCode(0), dwNumberOfBytesTransfered(0), continuation() {}
	OVERLAPPED overlap;
  DWORD dwErrorCode;
  DWORD dwNumberOfBytesTransfered;
	Deferred<DWORD> continuation;
};

class IOEvent : public QEvent
{
public:
	IOEvent(CompleteData *c)
		: QEvent(KEventType), comp(c)
	{
	}

	~IOEvent()
	{
	}

	void run() {
		comp->continuation.setResult(comp->dwNumberOfBytesTransfered);
		comp->continuation.resolve();
		delete comp;
		comp = NULL;
	}

	CompleteData *comp;
	static Type KEventType;
};

QEvent::Type IOEvent::KEventType = (QEvent::Type)QEvent::registerEventType();

class RacApplication
	: public QApplication
{
public:
	RacApplication(int &argc, char **argv)
		: QApplication(argc, argv)
	{
		SetWindowsHookEx(WH_CALLWNDPROC, CallWndProc, NULL, GetCurrentThreadId());
	}

protected:
	virtual bool event( QEvent *e )
	{
		if(e->type() == IOEvent::KEventType) {
			IOEvent* iev = (IOEvent*)e;
			iev->run();
			return true;
		}

		return QApplication::event(e);
	}
};


VOID CALLBACK ReadCompleteRoutine(
  DWORD dwErrorCode,
  DWORD dwNumberOfBytesTransfered,
  LPOVERLAPPED lpOverlapped)
{
	CompleteData *pData = (CompleteData*)lpOverlapped;
	pData->dwErrorCode = dwErrorCode;
	pData->dwNumberOfBytesTransfered = dwNumberOfBytesTransfered;
	QApplication::postEvent(QCoreApplication::instance(), new IOEvent(pData));
}

void cancelReadAsync(void *ctx)
{
	CompleteData *crData = (CompleteData*)ctx;
	HANDLE hFile = crData->overlap.hEvent;
	::CancelIoEx(hFile, &crData->overlap);
	//do not delete crData here, ReadCompleteRoutine always got called
}


Promise<DWORD> readAsync(HANDLE handle, LPVOID buffer, DWORD length, DWORD offset) {
	CompleteData *crData = new CompleteData;
	crData->overlap.Offset = offset;
	crData->overlap.hEvent = handle; //msdn said we can use it
	crData->continuation.setContext((void*)crData, cancelReadAsync);
  ReadFileEx(handle, buffer, length, (LPOVERLAPPED)crData, ReadCompleteRoutine);
  return crData->continuation.promise();
}


static const DWORD BUFFER_SIZE = 1024 * 1024;
CHAR buf1[BUFFER_SIZE];
CHAR buf2[BUFFER_SIZE];

void md5async(HANDLE h, QWidget* w)
{
	DWORD offset = 0;
	Promise<DWORD> reading;
	DWORD bytesReaded;
	md5_state_t context;
	md5_byte_t digest[16];
	CHAR *readingBuffer = buf1;
	CHAR *calculatingBuffer = buf1;

	md5_init(&context);

	reading = readAsync(h, readingBuffer, BUFFER_SIZE, offset);
	await(reading);
	bytesReaded = reading.result();
	offset += bytesReaded;
	do {
		std::swap(readingBuffer, calculatingBuffer);
		//w->setWindowTitle(QString("%1 bytes readed").arg(offset));
		reading = readAsync(h, readingBuffer, BUFFER_SIZE, offset);
		if(bytesReaded) {
			md5_append(&context, (const md5_byte_t *)calculatingBuffer, bytesReaded);
		}
		Promise<> timeout = setTimeout(1 * 1000);
		//alternative:
		//timeout.done(boost::bind(&Promise<DWORD>::cancel, &reading));
		try {
			await(reading || timeout);
		} catch(CancelAsyncException &e) {
			w->setWindowTitle("IO cancelled!");
			throw;
		}
		if(maybe(reading.state())) {
			reading.cancel();
			w->setWindowTitle("IO time out!");
			return;
		} else {
			timeout.cancel();
		}
		bytesReaded = reading.result();
		offset += bytesReaded;
	} while(0 != bytesReaded);
	md5_finish(&context, digest);

	char varStr[32+1] = {0};
	for (int i = 0; i < 16; ++i) {
		sprintf(&varStr[i*2], "%02x", digest[i]);
	}
	w->setWindowTitle(QString("file md5:%1").arg(varStr));
}

int main(int argc, char *argv[])
{
	RacApplication app(argc, argv);
	QWidget w;
	w.show();

	HANDLE hFile = CreateFile(
		app.arguments()[1].utf16(),
		GENERIC_READ,
		FILE_SHARE_READ,
		NULL,
		OPEN_EXISTING,
		FILE_FLAG_SEQUENTIAL_SCAN | FILE_FLAG_OVERLAPPED,
		NULL);

	Coroutine cr = async((md5async, hFile, &w));

	setTimeout(20 * 1000).done(boost::bind(&Promise<>::cancel, cr.promise()));

	return app.exec();
}
