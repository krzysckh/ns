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

static void elem_ptr_cpy(HTML_elem *to, HTML_elem *from) {
  to->t = from->t;
  to->argc = from->argc;
  to->argv = from->argv;
  to->child_n = from->child_n;
  to->child = from->child;
  to->TT_val = from->TT_val;
}

static void elem_append_child(HTML_elem *parent, HTML_elem *child) {
  /* assumes init_HTML_elem() */
  parent->child = realloc(parent->child, sizeof(HTML_elem));
  elem_ptr_cpy(&parent->child[parent->child_n], child);
  parent->child[parent->child_n].parent = parent;

  parent->child_n++;
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

  printf("lcase = %s\n", lcase);

  if (strcmp(lcase, "html") == 0) ret = HTML;
  else if (strcmp(lcase, "head") == 0) ret = HEAD;
  else if (strcmp(lcase, "body") == 0) ret = BODY;
  else if (strcmp(lcase, "p") == 0) ret = PARAGRAPH;

  printf("*lcase = %c\n", *lcase);
  if (*lcase == '/') ret = INTERNAL_BACK;

  free(lcase);
  return ret;
}

static HTML_elem create_child_fromhere(char *text, HTML_elem *parent) {
  HTML_elem ret;
  init_HTML_elem(&ret, parent);

  if (text == NULL)
    return *parent;

  int tt_len = 0;

  while (iswspace(*text))
    ++text;

  if (*text != '<') {
    ret.t = TEXT_TYPE;

    while (*text != '<') {
      if (*text == 0)
        return *parent;
      ++tt_len;
      ++text;
    }

    ret.TT_val = malloc(tt_len + 1);
    strncpy(ret.TT_val, text - tt_len, tt_len);
    ret.TT_val[tt_len] = 0;

    HTML_elem tmp = create_child_fromhere(text, parent);
    elem_append_child(parent, &tmp);
    /* returns ret */
  } else {
    ++text;
    ret.t = get_elem_type(text);
    while (*text++ != '>')
      if (text == NULL)
        return *parent;

    if (ret.t != INTERNAL_BACK) {
      HTML_elem tmp = create_child_fromhere(text, &ret);

      if (tmp.t == TEXT_TYPE) {
        tmp.parent = parent;
        elem_append_child(parent, &tmp);
        return *parent;
      }

      if (tmp.t == INTERNAL_BACK)
        return ret;
      else
        elem_append_child(&ret, &tmp);
    } else {
      /*HTML_elem tmp = create_child_fromhere(text, parent->parent);*/
      /*elem_append_child(parent->parent, &tmp);*/

      HTML_elem tmp = create_child_fromhere(text, parent);
      elem_append_child(parent, &tmp);
      return *parent;
    }
  }

  return ret;
}

HTML_elem create_HTML_tree(FILE *fp) {
  HTML_elem ret;
  init_HTML_elem(&ret, NULL);
  ret.t = ROOT;

  char *text;
  int text_sz;

  fseek(fp, 0, SEEK_END);
  text_sz = ftell(fp);
  fseek(fp, 0, SEEK_SET);

  text = malloc(text_sz + 1);
  fread(text, 1, text_sz, fp);
  ret = create_child_fromhere(text, &ret);

  html_print_tree(&ret, 0);

  free(text);
  return ret;
}

