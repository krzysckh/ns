#include "ns.h"

#ifdef USE_9
/* included in ns.h */
/*#include <draw.h>*/
#include <cursor.h>
#include <event.h>
#else
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include <time.h>
#endif

#ifdef USE_CONSOLE
extern int console;
#endif

/* so - the page rendering is slow if everything is calculated everytime 
 * an event happens. my idea: create a RenderMap that holds text and how it
 * should be rendered, so it has to be calculated once - when the page loads,
 * and then it just uses the RenderMap to render elements
 *
 * renderObj is an object, that needs to be rendered every time the screen is
 * cleared. e.g. a line, or a colored box.
 *
 * renderObjs are always rendered _before_ text, so they can (in the future)
 * be used as backgrounds :^)
 *
 * - replace all calls to libxft || string() via draw.h with calls to
 *   rendermap_add()
 * - when loading new webpage call rendermap_clear()
 */

typedef enum {
  BOX,
  LINE,
  /*CIRCLE,*/ /* ? */
  /* god knows what */
} ro_type;

typedef struct {
  int x1;
  int y1;
  int x2;
  int y2;
  ro_type t;
} renderObj;

typedef struct {
  char **v; /* value */
  int *x;
  int *y;
  Calculated_CSS **css; /* css */

  renderObj *ro;
  int ro_n;

  int rm_sz;
} RenderMap_t;

static RenderMap_t g_render_map = { .v = NULL, .x = NULL, .y = NULL, 
  .css = NULL, .rm_sz = 0, .ro = NULL, .ro_n = 0 };

static void rendermap_reg_robj(int x1, int y1, int x2, int y2, ro_type t) {
  g_render_map.ro = realloc(g_render_map.ro, sizeof(renderObj) *
      (g_render_map.ro_n + 1));

  g_render_map.ro[g_render_map.ro_n].x1 = x1;
  g_render_map.ro[g_render_map.ro_n].x2 = x2;
  g_render_map.ro[g_render_map.ro_n].y1 = y1;
  g_render_map.ro[g_render_map.ro_n].y2 = y2;
  g_render_map.ro[g_render_map.ro_n].t = t;

  g_render_map.ro_n++;
}
/* "easier" */

static void rendermap_add(int x, int y, char *v, Calculated_CSS *css, int l) {
  g_render_map.x = realloc(g_render_map.x, sizeof(int) * 
      (g_render_map.rm_sz + 1));
  g_render_map.y = realloc(g_render_map.y, sizeof(int) * 
      (g_render_map.rm_sz + 1));
  g_render_map.v = realloc(g_render_map.v, sizeof(char *) *
      (g_render_map.rm_sz + 1));
  g_render_map.css = realloc(g_render_map.css, sizeof(Calculated_CSS *) *
      (g_render_map.rm_sz + 1));

  g_render_map.x[g_render_map.rm_sz] = x;
  g_render_map.y[g_render_map.rm_sz] = y;
  g_render_map.v[g_render_map.rm_sz] = malloc(l + 1);
  strncpy(g_render_map.v[g_render_map.rm_sz], v, l);
  g_render_map.v[g_render_map.rm_sz][l] = 0;
  g_render_map.css[g_render_map.rm_sz] = css; /* just copy the pointer */

  g_render_map.rm_sz++;
}

static void rendermap_clear() {
  int i;

  free(g_render_map.ro);
  for (i = 0; i < g_render_map.rm_sz; ++i)
    free(g_render_map.v[i]);

  free(g_render_map.v);
  free(g_render_map.x);
  free(g_render_map.y);
  free(g_render_map.css);

  g_render_map.x = g_render_map.y = NULL;
  g_render_map.v = NULL;
  g_render_map.css = NULL;
  g_render_map.ro = NULL;

  /* so realloc() won't crash */
  g_render_map.rm_sz = 0;
  g_render_map.ro_n = 0;
}

/* rendermap_render() will be declared later for USE_X and USE_9 */

