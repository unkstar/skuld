#include <QObject>
#include <QPointer>
#include <QMetaObject>
#include <QMetaMethod>
#include "async/signal_promise.h"
#include "promise/deferred_context.h"

class DeferredSignal
	: public QObject, //for qt_metacall, persudo omni-slot
		public DeferredContext<QVariant>
{
	Q_DISABLE_COPY(DeferredSignal)
	public:
		DeferredSignal(QObject *src, QMetaMethod method, int idx);
		~DeferredSignal()
		{
		}

		void cancel_me();
		tribool reset_me();
		QVariant result();

		//omni-slot
		int qt_metacall(QMetaObject::Call call, int index, void **argv);

	private:
		QPointer<QObject>		m_src;
		int									m_idx;
		QMetaMethod					m_method;
		void								**m_argv;
};


DeferredSignal::DeferredSignal(QObject *src, QMetaMethod method, int idx)
	:m_src(src), m_idx(idx), m_method(method), m_argv(NULL)
{
	//connect the signal to our omni-slot
	QMetaObject::connect(m_src, m_idx, this, m_idx);
}

tribool DeferredSignal::reset_me()
{
	this->resetState(indeterminate);
	return indeterminate;
}

void DeferredSignal::cancel_me()
{
	QObject::disconnect(this->m_src, NULL, this, NULL);
	this->deleteLater();
}

static inline QByteArray paramType(const QByteArray &ptype, bool *out)
{
	*out = ptype.endsWith('&') || ptype.endsWith("**");
	if (*out) {
		QByteArray res(ptype);
		res.truncate(res.length() - 1);
		return res;
	}

	return ptype;
}

QVariant DeferredSignal::result()
{
	if(!m_argv) {
		return QVariant();
	}

	//legacy from QObjectComWrapper
	const QMetaObject *mo = m_method.enclosingMetaObject();
	QList<QByteArray> ptypes = m_method.parameterTypes();
	int pcount = ptypes.count();

	QVariantList result;
	result.reserve(pcount);

	//prepare arguments
	for (int p = 0; p < pcount; ++p) {

		bool out;
		QByteArray ptype = paramType(ptypes.at(p), &out);
		QVariant variant;
		if (mo->indexOfEnumerator(ptype) != -1) {
			// convert enum values to int
			variant = QVariant(*reinterpret_cast<int *>(m_argv[p+1]));
		} else {
			QVariant::Type vt = QVariant::nameToType(ptype);
			if (vt == QVariant::UserType) {
				if (ptype.endsWith('*')) {
					variant = QVariant(QMetaType::type(ptype), (void**)m_argv[p+1]);
				} else {
					variant = QVariant(QMetaType::type(ptype), m_argv[p+1]);
				}
			} else {
				variant = QVariant(vt, m_argv[p + 1]);
			}
		}
		result.push_back(variant);
	}

	return result;
}

//omni-slot
int DeferredSignal::qt_metacall(QMetaObject::Call call, int index, void **argv)
{
	Q_UNUSED(call);
	Q_UNUSED(index);
	Q_ASSERT(call == QMetaObject::InvokeMetaMethod);
	Q_ASSERT(index == m_idx);

	Promise<QVariant> hold(DeferredBase::d_ptr);

	QList<QByteArray> ptypes = m_method.parameterTypes();
	int pcount = ptypes.count();

	if (pcount && !argv) {
		qWarning("SignalPromise::qt_metacall: Missing %d arguments", pcount);
		return -1;
	}

	//result is only available within resolve enclosing
	m_argv = argv;
	this->resolve(this->result());
	m_argv = NULL;

	return -1;
}

Promise<QVariant> signalPromise(QObject *src, const char *signal)
{
	if(src) {
		const QMetaObject *meta = src->metaObject();
		int idx = meta->indexOfSignal(signal + 1);
		if(idx == -1) {
			idx = meta->indexOfSignal(QMetaObject::normalizedSignature(signal + 1));
		}
		if(idx != -1) {
			QMetaMethod method = meta->method(idx);
			if(method.methodType() == QMetaMethod::Signal) {
				DeferredSignal *ds = new DeferredSignal(src, method, idx);
				return ds->promise();
			}
		}
	}
	ASSERT(false && "signal not found!");
	return false;
}
