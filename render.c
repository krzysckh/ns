#include "ns.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include <time.h>

static void x_render_page(HTML_elem*);

typedef struct {
  uint8_t bold;
  uint8_t italic;
  uint8_t paragraph;

  uint8_t h1;
  uint8_t h2;
  uint8_t h3;
  uint8_t h4;
  uint8_t h5;
  uint8_t h6;
} Text_attr;

void init_text_attr(Text_attr *attr) {
  attr->bold = 0;
  attr->italic = 0;

  attr->h1 = 0;
  attr->h2 = 0;
  attr->h3 = 0;
  attr->h4 = 0;
  attr->h5 = 0;
  attr->h6 = 0;
}

void get_text_attr(HTML_elem *el, Text_attr *attr) {
  if (el->t == BOLD) attr->bold = 1;
  if (el->t == ITALIC) attr->italic = 1;

  if (el->t == H1) attr->h1 = 1;
  if (el->t == H2) attr->h2 = 1;
  if (el->t == H3) attr->h3 = 1;
  if (el->t == H4) attr->h4 = 1;
  if (el->t == H5) attr->h5 = 1;
  if (el->t == H6) attr->h6 = 1;

  if (el->t != ROOT)
    get_text_attr(el->parent, attr);
}

void render_page(HTML_elem *page) {
  x_render_page(page);
}

#ifdef USE_X

#define font_t "Dejavu Sans Mono"
#define fontname_n "DejaVu Sans Mono:size=8:antialias=true"
#define fontname_b "DejaVu Sans Mono:size=8:antialias=true:style=bold"
#define fontname_i "DejaVu Serif:size=8:antialias=true:style=Italic:hinting=true"
#define fontsz 8

#define h1_sz "20"
#define h2_sz "15"
#define h3_sz "10"
#define h4_sz "8"
#define h5_sz "6"
#define h6_sz "4"

#define padding 10

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>

#include <X11/Xft/Xft.h>

XftFont *font_n;
XftFont *font_b;
XftFont *font_i;

static void x_load_render_destroy(const char *fontn, const char *txt,
    const char *color, int x, int y, int len, Display *dpy, XftDraw *xd) {
  int s = DefaultScreen(dpy);
  XftColor c;

  XftFont *fnt = XftFontOpenName(dpy, s, fontn);

  if (!fnt)
    err("%s: couldn't load font %s", __FILE__, fontn);

  if (!XftColorAllocName(dpy, DefaultVisual(dpy, s), 
        DefaultColormap(dpy, s), color, &c))
    err("%s: couldn't allocate xft color", __FILE__);

  XftDrawStringUtf8(xd, &c, fnt, x, y, (const FcChar8*)txt, len);
  XftColorFree(dpy, DefaultVisual(dpy, s), DefaultColormap(dpy, s), &c);
}

