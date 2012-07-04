#ifndef __COROUTINE_ASIO_LINUX_H__
#define __COROUTINE_ASIO_LINUX_H__

#include <QtCore>
#include <QApplication>
#include <boost/shared_array.hpp>
#include "promise/promise.h"
#include "promise/deferred.h"
#include "promise/deferred_context.h"

Promise<> setTimeout(unsigned dwMillisecond);
Promise<ssize_t> readAsync(int fd, boost::shared_array<char> buffer, size_t length, off_t offset);

class RacApplication
	: public QApplication
{
  Q_OBJECT

public:
	RacApplication(int &argc, char **argv);

public slots:
  void signalActivated();
private:
  QSocketNotifier *m_notifier;
};


#endif //__COROUTINE_ASIO_LINUX_H__