/* :^) */
#ifdef USE_X
static void x_render_page(HTML_elem*);
#endif
#ifdef USE_9
static void plan9_render_page(HTML_elem*);
#endif

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

  uint8_t anchor;
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

  attr->anchor = 0;
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

  if (el->t == A) attr->anchor ++;

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
#ifdef USE_CONSOLE
  if (console) {
    console_render_page(page);
    return;
  }
#endif
#ifdef USE_X
  x_render_page(page);
#endif
#ifdef USE_9
  plan9_render_page(page);
#endif
}

#ifdef USE_X

#define def_font "monospace:pixelsize=15:weight=100:slant=roman"

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
/*#include <X11/Xutil.h>*/
/*#include <X11/Xos.h>*/

#include <X11/Xft/Xft.h>

static int fuck_you_this_is_bak_x = padding;
int scroll_now = 0;
const int scroll_pixels = 30;

static XftFont *default_font;

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
  XftFont *fnt;
  int s = DefaultScreen(dpy);
  XftColor c;
  if (strcmp(fontn, def_font) == 0) {
    fnt = default_font;
  } else {
   fnt = XftFontOpenName(dpy, s, fontn);
  if (!fnt)
    err("%s: couldn't load font %s", __FILE__, fontn);
  }

  if (!XftColorAllocName(dpy, DefaultVisual(dpy, s), 
        DefaultColormap(dpy, s), color, &c))
    err("%s: couldn't allocate xft color", __FILE__);

  XftDrawStringUtf8(xd, &c, fnt, x, y, (const FcChar8*)txt, len);
  XftColorFree(dpy, DefaultVisual(dpy, s), DefaultColormap(dpy, s), &c);
  XftFontClose(dpy, fnt);
}

static int intlen(int x) {
  /* DON'T LAUGH!
   * it's the fastest solution according to
   * https://stackoverflow.com/questions/3068397/
     finding-the-length-of-an-integer-in-c
   */
  if (x >= 1000000000) return 10;
  if (x >= 100000000) return 9;
  if (x >= 10000000) return 8;
  if (x >= 1000000) return 7;
  if (x >= 100000) return 6;
  if (x >= 10000) return 5;
  if (x >= 1000) return 4;
  if (x >= 100) return 3;
  if (x >= 10) return 2;
  return 1;
}

