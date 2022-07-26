#include <stdio.h>

typedef enum {
  ROOT,
  HTML,
  HEAD,
  BODY,
  PARAGRAPH,
  BOLD,

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

