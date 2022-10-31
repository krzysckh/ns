#include "ns.h"

extern int termw;

/* TODO: tables */

#define console_get_td_width(n) (int)(termw / n) - 2 - (n-1)

static void console_render_table(HTML_elem *t) {
  puts("[table]");
#if 0
  int i,
      j,
      k,
      n,
      td_w,
      drew = 0,
      bak_termw = termw;
  HTML_elem *tr;

  for (i = 0; i < t->child_n; ++i) {
    if (t->child[i].t == TABLE_TR) {
      n = 0;
      tr = &t->child[i];

      for (j = 0; j < tr->child_n; ++j) {
        if (tr->child[j].t == TABLE_TH || tr->child[j].t == TABLE_TD)
          n++;
      }

      if (n <= 0) goto noelems;

      td_w = console_get_td_width(n);

      for (j = 0; j < n; j++) {
        putchar('+');
        for (k = 0; k < td_w; k++)
          putchar('-');
      }
      puts("+\n");
    }
noelems: ;
  }
#endif
}

int console_render_page(HTML_elem *page) {
  int i,
      lines = 0;
  for (i = 0; i < page->child_n; ++i) {
    if (page->child[i].t == TABLE) {
      console_render_table(&page->child[i]);
      continue;
    } else if (page->child[i].t == SCRIPT
        || page->child[i].t == STYLE) {
      continue;
    } else if (page->child[i].t == TEXT_TYPE) {
      printf("%s", page->child[i].TT_val);
    } else
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
  ) {
    printf("%s", "\n");
    lines++;
  }
  /* wonky printf, because of plan9 and printf as macro */
  return lines;
}
