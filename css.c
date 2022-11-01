#include "ns.h"
#include "css_colors.h"

#ifdef USE_9
#else
#include <stdlib.h>
#include <string.h>
#include <wctype.h>
#include <ctype.h>
#include <math.h>
#endif

#define skipspace(v) while(isspace(*v)) v++

static char *PREDEF_CSS = "\
* {\
  font-family: monospace;\
  font-weight: normal;\
  font-style: normal;\
  color: black;\
  background-color: white;\
  font-size: 15px;\
}\
a {\
  color: blue;\
}\
b {\
  font-weight: bold;\
}\
i {\
  font-style: italic;\
}\
td { border-width: 1px; border-color: pink; }\
table { border-width: 1px; border-color: pink; }\
h1 { font-size: 40px; font-weight: bold; }\
h2 { font-size: 30px; font-weight: bold; }\
h3 { font-size: 25px; font-weight: bold; }\
h4 { font-size: 16px; font-weight: bold; }\
h5 { font-size: 13px; font-weight: bold; }\
h6 { font-size:  9px; font-weight: bold; }\
";

char *internal_color_to_str(uint32_t c) {
  char *ret = malloc(8);
#if __LITTLE_ENDIAN__ || __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
  snprintf(ret, 8, "#%02x%02x%02x", c >> 16 & 0xff , c >> 8 & 0xff,
      c >> 0 & 0xff);
#else
#warning untested :^)
  snprintf(ret, 8, "#%02x%02x%02x", c >> 0 & 0xff , c >> 8 & 0xff,
      c >> 16 & 0xff);
#endif
  ret[7] = 0;
  return ret;
}

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

  for (i = 0; i < c->o_n; ++i)
    if (c->o[i].m == M_STRING)
      if (c->o[i].v_str != NULL)
        free(c->o[i].v_str);

  if (c->o_n)
    free(c->o);
}