static void x_recursive_render_text(Display *dpy, XftDraw *xd, XftColor *color,
    int *x, int *y, HTML_elem *el, int maxw, int maxh, int scroll, 
    int *curr_scroll) {
  int i,
      maxlen = (maxw - *x) / fontsz,
      bakx = *x;
  char *draw;
  Text_attr at;

  if (maxlen <= 0) {
    *y = *y + fontsz * 2;
    *x = padding;
    maxlen = (maxw - *x) / fontsz;
    if (maxlen <= 0)
      maxlen = 1;
    /* fuck off */
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
#if 0
      if (scroll)
        if (*curr_scroll) {
          printf("curr_scroll: %d\n", *curr_scroll);
          --*curr_scroll;
          draw += ((int)strlen(draw) > maxlen) ? maxlen : strlen(draw);
          break;
        }
#endif
      if (*y > maxh)
        return;
      if (at.bold)
        XftDrawStringUtf8(xd, color, font_b, *x, *y, (const FcChar8*)draw,
            ((int)strlen(draw) > maxlen) ? maxlen : strlen(draw));
      else if (at.italic)
        XftDrawStringUtf8(xd, color, font_i, *x, *y, (const FcChar8*)draw,
            ((int)strlen(draw) > maxlen) ? maxlen : strlen(draw));
      else if (at.h1) {
        *y = *y + (2*atoi(h1_sz));
        x_load_render_destroy(font_t":size="h1_sz":style=bold", draw,
            "#000000", *x, *y,
          ((int)strlen(draw) > maxlen) ? maxlen : strlen(draw), dpy, xd);
        *y = *y + (2*atoi(h1_sz));
      } else if (at.h2) {
        *y = *y + (2*atoi(h2_sz));
        x_load_render_destroy(font_t":size="h2_sz":style=bold", draw,
            "#000000", *x, *y,
          ((int)strlen(draw) > maxlen) ? maxlen : strlen(draw), dpy, xd);
        *y = *y + (2*atoi(h2_sz));
      } else if (at.h3) {
        *y = *y + (2*atoi(h3_sz));
        x_load_render_destroy(font_t":size="h3_sz":style=bold", draw,
            "#000000", *x, *y,
          ((int)strlen(draw) > maxlen) ? maxlen : strlen(draw), dpy, xd);
        *y = *y + (2*atoi(h3_sz));
      } else if (at.h4) {
        *y = *y + (2*atoi(h4_sz));
        x_load_render_destroy(font_t":size="h4_sz":style=bold", draw,
            "#000000", *x, *y,
          ((int)strlen(draw) > maxlen) ? maxlen : strlen(draw), dpy, xd);
        *y = *y + (2*atoi(h4_sz));
      } else if (at.h5) {
        *y = *y + (2*atoi(h5_sz));
        x_load_render_destroy(font_t":size="h5_sz":style=bold", draw,
            "#000000", *x, *y,
          ((int)strlen(draw) > maxlen) ? maxlen : strlen(draw), dpy, xd);
        *y = *y + (2*atoi(h5_sz));
      } else if (at.h6) {
        *y = *y + (2*atoi(h6_sz));
        x_load_render_destroy(font_t":size="h6_sz":style=bold", draw,
            "#000000", *x, *y,
          ((int)strlen(draw) > maxlen) ? maxlen : strlen(draw), dpy, xd);
        *y = *y + (2*atoi(h6_sz));
      }
      else
        XftDrawStringUtf8(xd, color, font_n, *x, *y, (const FcChar8*)draw,
            ((int)strlen(draw) > maxlen) ? maxlen : strlen(draw));

      draw += ((int)strlen(draw) > maxlen) ? maxlen : strlen(draw);
      *y = *y + fontsz * 2;
      *x = padding;
    }

    *y = *y - fontsz * 2;
    if (at.h1 || at.h2 || at.h3 || at.h4 || at.h5 || at.h6)
      *x = padding;
    else 
      *x = bakx + (strlen(el->TT_val) * fontsz);

    /*XDrawString(d, w, DefaultGC(d, s), *x, *y, el->TT_val,*/
          /*strlen(el->TT_val));*/
  } else {
    for (i = 0; i < el->child_n; ++i)
      x_recursive_render_text(dpy, xd, color, x, y, &el->child[i], maxw, maxh,
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
  clock_t time_start,
          time_end;

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
#if 0
      if (force_expose) {
        /*XFlush(dpy);*/
        XSync(dpy, True);
        force_expose = 0;
      }
#endif

      x_now = padding, y_now = padding;
      cscroll = scroll;

      XGetWindowAttributes(dpy, win, &wa);
      width = wa.width;
      height = wa.height;

      time_start = clock();
      x_recursive_render_text(dpy, xd, &color, &x_now, &y_now, page, width,
          height, scroll, &cscroll);
      time_end = clock();
      fprintf(stderr, "%s: x_recursive_render_text() -> took %6.4f\n",
          __FILE__, (double)(time_end - time_start) / CLOCKS_PER_SEC);


    } else if (ev.type == KeyPress) {
      XLookupString(&ev.xkey, NULL, 0, &ks, NULL);

      switch (ks) {
        case XK_Escape:
          goto endloop;
#if 0
        case XK_k:
          if (scroll)
            --scroll;
          force_expose = 1;
          break;
        case XK_j:
          ++scroll;
          force_expose = 1;
          break;
#endif
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

