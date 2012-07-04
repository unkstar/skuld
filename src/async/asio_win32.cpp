#include "asio.h"


QEvent::Type IOEvent::KEventType = (QEvent::Type)QEvent::registerEventType();

class PromiseTimer
	: public DeferredContext<void>
{
	public:
		PromiseTimer();
		~PromiseTimer();
		bool start(UINT dwMillisecond);

		virtual void cancel_me();

		static VOID CALLBACK APCProc(
			LPVOID lpArgToCompletionRoutine,
			DWORD /*dwTimerLowValue*/,
			DWORD /*dwTimerHighValue*/);

	private:
		HANDLE			timer;
};

PromiseTimer::PromiseTimer()
{
}

VOID CALLBACK PromiseTimer::APCProc(
  LPVOID lpArgToCompletionRoutine,
  DWORD /*dwTimerLowValue*/,
  DWORD /*dwTimerHighValue*/)
{
	PromiseTimer *ctx = (PromiseTimer*)lpArgToCompletionRoutine;
	ctx->resolve();
	delete ctx;
}

PromiseTimer::~PromiseTimer()
{
	if(timer) {
		::CloseHandle(timer);
	}
}

bool PromiseTimer::start(UINT dwMillisecond)
{
	timer = ::CreateWaitableTimer(NULL, TRUE, NULL);
	if(timer) {
		LARGE_INTEGER due;
		due.QuadPart = -10000LL * dwMillisecond;
		if(::SetWaitableTimer(timer, &due, 0, &PromiseTimer::APCProc, (LPVOID)this, FALSE)) {
			return true;
		}
	}

	return false;
}

//virtual
void PromiseTimer::cancel_me()
{
	if(!isActivating()) {
		CancelWaitableTimer(timer);
		delete this;
	}
}

Promise<> setTimeout(UINT dwMillisecond) {
	PromiseTimer *ctx = new PromiseTimer;
	if(ctx->start(dwMillisecond)) {
		return ctx->promise();
	}
	delete ctx;
	return false;
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

RacApplication::RacApplication(int &argc, char **argv)
	: QApplication(argc, argv)
{
	SetWindowsHookEx(WH_CALLWNDPROC, CallWndProc, NULL, GetCurrentThreadId());
}

class CompleteData
	: public DeferredContext<DWORD>
{
	public:

		PINPOINT_DEFERRED_CONTEXT(CompleteData, OVERLAPPED, overlap)

		CompleteData(HANDLE file, LONGLONG offset, boost::shared_array<char> buffer)
			: overlap(), dwErrorCode(0),
				dwNumberOfBytesTransfered(0), m_buffer(buffer)
		{
			LARGE_INTEGER li;
			li.QuadPart = offset;
			overlap.hEvent = file;
			overlap.Offset = li.LowPart;
			overlap.OffsetHigh = li.HighPart;
		}

		~CompleteData()
		{
		}

		virtual void cancel_me();
		virtual DWORD result() {return dwNumberOfBytesTransfered;}

		static VOID CALLBACK ReadCompleteRoutine( DWORD dwErrorCode,
																							DWORD dwNumberOfBytesTransfered,
																							LPOVERLAPPED lpOverlapped);

		OVERLAPPED overlap;
		DWORD dwErrorCode;
		DWORD dwNumberOfBytesTransfered;
    boost::shared_array<char> m_buffer;
};


void IOEvent::run()
{
  if(0 == comp->dwErrorCode || ERROR_HANDLE_EOF == comp->dwErrorCode) {
    comp->resolve();
  } else {
    comp->reject();
  }
	delete comp;
	comp = NULL;
}

VOID CALLBACK CompleteData::ReadCompleteRoutine(
  DWORD dwErrorCode,
  DWORD dwNumberOfBytesTransfered,
  LPOVERLAPPED lpOverlapped)
{
	CompleteData *pData = CompleteData::fromContext(lpOverlapped);
	pData->dwErrorCode = dwErrorCode;
	pData->dwNumberOfBytesTransfered = dwNumberOfBytesTransfered;
	if(pData->overlap.hEvent) {
		QApplication::postEvent(QCoreApplication::instance(), new IOEvent(pData));
	} else {
		delete pData;
	}
}

//virtual
void CompleteData::cancel_me()
{
	HANDLE hFile = this->overlap.hEvent;
	::CancelIoEx(hFile, &this->overlap);
	this->overlap.hEvent = NULL;
	//do NOT delete myself here, ReadCompleteRoutine always got called
}


Promise<DWORD> readAsync(HANDLE handle, boost::shared_array<char> buffer, DWORD length, LONGLONG offset)
{
	CompleteData *crData = new CompleteData(handle, offset, buffer);
	if(ReadFileEx(handle, buffer.get(), length, crData->toContext(), &CompleteData::ReadCompleteRoutine)) {
		return crData->promise();
	}
	delete crData;
	return false;
}
