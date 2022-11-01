#include "ns.h"

#ifdef USE_9

#else
#include <stdlib.h>
#include <stdarg.h>
#include <getopt.h>
#endif

int debug,
    console,
    silent,
    termw = 80;

void usage() {
  err("%s: usage: %s [-hdc] [-t width] [link]", __FILE__, PROGNAME);
}

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
#ifdef USE_9
  sysfatal("err");
#else
  exit(1);
#endif
}

void warn(char *fmt, ...) {
  if (silent) return;

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
  if (silent) return;

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
  int opt;
#endif
  debug = 0;
  console = 0;
#ifdef USE_9
  ARGBEGIN {
    case 'h':
      usage();
      break;
    case 'd':
      debug = 1;
      break;
    case 'c':
      console = 1;
      /* fallthru */
    case 's':
      silent = 1;
      break;
    case 't':
      termw = atoi(EARGF(usage()));
      break;
    default:
      usage();
  } ARGEND;

  if (*argv == NULL) {
    f = open("test.html", OREAD);
    if (!f) err("%s: couldnt open test.html", __FILE__);
  } else {
    f = download_file(*argv);
  }
#else
  while ((opt = getopt(argc, argv, "hdcst:")) != -1) {
    switch (opt) {
      case 'h':
        usage();
        break;
      case 'd':
        debug = 1;
        break;
      case 'c':
        console = 1;
        /* fallthru */
      case 's':
        silent = 1;
        break;
      case 't':
        termw = atoi(optarg);
        break;
      default:
        usage();
    }
  }

  if (argv[optind] == NULL) {
    f = fopen("test.html", "r");
    if (!f) err("%s: couldnt open test.html", __FILE__);
  } else {
    f = download_file(argv[optind]);
  }
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

