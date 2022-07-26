#include "ns.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

typedef struct {
  uint8_t bold;
} Text_attr;

void init_text_attr(Text_attr *attr) {
  attr->bold = 0;
}

void get_text_attr(HTML_elem *el, Text_attr *attr) {
  if (el->t == BOLD) attr->bold = 1;

  if (el->t != ROOT)
    get_text_attr(el->parent, attr);
}

#ifdef USE_X

#define fontname "DejaVu Sans Mono:size=7:antialias=true"

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>

#include <X11/Xft/Xft.h>

void x_recursive_render_text(XftDraw *xd, XftColor *color, XftFont *font,
    int *x, int *y, HTML_elem *el) {
  int i;
  Text_attr at;

  if (el->t == TEXT_TYPE) {
    init_text_attr(&at);
    get_text_attr(el, &at);
    XftDrawStringUtf8(xd, color, font, *x, *y, (const FcChar8*)el->TT_val,
        strlen(el->TT_val));

    /*XDrawString(d, w, DefaultGC(d, s), *x, *y, el->TT_val,*/
          /*strlen(el->TT_val));*/
    *y = *y + 16;
  } else {
    for (i = 0; i < el->child_n; ++i)
      x_recursive_render_text(xd, color, font, x, y, &el->child[i]);
  }
}

void render_page(HTML_elem *page) {
  int s,
      width = 640,
      height = 480,
      x_now = 10,
      y_now = 10;
  Window win;
  XEvent ev;
  XWindowAttributes wa;
  KeySym ks;
  Colormap cmap;
  Visual *visual;
  XftDraw *xd;
  XftFont *font;
  XftColor color;

  Display *dpy = XOpenDisplay(NULL);
  if (!dpy) {
    err("%s: cannot open display", __FILE__);
    /* unreachable */
  }

  s = DefaultScreen(dpy);

  cmap = DefaultColormap(dpy, 0);
  visual = DefaultVisual(dpy, 0);

  win = XCreateSimpleWindow(
    dpy,
    RootWindow(dpy, s),
    0,
    0,
    width,
    height,
    1,
    BlackPixel(dpy, s),
    WhitePixel(dpy, s)
  );

  XSelectInput(dpy, win, ExposureMask | KeyPressMask);
  XMapWindow(dpy, win);

  XStoreName(dpy, win, "netskater");

  font = XftFontOpenName(dpy, s, fontname);
  if (!font)
    err("%s: couldn't load font %s", __FILE__, fontname);
  if (!XftColorAllocName(dpy, visual, cmap, "#000000", &color))
    err("%s: couldn't allocate xft color", __FILE__);

  xd = XftDrawCreate(
    dpy,
    win,
    visual,
    cmap
  );

  while (1) {
    XNextEvent(dpy, &ev);

    if (ev.type == Expose) {
      x_now = 10, y_now = 10;
      x_recursive_render_text(xd, &color, font, &x_now, &y_now, page);

      XGetWindowAttributes(dpy, win, &wa);
      width = wa.width;
      height = wa.height;
    } else if (ev.type == KeyPress) {
      XLookupString(&ev.xkey, NULL, 0, &ks, NULL);

      if (ks == XK_Escape)
        break;
    }
  }

  XftColorFree(dpy, visual, cmap, &color);
  XftDrawDestroy(xd);
  XDestroyWindow(dpy, win);
  XCloseDisplay(dpy);
}

#endif

