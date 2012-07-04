#ifndef __COROUTINE_ASIO_WIN32_H__
#define __COROUTINE_ASIO_WIN32_H__

#include <QtCore>
#include <QApplication>
#include "promise/promise.h"
#include "promise/deferred.h"
#include "promise/deferred_context.h"

DW_COROUTINE_EXPORT Promise<> setTimeout(UINT dwMillisecond);
DW_COROUTINE_EXPORT Promise<DWORD> readAsync(HANDLE handle, boost::shared_array<char> buffer, DWORD length, LONGLONG offset);

class CompleteData;

class DW_COROUTINE_EXPORT IOEvent : public QEvent
{
public:
	IOEvent(CompleteData *c)
		: QEvent(KEventType), comp(c)
	{
	}

	~IOEvent()
	{
	}

	void run();

	CompleteData *comp;
	static Type KEventType;
};

class DW_COROUTINE_EXPORT RacApplication
	: public QApplication
{
public:
	RacApplication(int &argc, char **argv);

  void init(){}

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

#endif //__COROUTINE_ASIO_WIN32_H__
