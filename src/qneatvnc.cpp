#include <libdrm/drm_fourcc.h>
#include <pixman.h>

#include <QApplication>
#include <QDebug>
#include <QEvent>
#include <QPainter>
#include <QPaintEvent>

#include <qneatvnc/qneatvnc.hpp>
#include "keys.hpp"

static uint32_t drmFormat(QImage::Format format)
{
	switch (format) {
	case QImage::Format_ARGB32:
		return DRM_FORMAT_ARGB8888;
	default:
		qFatal("Unsupported image format!!!");
		return DRM_FORMAT_INVALID;
	}
}

class QNVncFb
{
public:
	explicit QNVncFb(QImage &img);

	struct nvnc_fb *fb() { return mFb; }

private:
	struct nvnc_fb *mFb;
	QImage mImg;
};

static void freeFrameBuffer(struct nvnc_fb *fb, void *context)
{
	Q_UNUSED(fb);

	QNVncFb *qFb = static_cast<QNVncFb *>(context);
	delete qFb;
}

QNVncFb::QNVncFb(QImage &img)
{
	mImg = img;
	mFb = nvnc_fb_from_buffer((void *) mImg.constScanLine(0), img.width(),
							  img.height(), drmFormat(img.format()),
							  img.width());
	nvnc_fb_set_release_fn(mFb, freeFrameBuffer, this);
}

// A client connected to the VNC server, not a VNC client app.
class QNVncServerClient {
public:
	explicit QNVncServerClient(struct nvnc_client *client) :
		mClient(client)
	{}

	Qt::MouseButtons mouseButtons() { return mMouseButtons; }
	void setMouseButtons(Qt::MouseButtons buttons) { mMouseButtons = buttons; }

	struct nvnc_client *client() { return mClient; }

private:
	Qt::MouseButtons mMouseButtons = Qt::NoButton;
	struct nvnc_client *mClient;
};

QNVncDisplay::QNVncDisplay(QObject *parent) :
	QObject(parent)
{
	mDisplay = nvnc_display_new(0, 0);
}

QNVncDisplay::~QNVncDisplay() {
	nvnc_display_unref(mDisplay);
}

QNVncDisplayWidget::QNVncDisplayWidget(QNVncServer *vnc, QWidget *widget, QImage::Format format, QObject *parent)
	: QNVncDisplay(parent),
	  mWidget(widget),
	  mServer(vnc),
	  mFormat(format)
{
	nvnc_add_display(vnc->server(), mDisplay);

	// Queued so there is no recursion during paint.
	connect(this, SIGNAL(paintEvent()), SLOT(updateFrameBuffer()), Qt::QueuedConnection);

	vnc->setDisplay(this);

	idle();
}

void QNVncDisplayWidget::setup()
{
	mWidget->installEventFilter(this);
	updateFrameBuffer();
}

void QNVncDisplayWidget::idle()
{
	QImage dummy(1, 1, mFormat);
	dummy.fill(Qt::gray);
	feedFrameBuffer(dummy);
	mWidget->removeEventFilter(this);
}

void QNVncDisplayWidget::feedFrameBuffer(QImage &img, QList<QRect> damage)
{
	pixman_box16_t boxes[damage.count()];
	int n = 0;

	for (QRect &rec: damage) {
		boxes[n].x1 = rec.x();
		boxes[n].y1 = rec.y();
		boxes[n].x2 = rec.right();
		boxes[n].y2 = rec.bottom();
		n++;
	}

	QNVncFb *qFb = new QNVncFb(img);

	struct pixman_region16 pixmanDamage;
	if (damage.count())
		pixman_region_init_rects(&pixmanDamage, boxes, n);
	else
		pixman_region_init_rect(&pixmanDamage, 0, 0, img.width(), img.height());
	nvnc_display_feed_buffer(mDisplay, qFb->fb(), &pixmanDamage);
	pixman_region_fini(&pixmanDamage);
}

void QNVncDisplayWidget::handleKeyEvent(QNVncServerClient *client, Qt::Key key, const QString &str, bool pressed)
{
	Q_UNUSED(client);

	QEvent::Type type = (pressed ? QEvent::KeyPress : QEvent::KeyRelease);
	QKeyEvent event = QKeyEvent(type, key, Qt::NoModifier, str);
	qApp->sendEvent(mWidget, &event);
}

void QNVncDisplayWidget::handlePointerEvent(QNVncServerClient *client, QPoint &pos, Qt::MouseButton button, QEvent::Type type)
{
	QMouseEvent event = QMouseEvent(type, pos, mWidget->mapToGlobal(pos),
									button, client->mouseButtons(), Qt::NoModifier);
	QApplication::sendEvent(mWidget, &event);
}

