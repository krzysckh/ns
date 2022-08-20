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

  uint8_t table;
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

  attr->table = 0;

  attr->paragraph = 0;
}

void get_text_attr(HTML_elem *el, Text_attr *attr, int forwards) {
  int i;

  if (el->t == BOLD) attr->bold = 1;
  if (el->t == ITALIC) attr->italic = 1;

  if (el->t == H1) attr->h1 ++;
  if (el->t == H2) attr->h2 ++;
  if (el->t == H3) attr->h3 ++;
  if (el->t == H4) attr->h4 ++;
  if (el->t == H5) attr->h5 ++;
  if (el->t == H6) attr->h6 ++;

  if (el->t == TABLE) attr->table++;

  if (el->t == PARAGRAPH) attr->paragraph++;

  if (forwards)
    for (i = 0; i < el->child_n; ++i)
      get_text_attr(&el->child[i], attr, 1);
  else
    if (el->t != ROOT)
      get_text_attr(el->parent, attr, 0);
}

void render_page(HTML_elem *page) {
  x_render_page(page);
}

#ifdef USE_X

#define scroll_pixels 30

#define font_t "Dejavu Sans Mono"
#define fontname_n "DejaVu Sans Mono:pixelsize=15:antialias=true"
#define fontname_b "DejaVu Sans Mono:pixelsize=15:antialias=true:style=bold"
#define fontname_i "DejaVu Serif:pixelsize=15:antialias=true:style=Italic:hinting=true"
#define fontsz 15

#define fontratio 1.5

#define h1_sz "40"
#define h2_sz "30"
#define h3_sz "25"
#define h4_sz "16"
#define h5_sz "13"
#define h6_sz "9"

#define padding 10

#define table_bwidth 1

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>

#include <X11/Xft/Xft.h>

XftFont *font_n;
XftFont *font_b;
XftFont *font_i;

static int fuck_you_this_is_bak_x = padding;

static void x_recursive_render_text(Display *dpy, XftDraw *xd, XftColor *color,
    int *x, int *y, HTML_elem *el, int maxw, int maxh, int use_padding);
static int x_table_approx_height(HTML_elem *el, int width, int x);

static int x_get_maxlen(int maxw, int *x, int *y, int use_padding, int bakx,
    int l_fontsz, double l_fontratio, int fix_xy) {
  int ret = (maxw - *x) / (l_fontsz / l_fontratio);

  if (ret <= 0 && fix_xy) {
    *y = *y + l_fontsz * 2;
    *x = (use_padding) ? padding : bakx;
    ret = (maxw - *x) / (l_fontsz / l_fontratio);
    if (ret <= 0)
      ret = 1;
    /* fuck off */
  }
  
  return ret;
}
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

/* needed for x_table_approx_height */
static int x_get_full_child_text_len(HTML_elem *el) {
  int ret = 0,
      i;
  for (i = 0; i < el->child_n; ++i)
    if (el->child[i].t == TEXT_TYPE)
      ret += strlen(el->child[i].TT_val);
    else
      ret += x_get_full_child_text_len(&el->child[i]);

  return ret;
}

/* (recursive)
 * returns height of table in table
 */
int x_table_get_table_in_table_height(HTML_elem *el, int width, int x) {
  int i,
      j,
      ret = 0;
  for (i = 0; i < el->child_n; ++i) {
    if (el->child[i].t == TABLE)
      for (j = 0; j < el->child[i].child_n; ++j) {
        ret += x_table_approx_height(&el->child[i].child[j], width, x);
            x_table_approx_height(&el->child[i].child[j], width, x);
      }
    else
      ret += x_table_get_table_in_table_height(&el->child[i], width, x);
  }

  return ret;
}

/* approximates how big should a table row be */
/* and does a bad job at it :)) */
static int x_table_approx_height(HTML_elem *el, int width, int x) {
  /*return 30;*/
  int i,
      table_in_table,
      ret = 0,
      cur_child_len,
      longest = 0;
  Text_attr ta;

  for (i = 0; i < el->child_n; ++i) {
    if (el->child[i].t == TABLE_TD || el->child[i].t == TABLE_TH) {
      ret = (2 * fontsz);

      init_text_attr(&ta);
      get_text_attr(&el->child[i], &ta, 1);

      ret += (ta.h1 * atoi(h1_sz));
      ret += (ta.h2 * atoi(h2_sz));
      ret += (ta.h3 * atoi(h3_sz));
      ret += (ta.h4 * atoi(h4_sz));
      ret += (ta.h5 * atoi(h5_sz));
      ret += (ta.h6 * atoi(h6_sz));

      ret += (ta.paragraph * (fontsz * 2));

      table_in_table = x_table_get_table_in_table_height(el, width, x);
      /*if (table_in_table)*/
        /*ret += (padding * 2);*/
      ret += table_in_table;

      cur_child_len = x_get_full_child_text_len(&el->child[i]);
/* TODO: change this to use x_get_maxlen() */
      while ((width - x) / (fontsz / fontratio) < cur_child_len) {
        ret += (2*fontsz);
        cur_child_len -= ((width - x) / (fontsz / fontratio));
      }

      longest = (longest > ret) ? longest : ret;
    }
  }

  ret = longest;

  return ret;
}

