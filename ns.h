#include <stdio.h>

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
  CSS_UNKNOWN,
  CSS_NEXT_SELECTOR
} CSS_otype;

typedef enum {
  M_INCH,
  M_PIXEL,
  M_PERCENT,
  M_COLOR,
  M_STRING
} CSS_metric;

typedef struct CSS_opt {
  CSS_otype t;
  int *v;
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

void free_HTML_elem(HTML_elem *el);
HTML_elem *create_HTML_tree(FILE *fp);
void render_page(HTML_elem *page);
void html_print_tree(HTML_elem *el, int depth, FILE *outf);
void cpt_to_lower(char *from, char *to, int l);
HTML_elem_type get_elem_type(char *text);

void calculate_css(HTML_elem *el);

const char *elemt_to_str(HTML_elem_type t);

FILE *download_file(char *url);

void err(char *fmt, ...);
void warn(char *fmt, ...);
void info(char *fmt, ...);
