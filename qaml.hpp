#pragma once

#include <QObject>
#include <QSocketNotifier>

extern "C" {
	#include <aml.h>
}

class QAml : public QObject
{
	Q_OBJECT

public:
	QAml(QObject *parent = 0);
	~QAml();

	void start();
	struct aml *aml() { return mAml; }

private slots:
	void onActivated();

private:
	struct aml *mAml;
	QSocketNotifier *mNotifier;
};