static void x_render_table_row(Display *dpy, XftDraw *xd, XftColor *color,
    int *x, int *y, HTML_elem *el, int maxw, int maxh) {
  int t_width,
      t_height,
      i,
      c_count = 0,
      bak_x = *x,
      bak_y = *y,
      rendered_c = 0;

  for (i = 0; i < el->child_n; ++i)
    if (el->child[i].t == TABLE_TD || el->child[i].t == TABLE_TH)
      ++c_count;

  if (c_count == 0) {
    warn("%s: x_render_table_row(): c_count = 0 - no children of table %p",
        __FILE__, el->parent);
    return;
  }


  t_width = (maxw - padding - *x - (table_bwidth * c_count)) / c_count;
  t_height = x_table_approx_height(el, t_width, *x);

  XftDrawRect(xd, color, *x, *y, maxw - *x - padding, table_bwidth);
  for (i = 0; i < c_count; ++i)
    XftDrawRect(xd, color, *x + (i * t_width) + table_bwidth, *y, table_bwidth,
        t_height);
  XftDrawRect(xd, color, *x + (c_count * t_width), *y, table_bwidth, t_height);

  for (i = 0; i < el->child_n; ++i) {
    *y += fontsz;

    if (el->child[i].t == TABLE_TD || el->child[i].t == TABLE_TH) {
      fuck_you_this_is_bak_x = *x;
      x_recursive_render_text(dpy, xd, color, x, y, &el->child[i],
          *x + t_width, *y + t_height, 0);

      ++rendered_c;

      *y = bak_y;
      *x = bak_x + (rendered_c * t_width);
    }
  }

  *y += t_height;
  *x = padding;
  XftDrawRect(xd, color, *x, *y, maxw - *x - padding, table_bwidth);
  
  fuck_you_this_is_bak_x = padding;
}

static void x_render_table(Display *dpy, XftDraw *xd, XftColor *color, int *x,
    int *y, HTML_elem *table_root, int maxw, int maxh) {
  /**y += padding;*/
  *x = fuck_you_this_is_bak_x;
  /* x is set here, and at the end of x_render_table_row() */

  int i;

  for (i = 0; i < table_root->child_n; ++i)
    if (table_root->child[i].t != TABLE_TR)
      warn("%s: child of table %p at %p is not type TABLE_TR",
          __FILE__, table_root, &table_root->child[i]);
    else {
      x_render_table_row(dpy, xd, color, x, y, &table_root->child[i], maxw,
        maxh);
    }
}

static void x_render_image(HTML_elem *el, int *x, int *y, Display *dpy, 
    XftDraw *xd) {
  *y += 40;
  x_load_render_destroy("Comic Sans MS:pixelsize=20", "[IMAGE]",
    "#ff00ff", *x, *y, 7, dpy, xd);

  *x = fuck_you_this_is_bak_x;
  *y += 40;
}

