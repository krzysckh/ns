#include "ns.h"

#ifdef USE_9

#else
#include <stdlib.h>
#include <stdarg.h>
#endif

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

int main (int argc, char *argv[]) {
#ifdef USE_9
  int f;
#else
  FILE *f;
#endif

  if (argc > 1)
    f = download_file(argv[1]);
  else
#ifdef USE_9
    f = open("test.html", OREAD);
#else
    f = fopen("test.html", "r");
#endif

  HTML_elem *tree = create_HTML_tree(f);

  render_page(tree);

#ifdef USE_9
  close(f);
#else
  fclose(f);
#endif

  free_HTML_elem(tree);
  free(tree);
  return 0;
}

