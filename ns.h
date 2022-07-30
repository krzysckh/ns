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

  UNKNOWN,

  TEXT_TYPE,

  INTERNAL_BACK,
} HTML_elem_type;

typedef struct HTML_elem {
  HTML_elem_type t;
  int argc;
  char **argv;

  int child_n;
  struct HTML_elem *child;
  struct HTML_elem *parent;

  char *TT_val;
  /* value if TEXT_TYPE */
} HTML_elem;

void free_HTML_elem(HTML_elem *el);
HTML_elem *create_HTML_tree(FILE *fp);
void render_page(HTML_elem *page);

void err(char *fmt, ...);
void warn(char *fmt, ...);
void info(char *fmt, ...);

