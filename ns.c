#include "ns.h"

int main (void) {
  FILE *f = fopen("test.html", "r");

  HTML_elem tree = create_HTML_tree(f);

  fclose(f);

  free_HTML_elem(&tree);
  return 0;
}