static void rendermap_render(Display *dpy, XftDraw *xd, int scroll) {
  int i,
      j,
      sz = -1,
      f_weight = 400 / 4,
      s = scroll * scroll_pixels;
  char *clr,
       *fnt,
       *fntstr,
       *fnt_stl;

  for (i = 0; i < g_render_map.ro_n; i++) {
    switch (g_render_map.ro[i].t) {
      case LINE: ;
        XftColor c;
        if (!XftColorAllocName(dpy, DefaultVisual(dpy, DefaultScreen(dpy)),
              DefaultColormap(dpy, DefaultScreen(dpy)), "#000000", &c))
          err("%s: couldn't allocate xft color", __FILE__);

        XftDrawRect(xd, &c, g_render_map.ro[i].x1, g_render_map.ro[i].y1 + s,
            g_render_map.ro[i].x2, g_render_map.ro[i].y2);

        break;
      case BOX:
        err("No.");
    }
  }
  warn("%s: render me daddy", __FILE__);
  for (i = 0; i < g_render_map.rm_sz; ++i) {
    clr = NULL, fnt = NULL;
    for (j = 0; j < g_render_map.css[i]->o_n; ++j) {
      switch (g_render_map.css[i]->o[j].t) {
        case COLOR:
          clr = internal_color_to_str(g_render_map.css[i]->o[j].v);
          break;
        case FONT_FAMILY:
          fnt = g_render_map.css[i]->o[j].v_str;
          break;
        case FONTSIZE:
          switch (g_render_map.css[i]->o[j].m) {
            case M_INCH:
              warn("%s: inch not implemented. setting px (at %p)",
                  __FILE__, g_render_map.css[i]);
              /* fallthru */
            case M_PIXEL:
              sz = g_render_map.css[i]->o[j].v;
              break;
            case M_PERCENT:
              warn("%s: %% not implemented. defalulting to 15px (at %p)",
                  __FILE__, g_render_map.css[i]);
              sz = 15;
          }
          break;
        case FONT_WEIGHT:
          f_weight = g_render_map.css[i]->o[j].v / 4;
          break;
        case FONT_STYLE:
          fnt_stl = g_render_map.css[i]->o[j].v_str;
          /* don't free me. i will cry */
          break;
      }
    }

    if (sz < 0) {
      warn("%s: sz < 0 at %p - defaulting to 15px", __FILE__,
          g_render_map.css[i]);
    }

    fntstr = malloc(strlen(fnt) + strlen(fnt_stl) + intlen(sz) +
        intlen(f_weight) + 27);
    /* 20 -> strlen(":pixelsize=") + strlen(":weight=") + strlen(":slant=") + 1
     * where 1 is null byte
     */

    snprintf(fntstr, strlen(fnt) + strlen(fnt_stl)  + 27 + intlen(sz) +
        intlen(f_weight), "%s:pixelsize=%d:weight=%d:slant=%s", fnt, sz,
        f_weight, fnt_stl);
    /*warn("fontstr = %s", fntstr);*/

    if (g_render_map.y[i] + sz/2 + (scroll * scroll_pixels) < 0)
      goto do_not_render_me_please;

    x_load_render_destroy(fntstr, g_render_map.v[i], clr,
        g_render_map.x[i], g_render_map.y[i] + sz/2 + (scroll * scroll_pixels),
        strlen(g_render_map.v[i]), dpy, xd);

do_not_render_me_please:
    free(clr);
    free(fntstr);
  }
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

#if 0
  XftDrawRect(xd, color, *x, *y, maxw - *x - padding, table_bwidth);
  for (i = 0; i < c_count; ++i)
    XftDrawRect(xd, color, *x + (i * t_width) + table_bwidth, *y, table_bwidth,
        t_height);
  XftDrawRect(xd, color, *x + (c_count * t_width), *y, table_bwidth, t_height);
#endif

  rendermap_reg_robj(*x, *y, maxw - *x - padding, table_bwidth, LINE);
  for (i = 0; i < c_count; ++i)
    rendermap_reg_robj(*x + (i * t_width) + table_bwidth, *y, table_bwidth,
        t_height, LINE);
  rendermap_reg_robj(*x + (c_count * t_width), *y, table_bwidth, t_height,
      LINE);

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
  /*XftDrawRect(xd, color, *x, *y, maxw - *x - padding, table_bwidth);*/
  rendermap_reg_robj(*x, *y, maxw - *x - padding, table_bwidth, LINE);
  
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
#if 0
  x_load_render_destroy("Comic Sans MS:pixelsize=20", "[IMAGE]",
    "#ff00ff", *x, *y, 7, dpy, xd);

  *x = fuck_you_this_is_bak_x;
#endif
  *y += 40;
}