bool QNVncDisplayWidget::eventFilter(QObject *watched, QEvent *e)
{
	Q_UNUSED(watched);

	if (e->type() == QEvent::Paint && !mRendering) {
		QPaintEvent *paintArgs = static_cast<QPaintEvent *>(e);
		mDamaged.append(paintArgs->rect());
		emit paintEvent();
	}

	return false;
}

void QNVncDisplayWidget::updateFrameBuffer()
{
	QImage img(mWidget->size(), mFormat);

	mRendering = true;
	mWidget->render(&img);
	mRendering = false;

	feedFrameBuffer(img, mDamaged);
	mDamaged.clear();
}

void QNVncServer::onPointerEvent(QNVncServerClient *client, QPoint &pnt, Qt::MouseButton button, QEvent::Type type)
{
	if (mDisplay)
		mDisplay->handlePointerEvent(client, pnt, button, type);
}

static void onNvncPointerEvent(struct nvnc_client *client, uint16_t x, uint16_t y,
								enum nvnc_button_mask mask)
{
	struct nvnc *server = nvnc_client_get_server(client);
	QNVncServer *qServer = static_cast<QNVncServer *>(nvnc_get_userdata(server));
	QNVncServerClient *qClient = static_cast<QNVncServerClient *>(nvnc_get_userdata(client));

	Qt::MouseButtons mouseButtons = Qt::NoButton;
	if (mask & NVNC_BUTTON_LEFT)
		mouseButtons |= Qt::LeftButton;
	if (mask & NVNC_BUTTON_MIDDLE )
		mouseButtons |= Qt::MiddleButton;
	if (mask & NVNC_BUTTON_RIGHT)
		mouseButtons |= Qt::RightButton;

	QPoint pos = QPoint(x, y);

	Qt::MouseButtons changed = qClient->mouseButtons() ^ mouseButtons;
	qClient->setMouseButtons(mouseButtons);

	if (!changed) {
		qServer->onPointerEvent(qClient, pos, Qt::NoButton, QEvent::MouseMove);
	} else {
		unsigned button = Qt::LeftButton;
		while (changed) {
			if (changed & button) {
				QEvent::Type type = (mouseButtons & button ? QEvent::MouseButtonPress : QEvent::MouseButtonRelease);
				qServer->onPointerEvent(qClient, pos, (Qt::MouseButton) button, type);
				changed &= ~button;
			}
			button <<= 1;
		}
	}
}

void QNVncServer::onKeyEvent(QNVncServerClient *client, Qt::Key key, QString const &str, bool is_pressed)
{
	if (mDisplay)
		mDisplay->handleKeyEvent(client, key, str, is_pressed);
}

static void onNvncKeyEvent(struct nvnc_client *client, uint32_t x11key, bool is_pressed)
{
	struct nvnc *server = nvnc_client_get_server(client);
	QNVncServer *qServer = static_cast<QNVncServer *>(nvnc_get_userdata(server));
	QNVncServerClient *qClient = static_cast<QNVncServerClient *>(nvnc_get_userdata(client));

	QString text;
	Qt::Key key = qtKeyFromX11(x11key, text);
	qServer->onKeyEvent(qClient, key, text, is_pressed);
}

static void deleteClient(void *userdata)
{
	QNVncServerClient *qClient = static_cast<QNVncServerClient *>(userdata);

	struct nvnc *server = nvnc_client_get_server(qClient->client());
	QNVncServer *qServer = static_cast<QNVncServer *>(nvnc_get_userdata(server));
	qServer->removeClient(qClient);

	delete qClient;
}

static void onNvncNewClient(struct nvnc_client *client)
{
	QNVncServerClient *qClient = new QNVncServerClient(client);
	nvnc_set_userdata(client, qClient, deleteClient);

	struct nvnc *server = nvnc_client_get_server(client);
	QNVncServer *qServer = static_cast<QNVncServer *>(nvnc_get_userdata(server));
	qServer->addClient(qClient);

}

QNVncServer::QNVncServer(QObject *parent) :
	QObject(parent)
{
	mServer = nvnc_open("127.0.0.1", 5900);

	nvnc_set_userdata(mServer, this, NULL);
	nvnc_set_new_client_fn(mServer, onNvncNewClient);
	nvnc_set_pointer_fn(mServer, onNvncPointerEvent);
	nvnc_set_key_fn(mServer, onNvncKeyEvent);
}

QNVncServer::~QNVncServer()
{
	nvnc_close(mServer);
}

void QNVncServer::setDisplay(QNVncDisplay *display)
{
	if (mDisplay) {
		qWarning() << "Only one display can be added";
		return;
	}
	mDisplay = display;
}
