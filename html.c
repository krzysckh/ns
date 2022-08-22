#include "ns.h"

#ifdef USE_9
#ifndef PLAN9PORT
static char __FILE__[] = "html.c";
#endif
#else
#include <stdlib.h>
#include <string.h>
#include <wctype.h>
#include <ctype.h>
#include <time.h>
#endif

const char *elemt_to_str(HTML_elem_type t) {
  switch (t) {
    case ROOT: return "root";
    case HTML: return "html";
    case HEAD: return "head";
    case BODY: return "body";
    case PARAGRAPH: return "paragraph";
    case BOLD: return "bold";
    case ITALIC: return "italic";
    case BREAK_LINE: return "break_line";
    case A: return "a";
    case H1: return "h1";
    case H2: return "h2";
    case H3: return "h3";
    case H4: return "h4";
    case H5: return "h5";
    case H6: return "h6";
    case UNORDERED_LIST: return "unordered_list";
    case ORDERED_LIST: return "ordered_list";
    case LIST_ELEM: return "list_elem";
    case TABLE: return "table";
    case TABLE_TR: return "table_tr";
    case TABLE_TD: return "table_td";
    case TABLE_TH: return "table_th";
    case IMAGE: return "image";
    case STYLE: return "style";
    case SCRIPT: return "script";
    case META: return "meta";
    case UNKNOWN: return "unknown";
    case TEXT_TYPE: return "text_type";
    case CSS_ALL_SELECTORS: return "(css) *";
    case INTERNAL_BACK: return NULL;
  }

  err("%s: elemt_to_str(): unhandled HTML_elem_type (%d)", __FILE__, t);
  /* unreachable */
  return NULL;
}