static void x_recursive_render_text(Display *dpy, XftDraw *xd, XftColor *color,
    int *x, int *y, HTML_elem *el, int maxw, int maxh, int use_padding) {
  int i,
      maxlen, 
      bakx = *x,
      x1,
      y1,
      fsz;
  char *draw;
  Text_attr at;

  calculate_css(el);

  for (i = 0; i < el->css.o_n; ++i)
    if (el->css.o[i].t == FONTSIZE)
      fsz = el->css.o[i].v;

  if (el->t == TABLE) {
    x_render_table(dpy, xd, color, x, y, el, maxw, maxh);
    return;
  /*} else if (el->t == LIST_ELEM) {
    if (el->parent->t != ORDERED_LIST && el->parent->t != UNORDERED_LIST) {
      warn("%s: parent of %p (%p = %s) is neither ORDERED_LIST nor "
          "UNORDERED_LIST -- ignoring this branch", __FILE__, el,
          el->parent, elemt_to_str(el->parent->t));
      return ;
    }*/
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
      || el->t == LIST_ELEM
      ) {
    if (use_padding)
      *y = *y + (fsz * 2);
    *x = (use_padding) ? padding : fuck_you_this_is_bak_x;
  } else if (el->t == STYLE || el->t == SCRIPT || el->t == LINK) {
    return;
  } 

  if (el->t == TEXT_TYPE) {
    init_text_attr(&at);
    get_text_attr(el, &at, 0);

    if (at.table > 1) {
      warn("%s: tables in tables not supported!", __FILE__);
      /*return;*/
    }

    draw = el->TT_val;
    while (*draw) {
      if (*y + 10 < 0) {
#ifdef DUMB_WARNINGS
        warn("%s: couldn't draw text \"%s\" (%p) at [%d, %d] (y < 0)",
            __FILE__, draw, draw, *x, *y);
#endif
        return;
      }

      if (at.anchor) {
        x1 = *x;
        y1 = *y - fsz;
      }

      maxlen = x_get_maxlen(maxw, x, y, use_padding, bakx, fsz, fontratio, 1);

      rendermap_add(*x, *y, draw, &el->css, ((int)strlen(draw) > maxlen) ?
          maxlen : (int)strlen(draw));

      if (at.anchor) {
        int what_the_fuck = ((int)strlen(draw) > maxlen ? maxlen -
            (fontratio * *x) : strlen(draw)) / 2;
        register_click_object(x1, y1 + fsz/2 + (scroll_now * scroll_pixels),
            x1 + fsz * what_the_fuck,
            y1 + fsz + fsz/2 + (scroll_now * scroll_pixels), el->parent);
      }

      draw += ((int)strlen(draw) > maxlen) ? maxlen : (int)strlen(draw);
      *y = *y + fontsz * 2;
      *x = (use_padding) ? padding : bakx;
    }

    *y = *y - fsz * 2;
    if (at.h1 || at.h2 || at.h3 || at.h4 || at.h5 || at.h6)
      *x = (use_padding) ? padding : bakx;
    else 
      *x = bakx + (strlen(el->TT_val) * (fontsz / fontratio));

    /*XDrawString(d, w, DefaultGC(d, s), *x, *y, el->TT_val,*/
          /*strlen(el->TT_val));*/
    /* this is history */
  } 
  if (el->t != TEXT_TYPE) {
    for (i = 0; i < el->child_n; ++i)
      x_recursive_render_text(dpy, xd, color, x, y, &el->child[i], maxw, maxh,
        use_padding);
  }

  /* it will not handle links over multiple lines sometimes :^) */
  /* and TODO: it will not work if the link is an header, or anything, that
   * returns to the next line after beinng written, so h{1,2,3,4,5,6}
   */
#if 0
  if (el->t == A) {
    if (*x < x1) {
      /* there was a breakline */
      register_click_object(x1, y1, maxw, y1 - fontsz, el);
      x1 = (use_padding) ? padding : fuck_you_this_is_bak_x;
      /* y1 doesnt change */
      x2 = *x;
      y2 = *y;
      register_click_object(x1, y1, x2, y2, el);
    } else {
      x2 = *x;
      y2 = *y;
      register_click_object(x1, y1, x2, y2, el);
    }
  }
#endif
}

static void x_init_default_font(Display *dpy) {
  default_font = XftFontOpenName(dpy, DefaultScreen(dpy), def_font);

  if (!default_font)
    err("%s: couldn't load font %s", __FILE__, def_font);
}

