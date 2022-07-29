#include "ns.h"

#include <stdlib.h>
#include <string.h>
#include <wctype.h>
#include <ctype.h>

static void html_print_tree(HTML_elem *el, int depth) {
  int i;

  for (i = 0; i < depth; ++i)
    printf("  ");

  printf("type %d, children: %d, addr: %p, parent: %p\n", el->t, el->child_n,
      el, el->parent);
  if (el->t == TEXT_TYPE) {
    for (i = 0; i < 1 + depth; ++i)
      printf("  ");
    printf("text: %s\n", el->TT_val);
  }

  for (i = 0; i < el->child_n; ++i)
    html_print_tree(&el->child[i], depth + 1);
}

void init_HTML_elem(HTML_elem *el, HTML_elem *parent) {
  el->t = -1;
  el->argc = 0;
  el->child_n = 0;
  el->child = NULL;
  el->parent = parent;
  el->TT_val = NULL;
}

void free_HTML_elem(HTML_elem *el) {
  int i, j;

  if (el->TT_val != NULL)
    free(el->TT_val);

  for (j = 0; j < el->argc; ++j)
    free(el->argv[j]);

  for (i = 0; i < el->child_n; ++i)
    free_HTML_elem(&el->child[i]);
}

static void cpt_to_lower(char *from, char *to, int l) {
  while (l--)
    *to++ = tolower(*from++);
}

static HTML_elem_type get_elem_type(char *text) {
  HTML_elem_type ret = UNKNOWN;
  int name_sz = 0;
  char *lcase;

  while (!iswspace(*text) && *text != '>') {
    ++name_sz;
    ++text;
  }

  lcase = malloc(name_sz + 1);
  lcase[name_sz] = 0;
  cpt_to_lower(text - name_sz, lcase, name_sz);

  /*printf("lcase = %s\n", lcase);*/

  if (strcmp(lcase, "html") == 0) ret = HTML;
  else if (strcmp(lcase, "head") == 0) ret = HEAD;
  else if (strcmp(lcase, "body") == 0) ret = BODY;
  else if (strcmp(lcase, "p") == 0) ret = PARAGRAPH;
  else if (strcmp(lcase, "b") == 0) ret = BOLD;
  else if (strcmp(lcase, "i") == 0) ret = ITALIC;
  else if (strcmp(lcase, "br") == 0) ret = BREAK_LINE;
  else if (strcmp(lcase, "ol") == 0) ret = UNORDERED_LIST;
  else if (strcmp(lcase, "ul") == 0) ret = ORDERED_LIST;
  else if (strcmp(lcase, "li") == 0) ret = LIST_ELEM;

  else if (strcmp(lcase, "h1") == 0) ret = H1;
  else if (strcmp(lcase, "h2") == 0) ret = H2;
  else if (strcmp(lcase, "h3") == 0) ret = H3;
  else if (strcmp(lcase, "h4") == 0) ret = H4;
  else if (strcmp(lcase, "h5") == 0) ret = H5;
  else if (strcmp(lcase, "h6") == 0) ret = H6;

  if (*lcase == '/') ret = INTERNAL_BACK;

  free(lcase);
  return ret;
}

static void fucking_update_tt_parentship(HTML_elem *root) {
  int i;
  for (i = 0; i < root->child_n; ++i) {
    root->child[i].parent = root;
  }
  for (i = 0; i < root->child_n; ++i) {
    fucking_update_tt_parentship(&root->child[i]);
  }
}

HTML_elem *create_HTML_tree(FILE *fp) {
  HTML_elem *ret = malloc(sizeof(HTML_elem)),
            *cur = ret;
  init_HTML_elem(ret, ret);
  ret->t = ROOT;

  char *text,
       *text_orig_p;
  int text_sz,
      tt_sz;
  HTML_elem_type tmp_t;

  fseek(fp, 0, SEEK_END);
  text_sz = ftell(fp);
  fseek(fp, 0, SEEK_SET);

  text = malloc(text_sz + 1);
  fread(text, 1, text_sz, fp);
  text[text_sz] = 0;
  text_orig_p = text;

  while (*(text+1)) {
    while (iswspace(*text))
      ++text;

    if (*text == '<') {
      ++text;
      tmp_t = get_elem_type(text);
      while (*text++ != '>')
        ;
      if (tmp_t == INTERNAL_BACK) {
        cur = cur->parent;
      } else {
        /*printf("text - 2 = %s\n", text-2);*/
        if (*(text-2) == '/') {
          puts("giga kutas");
          cur->child_n++;
          cur->child = realloc(cur->child, sizeof(HTML_elem) * cur->child_n);
          init_HTML_elem(&cur->child[cur->child_n-1], cur);
          cur->child[cur->child_n-1].t = tmp_t;
        } else {
          cur->child_n++;
          cur->child = realloc(cur->child, sizeof(HTML_elem) * cur->child_n);
          init_HTML_elem(&cur->child[cur->child_n-1], cur);

          cur = &cur->child[cur->child_n-1];
          cur->t = tmp_t;
        }
      }
    } else {
      tt_sz = 0;

      cur->child_n++;
      cur->child = realloc(cur->child, sizeof(HTML_elem) * cur->child_n);
      init_HTML_elem(&cur->child[cur->child_n-1], cur);

      cur = &cur->child[cur->child_n-1];
      cur->t = TEXT_TYPE;

      while (*text && *text++ != '<')
        ++tt_sz;
      --text;

      cur->TT_val = malloc(tt_sz + 1);
      if (tt_sz <= 0) {
        cur->TT_val[1] = 0;
        ++text;
      } else {
        strncpy(cur->TT_val, text-tt_sz, tt_sz);
        cur->TT_val[tt_sz] = 0;
      }
      cur = cur->parent;
    }
  }

  fucking_update_tt_parentship(ret);
  html_print_tree(ret, 0);

  free(text_orig_p);
  return ret;
}