#ifdef USE_9
void html_print_tree(HTML_elem *el, int depth, int outf) {
#else
void html_print_tree(HTML_elem *el, int depth, FILE *outf) {
#endif
  int i,
      j;

  if (depth == 0 && (outf == stdout || outf == stderr))
    info("%s: html tree dump:", __FILE__);

  for (i = 0; i < depth; ++i)
    fprintf(outf, "  ");

  fprintf(outf, "type %s, children: %d, addr: %p, parent: %p\n",
      elemt_to_str(el->t), el->child_n, el, el->parent);
  for (i = 0; i < el->argc; ++i) {
    for (j = 0; j < depth; ++j)
      fprintf(outf, "  ");
    fprintf(outf, "arg (%d) %s = %s\n", i, el->argv[i][0], el->argv[i][1]);
  }
  if (el->t == TEXT_TYPE) {
    for (i = 0; i < 1 + depth; ++i)
      fprintf(outf, "  ");
    fprintf(outf, "text: %s\n", el->TT_val);
  }

  for (i = 0; i < el->child_n; ++i)
    html_print_tree(&el->child[i], depth + 1, outf);

  if (depth == 0 && (outf == stdout || outf == stderr))
    info("%s: end html tree dump", __FILE__);
}

void init_HTML_elem(HTML_elem *el, HTML_elem *parent) {
  el->t = -1;
  el->argc = 0;
  el->argv = NULL;
  el->child_n = 0;
  el->child = NULL;
  el->parent = parent;
  el->TT_val = NULL;
  el->css.o = NULL;
  /*el->css.o->value = NULL;*/
}

void free_HTML_elem(HTML_elem *el) {
  int i, j;

  if (el->TT_val != NULL)
    free(el->TT_val);

  for (j = 0; j < el->argc; ++j) {
    free(el->argv[j][0]);
    free(el->argv[j][1]);
  }

  for (i = 0; i < el->child_n; ++i)
    free_HTML_elem(&el->child[i]);
}

void cpt_to_lower(char *from, char *to, int l) {
  while (l--)
    *to++ = tolower(*from++);
}

HTML_elem_type get_elem_type(char *text) {
  HTML_elem_type ret = UNKNOWN;
  int name_sz = 0;
  char *lcase;

  /* the last one for css.c */
  while (!iswspace(*text) && *text != '>' && *text != '{') {
    if (!*text) {
      warn("%s: html/css ends prematurely (near %s)", __FILE__,
          text-5);
      break;
    }
    ++name_sz;
    ++text;
  }

  lcase = malloc(name_sz + 1);
  lcase[name_sz] = 0;
  cpt_to_lower(text - name_sz, lcase, name_sz);

  if (strcmp(lcase, "html") == 0) ret = HTML;
  else if (strcmp(lcase, "head") == 0) ret = HEAD;
  else if (strcmp(lcase, "body") == 0) ret = BODY;
  else if (strcmp(lcase, "style") == 0) ret = STYLE;
  else if (strcmp(lcase, "p") == 0) ret = PARAGRAPH;
  else if (strcmp(lcase, "b") == 0) ret = BOLD;
  else if (strcmp(lcase, "strong") == 0) ret = BOLD;
  else if (strcmp(lcase, "i") == 0) ret = ITALIC;
  else if (strcmp(lcase, "br") == 0) ret = BREAK_LINE;
  else if (strcmp(lcase, "ol") == 0) ret = UNORDERED_LIST;
  else if (strcmp(lcase, "ul") == 0) ret = ORDERED_LIST;
  else if (strcmp(lcase, "li") == 0) ret = LIST_ELEM;
  else if (strcmp(lcase, "a") == 0) ret = A;

  else if (strcmp(lcase, "h1") == 0) ret = H1;
  else if (strcmp(lcase, "h2") == 0) ret = H2;
  else if (strcmp(lcase, "h3") == 0) ret = H3;
  else if (strcmp(lcase, "h4") == 0) ret = H4;
  else if (strcmp(lcase, "h5") == 0) ret = H5;
  else if (strcmp(lcase, "h6") == 0) ret = H6;

  else if (strcmp(lcase, "table") == 0) ret = TABLE;
  else if (strcmp(lcase, "tr") == 0) ret    = TABLE_TR;
  else if (strcmp(lcase, "td") == 0) ret    = TABLE_TD;

  else if (strcmp(lcase, "img") == 0) ret = IMAGE;

  else if (strcmp(lcase, "style") == 0) ret = STYLE;
  else if (strcmp(lcase, "script") == 0) ret = SCRIPT;

  else if (strcmp(lcase, "meta") == 0) ret = META;

  else if (strcmp(lcase, "*") == 0) ret = CSS_ALL_SELECTORS;
  /* it does not check what the calling function is
   * so if someone was to specify their html like this:
   * <body> <*> ooga booga </lmao> </body>
   * this function _would_ return CSS_ALL_SELECTORS
   * and world _would_ collapse
   * :^)
   *
   * and that is all not because i "want to keep the code clean"
   * or "reuse already written code". i'm just lazy.
   *
   * it is 3AM in the morning, this is a cry for help.
   */

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

void get_elem_args(HTML_elem *el, char *t) {
  int len,
      usequot;

  while (!iswspace(*t++)) {
    if (*t == '>') return;
  }

  while (1) {
  while (iswspace(*t)) ++t;

  len = 0;
  usequot = 0;
  while (*t != '=') {
    if (*t == '/') return;
    if (*t == '>') return;

    if (*t == '>' || iswspace(*t)) {
      /*if (*t == '>' && !len) return;*/
      el->argv = realloc(el->argv, (el->argc+1) * sizeof(char**));
      el->argv[el->argc] = malloc(2 * sizeof(char*));
      el->argv[el->argc][0] = malloc(len + 1);
      strncpy(el->argv[el->argc][0], t-len, len);
      el->argv[el->argc][0][len] = 0;

      el->argv[el->argc][1] = malloc(5);
      strcpy(el->argv[el->argc][1], "true");
      el->argv[el->argc][1][4] = 0;

      el->argc++;

      if (*t == '>') return;

      t++;
      len = 0;
    }

    t++;
    ++len;
  }

  el->argv = realloc(el->argv, (el->argc+1) * sizeof(char**));
  el->argv[el->argc] = malloc(2 * sizeof(char*));
  el->argv[el->argc][0] = malloc(len + 1);
  strncpy(el->argv[el->argc][0], t-len, len);
  el->argv[el->argc][0][len] = 0;

  if (*++t == '"') {
    usequot = 1;
    ++t;
  } else {
    warn("%s: wouldn't say your html is wrong, but it isn't pretty.", __FILE__);
    warn("    at elem %p (%s)", el, elemt_to_str(el->t));
  }

  len = 0;
  while (1) {
    if (!*t)
      err("%s: html err: found EOF while string from el %p (%s) is still open",
          __FILE__, el, elemt_to_str(el->t));
    if (usequot) {
      if (*t == '"' && *(t-1) != '\\') {
        goto s_argv;
      }
    } else if (iswspace(*t) || *t == '>') {
      goto s_argv;
    }

    len++;
    t++;
    continue;
s_argv:
    el->argv[el->argc][1] = malloc(len+1);
    strncpy(el->argv[el->argc][1], t-len, len);
    el->argv[el->argc][1][len] = 0;
    el->argc++;
    break;
  }
  t++;
  }
}

#ifdef USE_9
HTML_elem *create_HTML_tree(int fp) {
#else
HTML_elem *create_HTML_tree(FILE *fp) {
#endif
#ifdef USE_9
  vlong fn_start = nsec(),
        fn_end;
#else
  clock_t fn_start = clock(),
          fn_end;
#endif

  HTML_elem *ret = malloc(sizeof(HTML_elem)),
            *cur = ret;
  init_HTML_elem(ret, ret);
  ret->t = ROOT;

  char *text,
       *text_orig_p,
       *bak_text;
  int text_sz,
      tt_sz,
      i;
  HTML_elem_type tmp_t;
#ifdef USE_9
  /* char *text -> full file
   * text_sz -> full file sz
   */

  /* i _will not_ read about <bio.h>, just because i can't be bothered
   * in __the future__ i'll fix this (TODO)
   */
  text_sz = seek(fp, 0, 2);
  seek(fp, 0, 0);
  text = malloc(text_sz + 1);
  read(fp, text, text_sz);
#else
  fseek(fp, 0, SEEK_END);
  text_sz = ftell(fp);
  fseek(fp, 0, SEEK_SET);

  text = malloc(text_sz + 1);
  fread(text, 1, text_sz, fp);
#endif
  text[text_sz] = 0;
  text_orig_p = text;

  while (*(text+1)) {
    while (iswspace(*text))
      ++text;

    if (*text == '<') {
      ++text;
      tmp_t = get_elem_type(text);
      bak_text = text;

      while (*text++ != '>')
        ;
      if (tmp_t == INTERNAL_BACK) {
        cur = cur->parent;
      } else {
        /*printf("text - 2 = %s\n", text-2);*/
        if (*(text-2) == '/' || tmp_t == IMAGE || tmp_t == BREAK_LINE 
            || tmp_t == META) {
          /*info("giga kutas");*/
          cur->child_n++;
          cur->child = realloc(cur->child, sizeof(HTML_elem) * cur->child_n);
          init_HTML_elem(&cur->child[cur->child_n-1], cur);
          cur->child[cur->child_n-1].t = tmp_t;

          get_elem_args(&cur->child[cur->child_n-1], bak_text);
        } else {
          cur->child_n++;
          cur->child = realloc(cur->child, sizeof(HTML_elem) * cur->child_n);
          init_HTML_elem(&cur->child[cur->child_n-1], cur);

          cur = &cur->child[cur->child_n-1];
          cur->t = tmp_t;
          get_elem_args(cur, bak_text);
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
      if (*text)
        --text;

      cur->TT_val = malloc(tt_sz + 1);
      if (tt_sz <= 0) {
        cur->TT_val[1] = 0;
        ++text;
      } else {
        strncpy(cur->TT_val, text-tt_sz, tt_sz);
        cur->TT_val[tt_sz] = 0;
      }

      for (i = 0; i < tt_sz; ++i)
        if (cur->TT_val[i] == '\n')
          cur->TT_val[i] = ' ';
        

      cur = cur->parent;
    }
  }

  fucking_update_tt_parentship(ret);
  /*html_print_tree(ret, 0, stdout);*/

  if (cur != ret)
    warn("%s: your html is bad: cur != ret (%p != %p)", __FILE__,
        cur, ret);

  free(text_orig_p);

#ifdef USE_9
  fn_end = nsec();
  info("%s: create_HTML_tree() -> took %6.4f",
      __FILE__, (double)(fn_end - fn_start) / 1000000000);
#else
  fn_end = clock();
  info("%s: create_HTML_tree() -> took %6.4f",
      __FILE__, (double)(fn_end - fn_start) / CLOCKS_PER_SEC);
#endif
  return ret;
}