static void x_recursive_render_text(Display *dpy, XftDraw *xd, XftColor *color,
    int *x, int *y, HTML_elem *el, int maxw, int maxh, int use_padding) {
  int i,
      maxlen, 
      bakx = *x;
  char *draw;
  Text_attr at;

  calculate_css(el);

  if (el->t == TABLE) {
    x_render_table(dpy, xd, color, x, y, el, maxw, maxh);
    return;
  } else if (el->t == LIST_ELEM) {
    if (el->parent->t != ORDERED_LIST && el->parent->t != UNORDERED_LIST) {
      warn("%s: parent of %p (%p = %s) is neither ORDERED_LIST nor "
          "UNORDERED_LIST -- ignoring this branch", __FILE__, el,
          el->parent, elemt_to_str(el->parent->t));
      return ;
    }
  } else if (el->t == IMAGE) {
    x_render_image(el, x, y, dpy, xd);
  } else if (el->t == PARAGRAPH 
      || el->t == BREAK_LINE
      || el->t == H1
      || el->t == H2
      || el->t == H3
      || el->t == H4
      || el->t == H5
      || el->t == H6
      ) {
    if (use_padding)
      *y = *y + (fontsz * 2);
    *x = (use_padding) ? padding : fuck_you_this_is_bak_x;
  } else if (el->t == STYLE || el->t == SCRIPT) {
    return;
  } else if (el->t == TEXT_TYPE) {
    init_text_attr(&at);
    get_text_attr(el, &at, 0);

    if (at.table > 1) {
      warn("%s: tables in tables not supported!", __FILE__);
      /*return;*/
    }

    draw = el->TT_val;
    

    while (*draw) {
      if (*y > maxh) {
#ifdef DUMB_WARNINGS
        warn("%s: couldn't draw text \"%s\" (%p) at [%d, %d] (no y space)",
            __FILE__, draw, draw, *x, *y);
#endif
        return;
      }
      if (*y < 0) {
#ifdef DUMB_WARNINGS
        warn("%s: couldn't draw text \"%s\" (%p) at [%d, %d] (y < 0)",
            __FILE__, draw, draw, *x, *y);
#endif
        return;
      }

      if (at.h1) maxlen = x_get_maxlen(maxw, x, y, use_padding, bakx,
          atoi(h1_sz), fontratio, 1);
      else if (at.h2) maxlen = x_get_maxlen(maxw, x, y, use_padding, bakx,
          atoi(h2_sz), fontratio, 1);
      else if (at.h3) maxlen = x_get_maxlen(maxw, x, y, use_padding, bakx,
          atoi(h3_sz), fontratio, 1);
      else if (at.h4) maxlen = x_get_maxlen(maxw, x, y, use_padding, bakx,
          atoi(h4_sz), fontratio, 1);
      else if (at.h5) maxlen = x_get_maxlen(maxw, x, y, use_padding, bakx,
          atoi(h5_sz), fontratio, 1);
      else if (at.h6) maxlen = x_get_maxlen(maxw, x, y, use_padding, bakx,
          atoi(h6_sz), fontratio, 1);
      else
        maxlen = x_get_maxlen(maxw, x, y, use_padding, bakx, fontsz, fontratio, 
            1);

      if (at.bold)
        XftDrawStringUtf8(xd, color, font_b, *x, *y, (const FcChar8*)draw,
            ((int)strlen(draw) > maxlen) ? maxlen : (int)strlen(draw));
      else if (at.italic)
        XftDrawStringUtf8(xd, color, font_i, *x, *y, (const FcChar8*)draw,
            ((int)strlen(draw) > maxlen) ? maxlen : (int)strlen(draw));
      else if (at.h1) {
        *y = *y + atoi(h1_sz);
        x_load_render_destroy(font_t":pixelsize="h1_sz":style=bold", draw,
            "#000000", *x, *y,
          ((int)strlen(draw) > maxlen) ? maxlen : (int)strlen(draw), dpy, xd);
        *y = *y + atoi(h1_sz);
      } else if (at.h2) {
        *y = *y + atoi(h2_sz);
        x_load_render_destroy(font_t":pixelsize="h2_sz":style=bold", draw,
            "#000000", *x, *y,
          ((int)strlen(draw) > maxlen) ? maxlen : (int)strlen(draw), dpy, xd);
        *y = *y + atoi(h2_sz);
      } else if (at.h3) {
        *y = *y + atoi(h3_sz);
        x_load_render_destroy(font_t":pixelsize="h3_sz":style=bold", draw,
            "#000000", *x, *y,
          ((int)strlen(draw) > maxlen) ? maxlen : (int)strlen(draw), dpy, xd);
        *y = *y + atoi(h3_sz);
      } else if (at.h4) {
        *y = *y + atoi(h4_sz);
        x_load_render_destroy(font_t":pixelsize="h4_sz":style=bold", draw,
            "#000000", *x, *y,
          ((int)strlen(draw) > maxlen) ? maxlen : (int)strlen(draw), dpy, xd);
        *y = *y + atoi(h4_sz);
      } else if (at.h5) {
        *y = *y + atoi(h5_sz);
        x_load_render_destroy(font_t":pixelsize="h5_sz":style=bold", draw,
            "#000000", *x, *y,
          ((int)strlen(draw) > maxlen) ? maxlen : (int)strlen(draw), dpy, xd);
        *y = *y + atoi(h5_sz);
      } else if (at.h6) {
        *y = *y + atoi(h6_sz);
        x_load_render_destroy(font_t":pixelsize="h6_sz":style=bold", draw,
            "#000000", *x, *y,
          ((int)strlen(draw) > maxlen) ? maxlen : (int)strlen(draw), dpy, xd);
        *y = *y + atoi(h6_sz);
      }
      else {
        char *l_font_t = NULL,
             *l_font_color = NULL,
             fontstr[1024];
        int l_font_sz = 15; /* lol */
        for (i = 0; i < el->css.o_n; ++i)
          if (el->css.o[i].t == COLOR) {
            l_font_color = internal_color_to_str(el->css.o[i].v);
            break;
          }
        for (i = 0; i < el->css.o_n; ++i)
          if (el->css.o[i].t == FONT_FAMILY)
            l_font_t = el->css.o[i].v_str;

        if (l_font_t == NULL) {
          warn("%s: font-family undefined (probably a bug)", __FILE__);
          l_font_t = "monospace";
        }

        if (l_font_color == NULL) {
          warn("%s: font color undefined (probably a bug)", __FILE__);
          l_font_t = "#ff0000";
        }

        snprintf(fontstr, 1024, "%s:pixelsize=%d:style=%s", l_font_t,
            l_font_sz, "Normal");
        x_load_render_destroy(fontstr, draw, l_font_color, *x, *y,
          ((int)strlen(draw) > maxlen) ? maxlen : (int)strlen(draw), dpy, xd);
        free(l_font_color);
      }
        /*XftDrawStringUtf8(xd, color, font_n, *x, *y, (const FcChar8*)draw,*/
            /*((int)strlen(draw) > maxlen) ? maxlen : (int)strlen(draw));*/

      draw += ((int)strlen(draw) > maxlen) ? maxlen : (int)strlen(draw);
      *y = *y + fontsz * 2;
      *x = (use_padding) ? padding : bakx;
    }

    *y = *y - fontsz * 2;
    if (at.h1 || at.h2 || at.h3 || at.h4 || at.h5 || at.h6)
      *x = (use_padding) ? padding : bakx;
    else 
      *x = bakx + (strlen(el->TT_val) * (fontsz / fontratio));

    /*XDrawString(d, w, DefaultGC(d, s), *x, *y, el->TT_val,*/
          /*strlen(el->TT_val));*/
  } 
  if (el->t != TEXT_TYPE) {
    for (i = 0; i < el->child_n; ++i)
      x_recursive_render_text(dpy, xd, color, x, y, &el->child[i], maxw, maxh,
        use_padding);
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
      force_expose = 0;
  signed int scroll = 0;
  Window win;
  XEvent ev;
  XWindowAttributes wa;
  KeySym ks;
  Colormap cmap;
  Visual *visual;
  XftDraw *xd;
  XftColor color,
           bgcolor;
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

  XSelectInput(dpy, win, ExposureMask | KeyPressMask | ButtonPressMask);
  XMapWindow(dpy, win);

  XStoreName(dpy, win, "netskater");

  x_init_fonts(dpy, s, visual, cmap, "#000000", &color);

  if (!XftColorAllocName(dpy, visual, cmap, "#ffffff", &bgcolor))
    err("%s: couldn't allocate xft color", __FILE__);

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
        /* it is a hack, 'cause i have no idea how to properly send an Expose
         * event
         */
        XftDrawRect(xd, &bgcolor, 0, 0, width, height);
        force_expose = 0;
      }

      x_now = padding, y_now = padding + (scroll * scroll_pixels);

      XGetWindowAttributes(dpy, win, &wa);
      width = wa.width;
      height = wa.height;

      time_start = clock();
      x_recursive_render_text(dpy, xd, &color, &x_now, &y_now, page, width,
          height, 1);
      time_end = clock();
      info("%s: x_recursive_render_text() -> took %6.4f",
          __FILE__, (double)(time_end - time_start) / CLOCKS_PER_SEC);

    } else if (ev.type == KeyPress) {
      XLookupString(&ev.xkey, NULL, 0, &ks, NULL);

      switch (ks) {
        case XK_Escape:
        case XK_q:
          goto endloop;
        case XK_k:
          if (scroll < 0) {
            ++scroll;
            force_expose = 1;
          }
          break;
        case XK_j:
          --scroll;
          force_expose = 1;
          break;
        case XK_space:
          scroll -= 10;
          force_expose = 1;
          break;
        case XK_d:
          html_print_tree(page, 0, stdout);
          break;
      }
    } else if (ev.type == ButtonPress) {
      switch (ev.xkey.keycode) {
        case Button4:
          if (scroll < 0) {
            ++scroll;
            force_expose = 1;
          }
          break;
        case Button5:
          --scroll;
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

