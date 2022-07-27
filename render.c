#include "ns.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

static void x_render_page(HTML_elem*);

typedef struct {
  uint8_t bold;
  uint8_t italic;
  uint8_t paragraph;
} Text_attr;

void init_text_attr(Text_attr *attr) {
  attr->bold = 0;
  attr->italic = 0;
}

void get_text_attr(HTML_elem *el, Text_attr *attr) {
  if (el->t == BOLD) attr->bold = 1;
  if (el->t == ITALIC) attr->italic = 1;

  if (el->t != ROOT)
    get_text_attr(el->parent, attr);
}

void render_page(HTML_elem *page) {
  x_render_page(page);
}

#ifdef USE_X

#define fontname_n "DejaVu Sans Mono:size=7:antialias=true"
#define fontname_b "DejaVu Sans Mono:size=7:antialias=true:style=bold"
#define fontname_i "DejaVu Serif:size=7:antialias=true:style=Italic:hinting=true"
#define fontsz 7

#define padding 10

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>

#include <X11/Xft/Xft.h>

XftFont *font_n;
XftFont *font_b;
XftFont *font_i;

static void x_recursive_render_text(XftDraw *xd, XftColor *color,
    int *x, int *y, HTML_elem *el, int maxw, int maxh, int scroll, 
    int *curr_scroll) {
  int i,
      maxlen = (maxw - *x) / fontsz,
      bakx = *x;
  char *draw;
  Text_attr at;

  printf("maxlen = %d, x = %d\n", maxlen, *x);
  if (maxlen <= 0) {
    *y = *y + fontsz * 2;
    *x = padding;
    maxlen = (maxw - *x) / fontsz;
    if (maxlen <= 0)
      maxlen = 1;
    /* fuck off */
    printf("maxlen = %d, x = %d\n", maxlen, *x);
  }

  if (el->t == PARAGRAPH || el->t == BREAK_LINE) {
    *y = *y + fontsz * 2;
    *x = padding;
  }

  if (el->t == TEXT_TYPE) {
    init_text_attr(&at);
    get_text_attr(el, &at);

    draw = el->TT_val;

    while (*draw) {
      if (scroll)
        if (*curr_scroll) {
          printf("curr_scroll: %d\n", *curr_scroll);
          --*curr_scroll;
          draw += ((int)strlen(draw) > maxlen) ? maxlen : strlen(draw);
          break;
        }
      if (*y > maxh)
        return;
      if (at.bold)
        XftDrawStringUtf8(xd, color, font_b, *x, *y, (const FcChar8*)draw,
            ((int)strlen(draw) > maxlen) ? maxlen : strlen(draw));
      else if (at.italic)
        XftDrawStringUtf8(xd, color, font_i, *x, *y, (const FcChar8*)draw,
            ((int)strlen(draw) > maxlen) ? maxlen : strlen(draw));
      else
        XftDrawStringUtf8(xd, color, font_n, *x, *y, (const FcChar8*)draw,
            ((int)strlen(draw) > maxlen) ? maxlen : strlen(draw));

      draw += ((int)strlen(draw) > maxlen) ? maxlen : strlen(draw);
      *y = *y + 16;
      *x = padding;
    }

    *y = *y - 16;
    *x = bakx + (strlen(el->TT_val) * fontsz);

    /*XDrawString(d, w, DefaultGC(d, s), *x, *y, el->TT_val,*/
          /*strlen(el->TT_val));*/
  } else {
    for (i = 0; i < el->child_n; ++i)
      x_recursive_render_text(xd, color, x, y, &el->child[i], maxw, maxh,
          scroll, curr_scroll);
  }
}

static void x_init_fonts(Display *dpy, int s, Visual *visual, 
    Colormap cmap, const char *color_color, XftColor *color) {
  font_n = XftFontOpenName(dpy, s, fontname_n);
  font_b = XftFontOpenName(dpy, s, fontname_b);
  font_i = XftFontOpenName(dpy, s, fontname_i);

  if (!font_n)
    err("%s: couldn't load font %s", __FILE__, fontname_n);
  if (!font_b)
    err("%s: couldn't load font %s", __FILE__, fontname_b);
  if (!font_i)
    err("%s: couldn't load font %s", __FILE__, fontname_i);

  if (!XftColorAllocName(dpy, visual, cmap, color_color, color))
    err("%s: couldn't allocate xft color", __FILE__);

}

static void x_render_page(HTML_elem *page) {
  int s,
      width = 640,
      height = 480,
      x_now = padding,
      y_now = padding,
      force_expose = 0,
      scroll = 0,
      cscroll = 0;
  Window win;
  XEvent ev;
  XWindowAttributes wa;
  KeySym ks;
  Colormap cmap;
  Visual *visual;
  XftDraw *xd;
  XftColor color;

  Display *dpy = XOpenDisplay(NULL);
  if (!dpy) {
    err("%s: cannot open display", __FILE__);
    /* unreachable */
  }

  s = DefaultScreen(dpy);

  cmap = DefaultColormap(dpy, s);
  visual = DefaultVisual(dpy, s);

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

  x_init_fonts(dpy, s, visual, cmap, "#000000", &color);

  xd = XftDrawCreate(
    dpy,
    win,
    visual,
    cmap
  );

  while (1) {
    if (!force_expose)
      XNextEvent(dpy, &ev);

    if (ev.type == Expose || force_expose) {
      if (force_expose) {
        /*XFlush(dpy);*/
        XSync(dpy, True);
        force_expose = 0;
      }

      printf("scroll: %d\n", scroll);
      x_now = padding, y_now = padding;
      cscroll = scroll;

      XGetWindowAttributes(dpy, win, &wa);
      width = wa.width;
      height = wa.height;

      x_recursive_render_text(xd, &color, &x_now, &y_now, page, width, height,
          scroll, &cscroll);
    } else if (ev.type == KeyPress) {
      XLookupString(&ev.xkey, NULL, 0, &ks, NULL);

      switch (ks) {
        case XK_Escape:
          goto endloop;
        case XK_k:
          if (scroll)
            --scroll;
          force_expose = 1;
          break;
        case XK_j:
          ++scroll;
          force_expose = 1;
          break;
      }
    }
  }
  
endloop:

  XftColorFree(dpy, visual, cmap, &color);
  XftDrawDestroy(xd);
  XDestroyWindow(dpy, win);
  XCloseDisplay(dpy);
}

#endif

