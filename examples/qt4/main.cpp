#include <QApplication>
#include <QDeclarativeView>
#include <QDir>

#include <qneatvnc/qaml.hpp>
#include <qneatvnc/qneatvnc.hpp>

#define __DEFINE_STR(a)			#a
#define _DEFINE_STR(a)			__DEFINE_STR(a)

int main(int argc, char *argv[])
{
	QApplication app(argc, argv);
	QAml aml;

	QDeclarativeView *view = new QDeclarativeView;
	view->setSource(QString(_DEFINE_STR(PWD)) + QDir::separator() + "main.qml");
	view->show();

	QNVncServer server("0.0.0.0");
	// NOTE: the viewport is needed here..
	QNVncDisplayWidget display(&server, view->viewport());
	Q_UNUSED(display);
	aml.start();

	return app.exec();
}
