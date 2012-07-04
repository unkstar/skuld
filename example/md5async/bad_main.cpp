#include "coroutine.h"
#include "deferred.h"
#include <QtGui>
#include "md5.h"

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
		delete comp;
	}

	void run() {
		comp->continuation.setResult(comp->dwNumberOfBytesTransfered);
		comp->continuation.resolve();
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


Promise<DWORD> readAsync(HANDLE handle, LPVOID buffer, DWORD length, DWORD offset) {
	CompleteData *crData = new CompleteData;
	crData->overlap.Offset = offset;
  ReadFileEx(handle, buffer, length, (LPOVERLAPPED)crData, ReadCompleteRoutine);
  return crData->continuation.promise();
}


static const DWORD BUFFER_SIZE = 1024 * 256;
CHAR buffer[BUFFER_SIZE];

void *md5async(HANDLE h, QWidget* w)
{
	DWORD offset = 0;
	Promise<DWORD> reading;
	DWORD bytesReaded;
	md5_state_t context;
	md5_byte_t digest[16];

	md5_init(&context);

	do {
		w->setWindowTitle(QString("%1 bytes readed").arg(offset));
		reading = await(readAsync(h, buffer, BUFFER_SIZE, offset));
		bytesReaded = reading.result();
		if(bytesReaded) {
			md5_append(&context, (const md5_byte_t *)buffer, bytesReaded);
		}
		offset += bytesReaded;
	} while(0 != bytesReaded);
	md5_finish(&context, digest);

	char varStr[32+1] = {0};
	for (int i = 0; i < 16; ++i) {
		sprintf(&varStr[i*2], "%02x", digest[i]);
	}
	w->setWindowTitle(QString("file md5:%1").arg(varStr));
	return NULL;
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

	async((md5async, hFile, &w));

	return app.exec();
}
