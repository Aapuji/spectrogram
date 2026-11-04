#include <spectrogram/platform.hpp>

#include <X11/Xlib.h>
#include <X11/Xatom.h>

void platform::maximize_window(sf::WindowHandle handle) {
    Display* display = XOpenDisplay(nullptr);
    if (!display) return;

    Atom wmState   = XInternAtom(display, "_NET_WM_STATE", False);
    Atom wmMaxHorz = XInternAtom(display, "_NET_WM_STATE_MAXIMIZED_HORZ", False);
    Atom wmMaxVert = XInternAtom(display, "_NET_WM_STATE_MAXIMIZED_VERT", False);

    XEvent xev{};
    xev.type = ClientMessage;
    xev.xclient.window = handle;
    xev.xclient.message_type = wmState;
    xev.xclient.format = 32;
    xev.xclient.data.l[0] = 1;  // _NET_WM_STATE_ADD
    xev.xclient.data.l[1] = wmMaxHorz;
    xev.xclient.data.l[2] = wmMaxVert;
    xev.xclient.data.l[3] = 1;

    XSendEvent(display, DefaultRootWindow(display), False,
               SubstructureNotifyMask | SubstructureRedirectMask, &xev);
    XFlush(display);
    XCloseDisplay(display);
}
