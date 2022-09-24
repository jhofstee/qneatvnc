#pragma once

extern "C" {
	#include <neatvnc.h>

}

#include <QEvent>
#include <QObject>
#include <QWidget>

class QNVncServer;
class QNVncServerClient;
class QNVncDisplayWidget;

class QNVncDisplay : public QObject
{
	Q_OBJECT

public:
	explicit QNVncDisplay(QObject *parent = 0);
	virtual ~QNVncDisplay();

	struct nvnc_display *display() {
		return mDisplay;
	}

protected:
	virtual void handleKeyEvent(QNVncServerClient *client, Qt::Key key, QString const &str, bool pressed) = 0;
	virtual void handlePointerEvent(QNVncServerClient *client, QPoint &pos, Qt::MouseButton button, QEvent::Type type) = 0;

	virtual void setup() = 0;
	virtual void idle() = 0;

	struct nvnc_display *mDisplay;

	friend class QNVncServer;
};

/*
 * Rendering the contents of a Widget again after it got painted.
 * The cons, rendered twice, the pros, there is no need to know
 * where the output of the original painter went to.
 */

class QNVncDisplayWidget : public QNVncDisplay {
Q_OBJECT

public:
	QNVncDisplayWidget(QNVncServer *vnc, QWidget *widget,
					   QImage::Format format = QImage::Format_ARGB32,
					   QObject *parent = 0);

	QWidget *widget() { return mWidget; }

signals:
	void paintEvent();

protected:
	virtual bool eventFilter(QObject *watched, QEvent *e);
	void feedFrameBuffer(QImage &img, QList<QRect> damage = QList<QRect>());
	void handleKeyEvent(QNVncServerClient *client, Qt::Key key, QString const &str, bool pressed) override;
	void handlePointerEvent(QNVncServerClient *client, QPoint &pos, Qt::MouseButton button, QEvent::Type type) override;

	void setup() override;
	void idle() override;

private slots:
	void updateFrameBuffer();

private:
	QWidget *mWidget;
	QList<QRect> mDamaged;
	bool mRendering = false;
	QNVncServer *mServer;
	QImage::Format mFormat;
};

class QNVncServer : public QObject
{
Q_OBJECT

public:
	QNVncServer(QObject *parent = 0);

	struct nvnc *server() {
		return mServer;
	}

	~QNVncServer();

	void onKeyEvent(QNVncServerClient *client, Qt::Key key, const QString &str, bool pressed);
	void onPointerEvent(QNVncServerClient *client, QPoint &pnt, Qt::MouseButton button, QEvent::Type type);

	void addClient(QNVncServerClient *) {
		if (!mNumClients && mDisplay)
			mDisplay->setup();
		mNumClients++;
	}
	void removeClient(QNVncServerClient *) {
		mNumClients--;
		if (!mNumClients && mDisplay)
			mDisplay->idle();
	}
	int numberOfClients() { return mNumClients; }

protected:
	void setDisplay(QNVncDisplay *display);

private:
	struct nvnc *mServer;
	QNVncDisplay *mDisplay = nullptr;
	int mNumClients = 0;

	friend class QNVncDisplay;
	friend class QNVncDisplayWidget;
};