static void x_render_page(HTML_elem *page) {
  int s,
      width = 640,
      height = 480,
      x_now = padding,
      y_now = padding,
      force_expose = 0,
      rendermap_created = 0,
      i;
  FILE *tmpf;
  signed show_links = 0;
  Window win;
  XEvent ev;
  XWindowAttributes wa;
  KeySym ks;
  Colormap cmap;
  Visual *visual;
  XftDraw *xd;
  XftColor color,
           bgcolor;
  HTML_elem *tmp_el;
  clock_t fn_start = clock(),
          fn_end;

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

  if (!XftColorAllocName(dpy, visual, cmap, "#ffffff", &bgcolor))
    err("%s: couldn't allocate xft color", __FILE__);

  xd = XftDrawCreate(
    dpy,
    win,
    visual,
    cmap
  );

  x_init_default_font(dpy);

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

      x_now = padding, y_now = padding + (scroll_now * scroll_pixels);

      XGetWindowAttributes(dpy, win, &wa);
      width = wa.width;
      height = wa.height;

      fn_start = clock();
      if (rendermap_created) {
        rendermap_render(dpy, xd, scroll_now);
      } else {
        clear_click_map();
        /* rn rendermap actually slows everything down, but once it's finished
         * it _should_ give some speed ups
         */
        rendermap_clear();
        x_recursive_render_text(dpy, xd, &color, &x_now, &y_now, page, width,
            height, 1);
        rendermap_render(dpy, xd, scroll_now);
        rendermap_created = 1;
      }
      fn_end = clock();
      info("%s: rendering took %6.4f",
          __FILE__, (double)(fn_end - fn_start) / CLOCKS_PER_SEC);

      if (show_links)
        x_draw_click_objects(xd, &color);

    } else if (ev.type == KeyPress) {
      XLookupString(&ev.xkey, NULL, 0, &ks, NULL);

      switch (ks) {
        case XK_Escape:
        case XK_q:
          goto endloop;
        case XK_k:
          if (scroll_now < 0) {
            ++scroll_now;
            force_expose = 1;
          }
          break;
        case XK_j:
          --scroll_now;
          force_expose = 1;
          break;
        case XK_space:
          scroll_now -= 10;
          force_expose = 1;
          break;
        case XK_d:
          html_print_tree(page, 0, stdout);
          break;
        case XK_l:
          show_links = !show_links;
          force_expose = 1;
          break;
      }
    } else if (ev.type == ButtonPress) {
      switch (ev.xkey.keycode) {
        case Button1:
          tmp_el = get_object_by_click(ev.xkey.x, ev.xkey.y);
          if (tmp_el != NULL) {
            /* TODO: move this to some other place e.g. a function */

            if (tmp_el->t == A) {
              for (i = 0; i < tmp_el->argc; ++i) {
                if (strcmp(tmp_el->argv[i][0], "href") == 0) {
                  tmpf = download_file(tmp_el->argv[i][1]);
                  free_HTML_elem(page);
                  page = create_HTML_tree(tmpf);
                  fclose(tmpf);
                  force_expose = 1;
                  rendermap_created = 0;
                  scroll_now = 0;
                  break;
                }
              }
            }
          }

          break;
        case Button4:
          if (scroll_now < 0) {
            ++scroll_now;
            force_expose = 1;
          }
          break;
        case Button5:
          --scroll_now;
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
#ifdef USE_9

static void p9_recursive_render_text(Image *screen, HTML_elem *el, int *x, 
    int *y, int maxw, int USEZERO, int bakx);
static Image *bg;
static HTML_elem *root;
static int showlinks;
#ifdef PLAN9PORT
#define fontsz atoi(FONTSIZE9)
#else
#define fontsz 8
/* ? */
#endif
#define padding 8

#ifdef PLAN9PORT
#define ZERO 0
#else
#define ZERO screen->r.min.x
#endif

static uint32_t p9_internal_color_to_rgba(uint32_t c) {
#if defined(__LITTLE_ENDIAN__) || __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
  return c << 8 | (c >> 24) | 0xff;
#else
#warning untested :^)
#warning it will not work probably
  return c << 8 | (c >> 24) | 0xff;
#endif
}

/* it will not be perfect
 * it's going to approximate the maximum length
 * and i know it's going to be wrong
 * :^)
 */
static int p9_get_maxlen(Image *screen, int *x, int *y, int maxw, int USEZERO,
    int bakx) {
  int ret;

  if (maxw - *x <= 0) {
    *y += fontsz + padding;
    *x = USEZERO ? ZERO : bakx;
  }
  ret = (maxw - *x) / (fontsz /*/ 2*/);
  if (ret < 1)
    return 1;
  else
    return ret;

  /* unreachable - for the plan9 compilers :^) */
  return ret;
}

static int p9_approx_td_height(HTML_elem *el, int x, int maxw, int depth) {
  int ret = 0,
      i;

  for (i = 0; i < el->child_n; ++i) {
    ret += p9_approx_td_height(&el->child[i], x, maxw, depth + 1);
  }
  if (el->t == TEXT_TYPE)
    ret += strlen(el->TT_val);

  if (depth == 0) {
    ret = fontsz * 2 * (1 + (ret / ((maxw - x) / fontsz / 2)));
  }

  return ret;
}

static void p9_render_table(Image *screen, HTML_elem *tbl_r, int *x, int *y,
    int maxw, int maxh) {
  int i,
      j,
      td_n,
      td_w,
      td_max_h = 0,
      tmp_max_h = 0,
      tmpx = *x,
      tmpy = *y;
  HTML_elem *cur_tr;
  Image *tmp_img = allocimage(display, Rect(0, 0, 1, 1), RGB24, 1,
      p9_internal_color_to_rgba(0x000000));

  for (i = 0; i < tbl_r->child_n; ++i) {
    if (tbl_r->child[i].t != TABLE_TR) {
      warn("%s: child of table %p is not type TABLE_TR (%s) - skipping", 
          __FILE__, tbl_r, elemt_to_str(tbl_r->child[i].t));
    } else {
      line(screen, Pt(*x + padding, *y), Pt(maxw - padding, *y),
          0, 0, 0, tmp_img, ZP);

      cur_tr = &tbl_r->child[i];
      td_w = td_n = td_max_h = tmp_max_h = 0;

      for (j = 0; j < cur_tr->child_n; ++j) {
        if (cur_tr->child[i].t != TABLE_TH && cur_tr->child[i].t != TABLE_TD) {
          warn("%s: child of TABLE_TR %p is not type TABLE_TH or TABLE_TD (%s)",
              __FILE__, cur_tr, elemt_to_str(cur_tr->child[i].t));
        } else {
          ++td_n;
        }
      }

      if (td_n != cur_tr->child_n) {
        warn("%s: bad html on elem %p", __FILE__, cur_tr);
        return;
        /* problem solved. i hate myself :^) */
      }

      for (j = 0; j < td_n; ++j) {
        tmp_max_h = p9_approx_td_height(&cur_tr->child[i], *x, maxw, 0);
        tmp_max_h += 2 * padding;
        td_max_h = (tmp_max_h > td_max_h) ? tmp_max_h : td_max_h;
      }

      td_w = (maxw - *x) / td_n;

      for (j = 0; j < td_n; ++j) {
        tmpx = *x + (2 * padding) + (j * td_w);
        tmpy = *y + padding;
        line(screen, Pt(*x + (j * td_w) + padding, *y),
            Pt(*x + (j * td_w) + padding, *y + td_max_h), 0, 0, 0, tmp_img,
            ZP);
        p9_recursive_render_text(screen, &cur_tr->child[j], &tmpx, &tmpy, 
            tmpx + td_w - (2*padding), 0, tmpx);
      }
      line(screen, Pt(maxw - padding, *y),
          Pt(maxw - padding, *y + td_max_h), 0, 0, 0, tmp_img,
          ZP);

      *y += td_max_h;
    }
  }
  line(screen, Pt(*x + padding, *y), Pt(maxw - padding, *y),
      0, 0, 0, tmp_img, ZP);
}

static void p9_recursive_render_text(Image *screen, HTML_elem *el, int *x, 
    int *y, int maxw, int USEZERO, int bakx) {
  int i,
      maxlen,
      a_x1,
      a_y1;
  Image *tmp_img;
  Point pt = { *x, *y };
  char *draw_me;
  Text_attr ta;

  calculate_css(el);

  if (el->t == PARAGRAPH) {
    *y += fontsz + (padding * 2);
    *x = USEZERO ? ZERO : bakx;
  } else if (el->t == TABLE) {
    *y += fontsz + (padding * 2);
    *x = USEZERO ? ZERO : bakx;
    p9_render_table(screen, el, x, y, screen->r.max.x, screen->r.max.y);
    return;
  } else if (el->t == A) {
    a_x1 = *x;
    a_y1 = *y - fontsz;
  } else if (el->t == STYLE || el->t == SCRIPT) {
    /* just don't draw them lol */
    return;
  } if (el->t == TEXT_TYPE) {
    init_text_attr(&ta);
    get_text_attr(el, &ta, 0);

    for (i = 0; i < el->css.o_n; ++i)
      if (el->css.o[i].t == COLOR)
        break;
    /* will not check for problems. too alpha to do so B) */
    draw_me = el->TT_val;

    while (*draw_me) {
      maxlen = p9_get_maxlen(screen, x, y, maxw, USEZERO, bakx);
      if (ta.anchor) {
        a_x1 = *x;
        a_y1 = *y;
      }

      /*warn("color =  0x%08x", p9_internal_color_to_rgba(el->css.o[i].v));*/
      tmp_img = allocimage(display, Rect(0, 0, 1, 1), RGB24, 1,
          p9_internal_color_to_rgba(el->css.o[i].v));

      pt = stringn(screen, Pt(*x, *y), tmp_img, ZP, font, draw_me,
          (strlen(draw_me) > maxlen) ? maxlen : strlen(draw_me));
      *x = pt.x;
      *y = pt.y;
      if (ta.anchor)
        register_click_object(a_x1, a_y1, *x, *y + (2 * fontsz), el->parent);

      draw_me += (strlen(draw_me) > maxlen) ? maxlen : strlen(draw_me);
      if (*draw_me) {
        *x = USEZERO ? ZERO : bakx;
        *y += fontsz + padding;
      }
    }
  } else {
    for (i = 0; i < el->child_n; ++i)
      p9_recursive_render_text(screen, &el->child[i], x, y, maxw, USEZERO,
          bakx);
  }
}

static void redraw(Image *screen) {
#ifdef PLAN9PORT
  int x_r = 0,
      y_r = 0;
#else
  int x_r = screen->r.min.x,
      y_r = screen->r.min.y;
#endif

  draw(screen, screen->r, bg, nil, ZP);
  clear_click_map();
#ifdef PLAN9PORT
  p9_recursive_render_text(screen, root, &x_r, &y_r, Dx(screen->r), 1, 0);
#else
  p9_recursive_render_text(screen, root, &x_r, &y_r, Dx(screen->r) + x_r, 1, 0);
#endif
  if (showlinks)
    p9_draw_click_objects(screen);

  flushimage(display, Refnone);
}

void eresized(int new) {
  if (new && getwindow(display, Refnone) < 0)
    warn("%s: can't reattach window", __FILE__);
  redraw(screen);
}

static void plan9_render_page(HTML_elem* page) {
  int key,
      i;
  uint32_t color = 0xffeeffff;
  Event e;
  Mouse m;
  Menu menu;
  char *menus[] = {"exit", "toggle link view", 0};
  HTML_elem *tmp_el;

  root = page;

#ifdef PLAN9PORT
  if (initdraw(nil, FONTDIR9"/"FONTTYPE9"/"FONTNAME9"."FONTSIZE9".font", argv0)
      < 0)
    sysfatal("%s: %r", argv0);
#else
  /* we aint doin any of that shit here */
  if (initdraw(nil, nil, argv0) < 0)
    sysfatal("%s: %r", argv0);
#endif

  bg = allocimage(display, Rect(0, 0, 1, 1), RGB24, 1, color);
  if (bg == nil)
    err("failed to alloc images");

  redraw(screen);
  einit(Emouse);
  menu.item = menus;
  menu.lasthit = 1;
  while (1) {
    key = event(&e);
    switch (key) {
      case Emouse:
        m = e.mouse;
        if (m.buttons & 4) {
          switch (emenuhit(3, &m, &menu)) {
            case 0:
              exits(0);
              break;
            case 1:
              showlinks = !showlinks;
              redraw(screen);
              break;
          }
        } else if (m.buttons & 1) {
          if ((tmp_el = get_object_by_click(m.xy.x, m.xy.y)) != NULL) {
            for (i = 0; i < tmp_el->argc; ++i) {
              if (strcmp(tmp_el->argv[i][0], "href") == 0) {
                info("%s: you fool! i didn't implement webfs support yet!",
                    __FILE__);
              }
            }
          }
        }
        break;
    }
  }

  if (getwindow(display, Refnone) < 0)
    sysfatal("%s: %r", argv0);

  exits(nil);
}

#endif
