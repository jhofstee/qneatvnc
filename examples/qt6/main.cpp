#include <QApplication>
#include <QQuickWidget>

#include <qneatvnc/qaml.hpp>
#include <qneatvnc/qneatvnc.hpp>

int main(int argc, char *argv[])
{
	QApplication app(argc, argv);
	QAml aml;

	QQuickWidget *view = new QQuickWidget;
	view->setSource(u"qrc:/qt6qneatvnc/main.qml"_qs);
	view->show();

	QNVncServer server;
	QNVncDisplayWidget display(&server, view);
	aml.start();

	return app.exec();
}
