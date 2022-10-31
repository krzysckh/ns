#define MAX(x, y) x > y ? x : y
#define MIN(x, y) x > y ? y : x

#define _DEFAULT_SOURCE

#define PROGNAME "ns"

#ifdef USE_9
#include <u.h>
#include <libc.h>

#include <ctype.h>
#define iswspace(x) isspace(x)

enum {
  stdin,
  stdout,
  stderr
};

#define printf(fmt, ...) print(fmt, __VA_ARGS__)
#define fprintf(fmt, ...) fprint(fmt, __VA_ARGS__)
#define vfprintf(fmt, ...) vfprint(fmt, __VA_ARGS__)
#define snprintf(x, n, fmt, ...) snprint(x, n, fmt, __VA_ARGS__)

#ifndef PLAN9PORT
typedef short int uint8_t;
typedef unsigned long int uint32_t;
#define NULL nil
#endif

#else
#include <stdio.h>
#include <stdint.h>
#endif

#if defined(USE_9) && defined(USE_X)
#error cannot USE_9 and USE_X
#endif

typedef enum {
  ROOT,
  HTML,
  HEAD,
  BODY,
  PARAGRAPH,
  BOLD,
  ITALIC,
  BREAK_LINE,
  
  A,

  H1,
  H2,
  H3,
  H4,
  H5,
  H6,

  UNORDERED_LIST,
  ORDERED_LIST,
  LIST_ELEM,

  TABLE,
  TABLE_TR,
  TABLE_TD,
  TABLE_TH,

  IMAGE,

  STYLE,
  SCRIPT, 

  META,
  LINK,

  UNKNOWN,

  TEXT_TYPE,

  CSS_ALL_SELECTORS, /* :^) */
  INTERNAL_BACK,
} HTML_elem_type;

typedef enum {
  BACKGROUND_COLOR,
  COLOR,
  FONTSIZE,
  FONT_FAMILY,
  FONT_WEIGHT,
  FONT_STYLE,
  BORDER_WIDTH,
  BORDER_COLOR,
  CSS_UNKNOWN,
  CSS_NEXT_SELECTOR
} CSS_otype;

typedef enum {
  M_INCH,
  M_PIXEL,
  M_PERCENT,
  M_COLOR,
  M_STRING,
  M_NUMBER
} CSS_metric;

typedef struct CSS_opt {
  CSS_otype t;
  uint32_t v;
  char *v_str;
  CSS_metric m;
} CSS_opt;

typedef struct Calculated_CSS {
  CSS_opt *o;
  int o_n;
} Calculated_CSS;

typedef struct HTML_elem {
  HTML_elem_type t;
  int argc;
  char ***argv;

  int child_n;
  struct HTML_elem *child;
  struct HTML_elem *parent;

  char *TT_val;
  /* value if TEXT_TYPE */

  Calculated_CSS css;
} HTML_elem;

#ifdef USE_9
HTML_elem *create_HTML_tree(int fp);
void html_print_tree(HTML_elem *el, int depth, int outf);
int download_file(char *url);
#else
HTML_elem *create_HTML_tree(FILE *fp);
void html_print_tree(HTML_elem *el, int depth, FILE *outf);
FILE *download_file(char *url);
#endif

void free_HTML_elem(HTML_elem *el);
void render_page(HTML_elem *page);
void cpt_to_lower(char *from, char *to, int l);
HTML_elem_type get_elem_type(char *text);
void calculate_css(HTML_elem *el);
char *internal_color_to_str(uint32_t c);

const char *elemt_to_str(HTML_elem_type t);

HTML_elem *get_object_by_click(int x, int y);
void clear_click_map();
void register_click_object(int x1, int y1, int x2, int y2, HTML_elem *el);

#ifdef USE_X
#include <X11/Xlib.h>
#include <X11/Xft/Xft.h>

void x_draw_click_objects(XftDraw *xd, XftColor *color);
#endif

#ifdef USE_9
#include <draw.h>
void p9_draw_click_objects(Image *screen);
#endif

void err(char *fmt, ...);
void warn(char *fmt, ...);
void info(char *fmt, ...);

#ifdef USE_CONSOLE
int console_render_page(HTML_elem *page);
#endif
