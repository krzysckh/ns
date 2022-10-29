#include "ns.h"

/* TODO: tables */
void console_render_page(HTML_elem *page) {
  int i;
  for (i = 0; i < page->child_n; ++i) {
    if (page->child[i].t == SCRIPT
        || page->child[i].t == STYLE) continue;

    if (page->child[i].t == TEXT_TYPE)
      printf("%s", page->child[i].TT_val);
    else
      console_render_page(&page->child[i]);
  }
  if (
      page->t == PARAGRAPH
      || page->t == LIST_ELEM
      || page->t == H1
      || page->t == H2
      || page->t == H3
      || page->t == H4
      || page->t == H5
      || page->t == H6
      || page->t == BREAK_LINE
      || page->t == IMAGE

      || page->t == TABLE_TR

      || page->t == UNKNOWN
  ) printf("%s", "\n");
  /* wonky printf, because of plan9 and printf as macro */
  return;
}
