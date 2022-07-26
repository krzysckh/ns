#include "ns.h"

#include <stdlib.h>
#include <string.h>
#include <wctype.h>
#include <ctype.h>

static void html_print_tree(HTML_elem *el, int depth) {
  int i;

  /*printf("%d\n", depth);*/
  for (i = 0; i < depth; ++i)
    printf("  ");

  printf("type %d, children: %d\n", el->t, el->child_n);
  if (el->t == TEXT_TYPE) {
    for (i = 0; i < 1 + depth; ++i)
      printf("  ");
    printf("text: %s\n", el->TT_val);
  }

  for (i = 0; i < el->child_n; ++i)
    html_print_tree(&el->child[i], depth + 1);
}

void init_HTML_elem(HTML_elem *el, HTML_elem *parent) {
  el->t= -1;
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
    /*printf("%c\n", *text);*/
    ++name_sz;
    ++text;
  }

  lcase = malloc(name_sz + 1);
  lcase[name_sz] = 0;
  cpt_to_lower(text - name_sz, lcase, name_sz);

  if (strcmp(lcase, "html") == 0) ret = HTML;
  else if (strcmp(lcase, "head") == 0) ret = HEAD;
  else if (strcmp(lcase, "body") == 0) ret = BODY;
  else if (strcmp(lcase, "p") == 0) ret = PARAGRAPH;

  if (*lcase == '/') ret = INTERNAL_BACK;

  free(lcase);
  return ret;
}

HTML_elem create_HTML_tree(FILE *fp) {
  HTML_elem ret,
            *cur = &ret;
  init_HTML_elem(&ret, &ret);
  ret.t = ROOT;

  char *text,
       *text_orig_p;
  int text_sz,
      tt_sz;

  fseek(fp, 0, SEEK_END);
  text_sz = ftell(fp);
  fseek(fp, 0, SEEK_SET);

  text = malloc(text_sz + 1);
  fread(text, 1, text_sz, fp);
  text[text_sz] = 0;
  text_orig_p = text;

  while (*text) {
    while (iswspace(*text))
      ++text;

    if (*text == '<') {
      ++cur->child_n;
      cur->child = realloc(cur->child, sizeof(HTML_elem) * cur->child_n);
      init_HTML_elem(&cur->child[cur->child_n-1], cur);
      cur = &cur->child[cur->child_n-1];
      
      ++text;
      cur->t = get_elem_type(text);
      while (*text++ != '>')
        ;
      if (cur->t == INTERNAL_BACK) {
        cur = cur->parent;
        cur->child_n--;
        cur = cur->parent;
      }
      /* ? */
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
        cur->TT_val[0] = 0;
        ++text;
      } else {
        strncpy(cur->TT_val, text-tt_sz, tt_sz-1);
        cur->TT_val[tt_sz-1] = 0;
      }

      cur = cur->parent;
    }
  }

  html_print_tree(&ret, 0);

  free(text_orig_p);
  return ret;
}

