#define XK_MISCELLANY

#include "keys.hpp"
#include <X11/keysymdef.h>

Qt::Key qtKeyFromX11(int x11key, QString &str)
{
	// latin-1
	if (x11key >= 0x20 && x11key <= 0xff) {
		str = QString(QChar(x11key));
		return Qt::Key_unknown;
	}

	// unicode
	if (x11key >= 0x01000000) {
		char32_t val = x11key - 0x01000000;
		str = QString::fromUcs4(&val, 1);
		return Qt::Key_unknown;
	}

	// special keys
	switch (x11key) {
	case XK_Shift_L:
	case XK_Shift_R:
		return Qt::Key_Shift;
	case XK_BackSpace:
		return Qt::Key_Backspace;
	case XK_Delete:
		return Qt::Key_Delete;
	case XK_Left:
		return Qt::Key_Left;
	case XK_Right:
		return Qt::Key_Right;
	case XK_Down:
		return Qt::Key_Down;
	case XK_Up:
		return Qt::Key_Up;
	case XK_Return:
		return Qt::Key_Return;
	case XK_Escape:
		return Qt::Key_Escape;
	case XK_Control_L:
	case XK_Control_R:
		return Qt::Key_Control;
	case XK_Alt_L:
	case XK_Alt_R:
		return Qt::Key_Alt;
	default:
		return Qt::Key_unknown;
	}
}
