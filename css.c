#include "ns.h"

#include <stdlib.h>
#include <string.h>
#include <wctype.h>

/* update if needed */
static int total_hours_wasted_here = 2;
/* 17:40 */

static HTML_elem *get_root_from_elem(HTML_elem *e) {
  return (e->t == ROOT) ? e : get_root_from_elem(e->parent);
}
static char *find_styles(HTML_elem *root) {
  int i,
      j,
      wasnull = 0,
      l;
  char *ret = NULL,
       *tmp = NULL;

  for (i = 0; i < root->child_n; ++i) {
    if (root->child[i].t != STYLE) {
      tmp = find_styles(&root->child[i]);

      if (tmp != NULL) {
        if (ret == NULL) {
          wasnull = 1;
          l = 0;
        } else {
          l = strlen(ret);
        }
        ret = realloc(ret, l + strlen(tmp) + 1);
        if (wasnull) *ret = 0;
        strcat(ret, tmp);
        wasnull = 0;
      }
    } else {
      for (j = 0; j < root->child[i].child_n; ++j) {
        if (root->child[i].child[j].t != TEXT_TYPE) {
          warn("%s: invalid child of style %p (%s instead of TEXT_TYPE)",
              __FILE__, &root->child[i].child[j], 
              elemt_to_str(root->child[i].child[j].t));
        } else {
          if (ret == NULL) {
            wasnull = 1;
            l = 0;
          } else {
            l = strlen(ret);
          }
          /*warn("tt_val = %s", root->child[i].child[j].TT_val);*/
          ret = realloc(ret, l + (int)strlen(root->child[i].child[j].TT_val) 
              + 1);
          if (wasnull) *ret = 0;
          strcat(ret, root->child[i].child[j].TT_val);
          wasnull = 0;
        }
      }
    }
  }

  return ret;
}

void free_css(Calculated_CSS *c) {
  int i;

  for (i = 0; i < c->o_n; ++i);
    /*if (c->o->value != NULL)*/
      /*free(c->o->value);*/
  if (c->o != NULL)
    free(c->o);
}

CSS_otype get_css_otype(char *s) {
  int len = 0;
  char *lcase;
  CSS_otype ret;
  while (iswspace(*s)) s++;
  while (*s != ':') {
    if (!*s) {
      warn("%s: css ends prematurely (near %s)", __FILE__, s-5);
      break;
    }
    s++;
    len++;
  }

  lcase = malloc(len + 1);
  lcase[len] = 0;
  cpt_to_lower(s - len, lcase, len);
  warn(lcase);

  if (strcmp(lcase, "background-color") == 0) ret = BACKGROUND_COLOR;
  else if (strcmp(lcase, "color") == 0) ret = COLOR;
  else if (strcmp(lcase, "font-size") == 0) ret = FONTSIZE;
  else if (strcmp(lcase, "font-family") == 0) ret = FONT_FAMILY;
  else ret = CSS_UNKNOWN;

  free(lcase);
  return ret;
}

char *get_css_val(char *stl) {
  while (iswspace(*stl))
    stl++;
}

void calculate_css(HTML_elem *el) {
  /* oh it will be inefficient */
  int i;
  char *stl,
       *orig_stl,
       *tmp_val;
  HTML_elem_type tmp_type;
  CSS_otype tmp_otype;

  if (el->css.o != NULL) free_css(&el->css);
  el->css.o = 0;

  if (el->t != ROOT)
    calculate_css(el->parent);
  
  stl = find_styles(get_root_from_elem(el));
  orig_stl = stl;

  if (stl == NULL) return;
  /*info("stl: %s", stl);*/

  while (*stl) {
    while (iswspace(*stl)) stl++;
    if (!*stl) break;

    if (*stl == '.' || *stl == '#') { 
      warn("%s: id or class not supported in css (near %.5s[...]) (elem %p)",
          __FILE__, stl, el);
      goto next;
    }

    tmp_type = get_elem_type(stl);
    if (tmp_type == UNKNOWN) {
      warn("%s: unknown css selector near %.5s[...] (elem %p)", __FILE__,
          stl, el);
      goto next;
    }

    if (tmp_type != el->t)
      goto next;

    while (*stl++ != '{')
      ;

    tmp_otype = get_css_otype(stl);
    if (tmp_otype == CSS_UNKNOWN) {
      warn("%s: unknown css option near %.5s[...] (elem %p)", __FILE__,
          stl, el);
      goto next;
    }

    while (*stl++ != ':')
      ;

    tmp_val = get_css_val(stl);

    el->css.o_n++;
    el->css.o = realloc(el->css.o, el->css.o_n * sizeof(CSS_opt));
    el->css.o[el->css.o_n].t = tmp_otype;
    el->css.o[el->css.o_n].t = tmp_otype;
next:
    while (*stl != '}') {
      if (!*stl)
        break;

      ++stl;
    }
    ++stl;
  }
  free(orig_stl);
}

