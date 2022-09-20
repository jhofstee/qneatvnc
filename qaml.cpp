#include <qneatvnc/qaml.hpp>

// https://github.com/any1/aml Andri's mainloop running form the Qt mainloop.

QAml::QAml(QObject *parent) :
	QObject(parent)
{
	mAml = aml_new();
	if (!aml_get_default())
		aml_set_default(mAml);
}

QAml::~QAml()
{
	aml_unref(mAml);
}

void QAml::start()
{
	int fd = aml_get_fd(mAml);
	mNotifier = new QSocketNotifier(fd, QSocketNotifier::Read, this);
	connect(mNotifier, SIGNAL(activated(int)), SLOT(onActivated()));

	aml_poll(mAml, 0);
}

void QAml::onActivated()
{
	aml_poll(mAml, 0);
	aml_dispatch(mAml);
}
