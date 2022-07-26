#include "ns.h"

#include <stdlib.h>
#include <stdarg.h>

void err(char *fmt, ...) {
  va_list vl;
  va_start(vl, fmt);
  fprintf(stderr, "error: ");
  vfprintf(stderr, fmt, vl);
  fprintf(stderr, "\n");
  va_end(vl);
  exit(1);
}

void warn(char *fmt, ...) {
  va_list vl;
  va_start(vl, fmt);
  fprintf(stderr, "warning: ");
  vfprintf(stderr, fmt, vl);
  fprintf(stderr, "\n");
  va_end(vl);
}

int main (void) {
  FILE *f = fopen("test.html", "r");

  HTML_elem *tree = create_HTML_tree(f);

  render_page(tree);

  fclose(f);

  free_HTML_elem(tree);
  free(tree);
  return 0;
}