static CSS_otype get_css_otype(char *s) {
  int len = 0;
  char *lcase;
  CSS_otype ret;
  while (iswspace(*s)) s++;
  while (*s != ':') {
    if (*s == '}')
      return CSS_NEXT_SELECTOR;
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

  if (strcmp(lcase, "background-color") == 0) ret = BACKGROUND_COLOR;
  else if (strcmp(lcase, "color") == 0) ret = COLOR;
  else if (strcmp(lcase, "font-size") == 0) ret = FONTSIZE;
  else if (strcmp(lcase, "font-family") == 0) ret = FONT_FAMILY;
  else if (strcmp(lcase, "font-weight") == 0) ret = FONT_WEIGHT;
  else if (strcmp(lcase, "font-style") == 0) ret = FONT_STYLE;
  else if (strcmp(lcase, "border-width") == 0) ret = BORDER_WIDTH;
  else if (strcmp(lcase, "border-color") == 0) ret = BORDER_COLOR;

  else ret = CSS_UNKNOWN;

  free(lcase);
  return ret;
}

static char *get_css_val(char *stl, int isinline) {
  int len = 0;
  char *ret;
  while (iswspace(*stl))
    stl++;
  while (*stl != ';') {
    if (!*stl) {
      if (isinline)
        break;
      warn("%s: css value ends abruptly (near '%s')", __FILE__, stl-10);
      return NULL;
    }

    ++len;
    ++stl;
  }

  ret = malloc(len + 1);
  strncpy(ret, stl-len, len);
  ret[len] = 0;

  return ret;
}

static int count_whitespace(char *s, int n) {
  int ret = 0;

  while (n--)
    if (isspace(*s++)) ret++;

  return ret;
}

#ifdef WITH_HSL
static double hue2rgb(double p, double q, double t) {
  if (t < 0) t++;
  if (t > 1) t--;
  if (t < 1 / 6) return p + (q - p) * 6 * t;
  if (t < 1 / 2) return q;
  if (t < 2 / 3) return p + (q - p) * (2/3 - t) * 6;
  return p;
  /* via http://mjijackson.com/ */
}
#endif

static int css_colname_to_int(char *v) {
  int i,
      val;
  char *n;

  /* TODO: replace this with binary search */
  for (i = 0; i < CSS_COLOR_MAP_L; ++i) {
    if (strcmp(v, CSS_COLOR_MAP[i].n) == 0) {
      return CSS_COLOR_MAP[i].v;
    }
  }

  if ((n = strchr(v, '(')) != NULL) {
    /* it's hsl() or rgb() etc. */
    char *fun_n = malloc(n - v + 1),
         *ex = malloc(4),
         *tmp_c = malloc(7);
    short val[3] = {0};
    long ret;
    int x,
        type,
        cur_v = 0;
    enum { t_rgb = 0, t_hsl = 1, t_rgba = 2, t_hsla = 3 };

    strncpy(fun_n, v, n - v);
    fun_n[n - v] = 0;

    if      (strcmp(fun_n, "rgb" ) == 0) type = t_rgb;
    else if (strcmp(fun_n, "rgba") == 0) type = t_rgba;
    else if (strcmp(fun_n, "hsl" ) == 0) type = t_hsl;
    else if (strcmp(fun_n, "hsla") == 0) type = t_hsla;
    else {
      warn("%s: unknown '%s()' - using as rgb()", __FILE__, fun_n);
      type = t_rgb;
    }

    if (type > 1)
      warn("%s: alpha in colors not implemented (%s)", __FILE__, v);

    if (type > 1)
      strcpy(ex, ",,,");
    else
      strcpy(ex, ",,)");
    v = n + 1;

    while (*ex) {
      skipspace(v);
      x = strchr(v, *ex++) - v;
      x -= count_whitespace(v, x) + 1;
      while (isdigit(*v))
        val[cur_v] += (*v++ - '0') * pow(10, x--);
      skipspace(v);
      v++, cur_v++;
    }

    if (type == t_hsl || type == t_hsla) {
#ifdef WITH_HSL
      /* thank you michael jackson (http://mjijackson.com) for that
       *
       * this is not my code and i have _no clue_ what it is doing
       * i mean - nothing harmless, but i just don't have the brains to figure
       * out how to convert hsl -> rgb
       */
      double r,
             g,
             b,
             p,
             q,
             h = val[0],
             s = val[1],
             l = val[2];

      h /= 100, s /= 100, l /= 100; /* assumes h, s and l are in % */

      if (s == 0) {
        r = g = b = l;
      } else {
        q = l < 0.5 ? l * (1 + s) : l + s - l * s;
        p = 2 * l - q;
        r = hue2rgb(p, q, h + (double)(1/3));
        g = hue2rgb(p, q, h);
        b = hue2rgb(p, q, h - (double)(1/3));
      }

      val[0] = r * 255;
      val[1] = g * 255;
      val[2] = b * 255;
#else
      warn("%s: compiled without hsl() support", __FILE__);
      val[0] = 255;
      val[1] = 0;
      val[2] = 255;
#endif
    }

    /* TODO: faster solution without snprintf */
    snprintf(tmp_c, 7, "%.2x%.2x%.2x", val[0], val[1], val[2]);
    ret = strtoul(tmp_c, NULL, 16);

    free(ex - 3);
    free(tmp_c);
    return ret;
  } else {
    warn("%s: unknown color '%s'", __FILE__, v);
    return 0xff00ff;
  }
}

/* CSS_otype has to be specified in the struct before calling that function */
static void css_str_to_val_metric(CSS_opt *opt, char *v) {
  int i = 0;

  switch (opt->t) {
    /* put anything color-related here */
    case COLOR:
    case BACKGROUND_COLOR:
      opt->m = M_COLOR;
      switch (*v) {
        case '#':
          opt->v = strtoul(++v, NULL, 16);
          return;
          break;
        default:
          opt->v = css_colname_to_int(v);
          return;
          break;
      }
      break;
    case FONT_FAMILY:
      /* or anything else that requires type M_STRING */
      opt->m = M_STRING;
      opt->v_str = malloc(strlen(v) + 1);
      strcpy(opt->v_str, v);
      opt->v_str[strlen(v)] = 0;
      return;
    /* anything size-related */
    case FONTSIZE:
      while (isdigit(*v))
        ++i, ++v;

      if (strcmp(v, "px") == 0) opt->m = M_PIXEL;
      if (strcmp(v, "in") == 0) opt->m = M_INCH;
      if (strcmp(v, "%") == 0) opt->m = M_PERCENT;
      else opt->m = M_PIXEL;

      opt->v = atoi(v - i);
      return;
      break;
    case FONT_WEIGHT:
      opt->m = M_NUMBER;

      if (isdigit(*v)) {
        while (isdigit(*v))
          ++i, ++v;
        opt->v = atoi(v - i);
      } else {
        /* choose normal, bold, lighter, bolder */
        if (strcmp("bold", v) == 0) opt->v = 700;
        else if (strcmp("normal", v) == 0) opt->v = 400;
        else if (strcmp("lighter", v) == 0) opt->v = 300;
        else if (strcmp("bolder", v) == 0) opt->v = 500;
        else {
          warn("%s: unsupported option for FONT_WEIGHT - '%s'", __FILE__, v);
          opt->v = 400;
        }
      }

      return;
      break;
    case FONT_STYLE:
      opt->m = M_STRING;
      opt->v_str = malloc(8); /* strlen("oblique") + 1   :^) */
      
      if (strcmp("normal", v) == 0) strcpy(opt->v_str, "roman");
      else if (strcmp("italic", v) == 0) strcpy(opt->v_str, v);
      else if (strcmp("oblique", v) == 0) strcpy(opt->v_str, v);
      else {
        warn("%s: unsupported option for FONT_STYLE - '%s'", __FILE__, v);
        strcpy(opt->v_str, "roman");
      }

      return;
      break;
    case CSS_UNKNOWN:
    case CSS_NEXT_SELECTOR:
      break;
  }

  opt->v = -1;
  opt->v_str = NULL;
}

/* returns a new calculated_css with copied opts :^) */
Calculated_CSS csscpy(Calculated_CSS *c) {
  Calculated_CSS ret;
  int i;

  ret.o_n = c->o_n;
  ret.o = malloc(sizeof(CSS_opt) * ret.o_n);

  for (i = 0; i < ret.o_n; ++i) {
    ret.o[i].t = c->o[i].t;
    ret.o[i].m = c->o[i].m;

    if (ret.o[i].m == M_STRING) {
      ret.o[i].v_str = malloc(strlen(c->o[i].v_str) + 1);
      strcpy(ret.o[i].v_str, c->o[i].v_str);
      ret.o[i].v_str[strlen(c->o[i].v_str)] = 0;
    } else {
      ret.o[i].v = c->o[i].v;
    }
  }

  return ret;
}

void calculate_css(HTML_elem *el) {
  /* oh it will be inefficient */
  int i,
      checked_inline = 0,
      isinline = 0,
      set_me,
      len;
  char *stl,
       *orig_stl,
       *tmp_val;
  HTML_elem_type tmp_type;
  CSS_otype tmp_otype;

  if (el->t == TEXT_TYPE)
    return;

  if (el->css.o != NULL) free_css(&el->css);
  el->css.o = NULL;
  el->css.o_n = 0;

  if (el->t != ROOT)
    calculate_css(el->parent);

  stl = find_styles(get_root_from_elem(el));

  if (stl == NULL) {
    stl = malloc(strlen(PREDEF_CSS) + 1);
    strcpy(stl, PREDEF_CSS);
    stl[strlen(PREDEF_CSS)] = 0;
  } else {
    len = strlen(stl);
    stl = realloc(stl, len + strlen(PREDEF_CSS) + 1);
    tmp_val = malloc(len + strlen(PREDEF_CSS) + 1);
    strcpy(tmp_val, PREDEF_CSS);
    strcat(tmp_val, stl);
    strcpy(stl, tmp_val);

    stl[strlen(PREDEF_CSS) + len] = 0;
    free(tmp_val);
  }

  orig_stl = stl;

  if (stl == NULL) goto real_end;
  while (*stl) {
    while (iswspace(*stl)) stl++;
    if (!*stl) goto end;

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

    if (tmp_type != el->t && tmp_type != CSS_ALL_SELECTORS)
      if (!(el->parent->t == tmp_type && el->t == TEXT_TYPE))
        goto next;

    while (*stl++ != '{')
      ;

next_opt:
    if (isinline) {
      while (iswspace(*stl)) stl++;
      if (!*stl) goto end;
    }
    tmp_otype = get_css_otype(stl);
    if (tmp_otype == CSS_NEXT_SELECTOR)
      goto next;

    if (tmp_otype == CSS_UNKNOWN) {
      warn("%s: unknown css option near %.5s[...] (elem %p)", __FILE__,
          stl, el);
      goto nextopt_plus_movestl;
    }

    while (*stl++ != ':')
      ;

    tmp_val = get_css_val(stl, isinline);
    if (tmp_val == NULL)
      goto nextopt_plus_movestl;
    /* yeahhh man fuckkk that */

    set_me = -1;
    for (i = 0; i < el->css.o_n; ++i)
      if (el->css.o[i].t == tmp_otype)
        set_me = i;

    if (set_me < 0) {
      el->css.o = realloc(el->css.o, (el->css.o_n + 1) * sizeof(CSS_opt));
      el->css.o[el->css.o_n].t = tmp_otype;
      css_str_to_val_metric(&el->css.o[el->css.o_n], tmp_val);
      el->css.o_n++;
    } else {
      el->css.o[set_me].t = tmp_otype;
      css_str_to_val_metric(&el->css.o[set_me], tmp_val);
    }

    free(tmp_val);

nextopt_plus_movestl:
    while (*stl++ != ';')
      if (!*stl) goto end;
    goto next_opt;
next:
    while (*stl != '}') {
      if (!*stl) {
        stl--;
        break;
      }

      ++stl;
    }
    ++stl;
  }
end:
  free(orig_stl);

real_end:
  if (!checked_inline) {
    for (i = 0; i < el->argc; ++i) {
      if (strcmp(el->argv[i][0], "style") == 0) {
        stl = malloc(strlen(el->argv[i][1]) + 1);
        strcpy(stl, el->argv[i][1]);
        stl[strlen(el->argv[i][1])] = 0;
        orig_stl = stl;

        checked_inline = 1;
        isinline = 1;
        goto next_opt;
      }
    }
  }

  for (i = 0; i < el->child_n; ++i)
    el->child[i].css = csscpy(&el->css);
}

