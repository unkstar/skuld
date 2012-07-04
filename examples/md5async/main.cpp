#include <QtGui>
#include <QLabel>
#include <QTime>
#include <QPushButton>
#include <boost/shared_ptr.hpp>

#include "md5.h"

#include "coroutine/asio.h"
#include "coroutine/coroutine.h"
#include "coroutine/async_jobpool.h"
#include "coroutine/signal_promise.h"

#ifdef POSIX

#define SIZE_TYPE ssize_t
#define OFFSET_TYPE off_t
#define HANDLE int

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


HANDLE openFile(const QString &filename) {
  return open(filename.toUtf8().constData(), O_RDONLY);
}

#elif defined(WIN32)

#define SIZE_TYPE DWORD
#define OFFSET_TYPE LONGLONG

HANDLE openFile(const QString &filename) {
  return CreateFile(
      filename.utf16(),
      GENERIC_READ,
      FILE_SHARE_READ,
      NULL,
      OPEN_EXISTING,
      FILE_FLAG_SEQUENTIAL_SCAN | FILE_FLAG_OVERLAPPED,
      NULL);
}

#endif

static const SIZE_TYPE BUFFER_SIZE = 1024 * 1024;
static const SIZE_TYPE kTimeoutDue = 1000 * 1000;

class md5task
	: public ITask
{
	public:
		md5task(boost::shared_ptr<md5_state_t> ctx, const md5_byte_t *content, unsigned int len)
			: m_ctx(ctx), m_content(content), m_len(len)
		{
		}

		void run()
		{
			md5_append(m_ctx.get(), m_content, m_len);
		}

	private:
    boost::shared_ptr<md5_state_t> m_ctx;
		const md5_byte_t *m_content;
		unsigned int m_len;
};

class MainWindow
	: public QWidget
{
	Q_OBJECT
		Q_DISABLE_COPY(MainWindow)

	public:
		MainWindow()
			:buf1(new char[BUFFER_SIZE]), buf2(new char[BUFFER_SIZE])
		{
			setAttribute(Qt::WA_DeleteOnClose);
			setAcceptDrops(true);

			QHBoxLayout *hlayout = new QHBoxLayout;
			QVBoxLayout *vlayout = new QVBoxLayout;
			toggle = new QPushButton("&Start", this);
			label = new QLabel(this);
			label->setText("Please drop file here");
			cancel = new QPushButton("&Cancel", this);
			toggle->setDisabled(true);
			cancel->setDisabled(true);
			hlayout->addWidget(toggle);
			hlayout->addWidget(cancel);
			vlayout->addWidget(label);
			vlayout->addItem(hlayout);
			setLayout(vlayout);
		}

		~MainWindow()
		{
		}

	protected:

		virtual void dragEnterEvent(QDragEnterEvent *event)
		{
			if(event->mimeData()->hasUrls())
				event->acceptProposedAction();
		}

		virtual void dragMoveEvent(QDragMoveEvent *event)
		{
			if(event->mimeData()->hasUrls())
				event->acceptProposedAction();
		}

		virtual void dropEvent ( QDropEvent * event )
		{
			if(event->mimeData()->hasUrls())
			{
				QString path = event->mimeData()->urls().first().toLocalFile();
				if(!path.isEmpty())
				{
					async((&MainWindow::on_file_add, this, path));
				}
			}
		}

		void closeEvent ( QCloseEvent *)
		{
			m_closed.resolve();
		}

		void md5async(HANDLE hFile)
		{
			OFFSET_TYPE offset = 0;
			SIZE_TYPE bytesReaded = 0;
			Promise<SIZE_TYPE> reading;
			Promise<ITask*> calculating(true);
      boost::shared_ptr<md5_state_t> context(new md5_state_t);
			md5_byte_t digest[16];
			boost::shared_array<char> readingBuffer = buf1;
			boost::shared_array<char> calculatingBuffer = buf1;
			QTime t;
			int elapsed = 0;

			SignalPromise toggle_clicked = signalPromise(toggle, SIGNAL(clicked(bool)));
			await(toggle_clicked);

			toggle->setText("&Pause");
			t.start();

			md5_init(context.get());

			do {
				std::swap(readingBuffer, calculatingBuffer);
				if(offset) {
					label->setText(QString("%1 bytes readed").arg(offset));
				}
				reading = readAsync(hFile, readingBuffer, BUFFER_SIZE, offset);
				if(bytesReaded) {
					//md5_append(&context, (const md5_byte_t *)calculatingBuffer, bytesReaded);
					calculating = calculateAsync(new md5task(context, (const md5_byte_t *)calculatingBuffer.get(), bytesReaded));
				}
				Promise<> timeout = setTimeout(kTimeoutDue);
				bool paused = false;

				do {
					paused = false;
					await((reading && calculating) || timeout || toggle_clicked);
					if(reading.state() && calculating.state()) {
						timeout.cancel();
					} else if(!reading.state()) {
						label->setText("IO failed!");
						timeout.cancel();
						return;
					} else {
						reading.cancel();
						timeout.cancel();
						if(!indeterminate(toggle_clicked.state())) {
							elapsed += t.elapsed();
							toggle->setText("&Start");
							await(toggle_clicked);
							t.start();
							toggle->setText("&Pause");
							reading = readAsync(hFile, readingBuffer, BUFFER_SIZE, offset);
							timeout = setTimeout(kTimeoutDue);
							paused = true;
						} else {
							timeout.cancel();
							label->setText("IO time out!");
							return;
						}
					}
				} while(paused);

				bytesReaded = reading.result();
				offset += bytesReaded;
			} while(0 != bytesReaded);
			md5_finish(context.get(), digest);

			toggle->setText("&Start");

			elapsed += t.elapsed();
			label->setText(QString("file md5: %1, calculated in %2 millisecond")
					.arg(QByteArray((const char*)digest, sizeof(digest)).toHex().constData())
					.arg(elapsed));
		}

		void on_file_add(QString path)
		{
			m_file_dropped.resolve();
			Deferred<void> new_deferred;
			m_file_dropped = new_deferred;
			label->setText(path);

			HANDLE hFile = openFile(path);

			SignalPromise cancel_clicked = signalPromise(cancel, SIGNAL(clicked()));
			Promise<> cancel_promise = cancel_clicked || m_closed || m_file_dropped;
			cancel_promise.done(boost::bind(&Promise<>::cancel, GetCurrentCoroutine().promise()));

			toggle->setText("&Start");
			toggle->setDisabled(false);
			cancel->setDisabled(false);
			try {
				md5async(hFile);
			} catch(CancelAsyncException &) {
				label->setText("IO cancelled!");
			}
			toggle->setDisabled(true);
			cancel->setDisabled(true);
		}

	private:
		QPushButton *toggle;
		QPushButton *cancel;
		QLabel *label;
		boost::shared_array<char> buf1;
		boost::shared_array<char> buf2;
		Deferred<void> m_file_dropped;
		Deferred<void> m_closed;
};

int main(int argc, char *argv[])
{
	RacApplication app(argc, argv);

	MainWindow *w0 = new MainWindow;
	w0->show();
	MainWindow *w1 = new MainWindow;
	w1->show();

	return app.exec();
}

#include "main.moc"
