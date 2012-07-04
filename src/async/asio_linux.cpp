#include <aio.h>
#include <errno.h>
#include <sys/signalfd.h>
#include <QSocketNotifier>
#include "async/asio.h"

//XXX:not good, random coupling
#include "async/async_jobpool_p.h"

//use QTimer instead
class PromiseTimer
	: public DeferredContext<void>
{
	public:
		PromiseTimer();
		~PromiseTimer();
		bool start(unsigned dwMillisecond);

		virtual void cancel_me();

	private:
		QTimer timer;
};

PromiseTimer::PromiseTimer()
  :timer()
{
}


PromiseTimer::~PromiseTimer()
{
}

bool PromiseTimer::start(unsigned dwMillisecond)
{
  timer.setSingleShot(true);
  timer.start(dwMillisecond);
  return true;
}

//virtual
void PromiseTimer::cancel_me()
{
	if(!isActivating()) {
    timer.stop();
		delete this;
	}
}

Promise<> setTimeout(unsigned dwMillisecond) {
	PromiseTimer *ctx = new PromiseTimer;
	if(ctx->start(dwMillisecond)) {
		return ctx->promise();
	}
	delete ctx;
	return false;
}

class CompleteData
	: public DeferredContext<ssize_t>
{
  public:
		CompleteData(int filfdes, boost::shared_array<char> buf, size_t nbytes, off_t offset)
      : m_aiocb(), m_buf(buf)
    {
      m_aiocb.aio_fildes = filfdes;
      m_aiocb.aio_offset = offset;
      m_aiocb.aio_buf = buf.get();
      m_aiocb.aio_nbytes = nbytes;

      m_aiocb.aio_sigevent.sigev_notify = SIGEV_SIGNAL;
      m_aiocb.aio_sigevent.sigev_signo = IO_SIGNAL;
      m_aiocb.aio_sigevent.sigev_value.sival_ptr = this;
    }

    void done()
    {
      int err = aio_error(&m_aiocb);
      Q_ASSERT(EINPROGRESS != err);
      int result = aio_return(&m_aiocb);

      if(0 == err && -1 != result) {
        this->resolve(result);
      } else {
        this->reject();
      }
    }

    virtual void cancel_me()
    {
      aio_cancel(m_aiocb.aio_fildes, &m_aiocb);
    }

    aiocb m_aiocb;
    boost::shared_array<char> m_buf;
};

Promise<ssize_t> readAsync(int fd, boost::shared_array<char> buffer, size_t length, off_t offset)
{
  CompleteData *crData = new CompleteData(fd, buffer, length, offset);
  if(0 == aio_read(&crData->m_aiocb)) {
    return crData->promise();
  }
  delete crData;
  return false;
}

RacApplication::RacApplication(int &argc, char **argv)
	: QApplication(argc, argv), m_notifier(NULL)
{
  sigset_t mask;
  int sfd;

  sigemptyset(&mask);
  sigaddset(&mask, JOB_IS_DONE_SIGNAL);
  sigaddset(&mask, IO_SIGNAL);

  sfd = signalfd(-1, &mask, SFD_NONBLOCK);
  if (sfd != -1) {
    m_notifier = new QSocketNotifier(sfd, QSocketNotifier::Read, this);
    if(connect(m_notifier, SIGNAL(activated(int)), SLOT(signalActivated()))) {
      return; //ok!
    }
  }
  quit();
}

void RacApplication::signalActivated()
{
  struct signalfd_siginfo fdsi;
  ssize_t s;

  m_notifier->setEnabled(false);

  s = read(m_notifier->socket(), &fdsi, sizeof(struct signalfd_siginfo));
  if (s != sizeof(struct signalfd_siginfo)) {
    quit();
  }

  if (fdsi.ssi_signo == JOB_IS_DONE_SIGNAL) {
    ComputationalJob *j = (ComputationalJob*)fdsi.ssi_ptr;
    j->CompleteRoutine();
    delete j;
  } else if (fdsi.ssi_signo == IO_SIGNAL) {
    CompleteData *crData = (CompleteData*)fdsi.ssi_ptr;
    crData->done();
    delete crData;
  } else {
    qWarning("Read unexpected signal\n");
  }

  m_notifier->setEnabled(true);
}

//XXX:if some global object create thread before we mask signal, readAsync will crash on it!
struct BlockMyRtSignals {

  BlockMyRtSignals() {
    sigset_t mask;

    sigemptyset(&mask);
    sigaddset(&mask, JOB_IS_DONE_SIGNAL);
    sigaddset(&mask, IO_SIGNAL);

    sigprocmask(SIG_BLOCK, &mask, NULL);
  }
};

static BlockMyRtSignals block_me;
