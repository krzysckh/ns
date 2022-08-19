#include "ns.h"

#include <stdlib.h>
#include <stdarg.h>

void err(char *fmt, ...) {
  va_list vl;
  va_start(vl, fmt);
#ifdef USE_COLOR
  fprintf(stderr, "\x1b[48;2;255;0;0m");
  fprintf(stderr, "\x1b[38;2;0;0;0m");
#endif
  fprintf(stderr, "error: ");
#ifdef USE_COLOR
  fprintf(stderr, "\033[0m");
#endif
  vfprintf(stderr, fmt, vl);
  fprintf(stderr, "\n");
  va_end(vl);
  exit(1);
}

void warn(char *fmt, ...) {
  va_list vl;
  va_start(vl, fmt);
#ifdef USE_COLOR
  fprintf(stderr, "\x1b[38;2;255;255;255m");
  fprintf(stderr, "\x1b[48;2;255;102;0m");
#endif
  fprintf(stderr, "warning: ");
#ifdef USE_COLOR
  fprintf(stderr, "\033[0m");
#endif
  vfprintf(stderr, fmt, vl);
  fprintf(stderr, "\n");
  va_end(vl);
}

void info(char *fmt, ...) {
  va_list vl;
  va_start(vl, fmt);
#ifdef USE_COLOR
  fprintf(stderr, "\x1b[38;2;255;255;255m");
  fprintf(stderr, "\x1b[48;2;80;80;180m");
#endif
  fprintf(stderr, "info: ");
#ifdef USE_COLOR
  fprintf(stderr, "\033[0m");
#endif
  vfprintf(stderr, fmt, vl);
  fprintf(stderr, "\n");
  va_end(vl);
}

int main (void) {
  FILE *f = download_file("http://9front.org");

  HTML_elem *tree = create_HTML_tree(f);

  render_page(tree);

  fclose(f);

  free_HTML_elem(tree);
  free(tree);
  return 0;
}

