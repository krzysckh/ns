#include "ns.h"

#ifdef USE_9
#warning there is probably a better way to do that
#else
#include <stdlib.h>
#endif

/* this is used mostly for ui.c (in the future), and for clicking links */

/* nw        ne
 * +----------+
 * |   click  |
 * |      me  |
 * +----------+
 * sw        se
 */
struct Click_object {
  int x1;
  int y1;
  int x2;
  int y2;
  HTML_elem *el;
};

struct Click_map {
  struct Click_object *c;
  int c_n;
};

static struct Click_map cm = { .c_n = 0, .c = NULL };

void clear_click_map() {
  cm.c_n = 0;

  if (cm.c != NULL)
    free(cm.c);
  cm.c = NULL;
}

void register_click_object(int x1, int y1, int x2, int y2, HTML_elem *el) {
  /*warn("registered %p [%d, %d] -> [%d, %d]", el, x1, y1, x2, y2);*/
  cm.c = realloc(cm.c, (cm.c_n + 1) * sizeof(struct Click_object));
  cm.c[cm.c_n].x1 = x1;
  cm.c[cm.c_n].y1 = y1;
  cm.c[cm.c_n].x2 = x2;
  cm.c[cm.c_n].y2 = y2;
  cm.c[cm.c_n].el = el;
  cm.c_n++;
}

static int isinside(int x, int y, struct Click_object c) {
  if (x > c.x1 && x < c.x2 && y > c.y1 && y < c.y2)
    return 1;
  return 0;
}

HTML_elem *get_object_by_click(int x, int y) {
  int i;

  for (i = 0; i < cm.c_n; ++i)
    if (isinside(x, y, cm.c[i]))
      return cm.c[i].el;

  return NULL;
}

#ifdef USE_X
#include <X11/Xlib.h>
#include <X11/Xft/Xft.h>
void x_draw_click_objects(XftDraw *xd, XftColor *color) {
  int i;
  for (i = 0; i < cm.c_n; ++i) {
    XftDrawRect(xd, color, cm.c[i].x1, cm.c[i].y1, cm.c[i].x2 - cm.c[i].x1,
        cm.c[i].y2 - cm.c[i].y1);
  }
}
#endif
#ifdef USE_9
#include <draw.h>
void p9_draw_click_objects(Image *screen) {
  int i;
  Image *c = allocimage(display, Rect(0,0,1,1), RGB24, 1, 0xff00ff55);
  for (i = 0; i < cm.c_n; ++i) {
    border(screen, Rect(cm.c[i].x1, cm.c[i].y1, cm.c[i].x2, cm.c[i].y2), -1,
        c, ZP);
  }
}
#endif
