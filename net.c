#include "ns.h"

#ifdef USE_9
#endif

#ifdef USE_CURL
#include <curl/curl.h>
#endif

static char *cur_website = NULL;

#ifdef USE_9
int download_file(char *url) {
#else
FILE *download_file(char *url) {
#endif
  char *tmp_w,
       *actual_url = NULL;
  if (*url == '/' && *(url+1) == '/')
    url += 2;
  /* e.g. //dwm.suckless.org on suckless.org */
  else if (strncmp("https://", url, 8) == 0 || strncmp("http://", url, 7) == 0) {
    tmp_w = url + strlen(url);
    while (*tmp_w-- != '/')
      ; /* go to last backslash (curr directory) */
    if (tmp_w - url < 9) {/* the last slash is from https?:// */
      if (cur_website != NULL)
        free(cur_website);
      cur_website = malloc(strlen(url) + 2);
      strcpy(cur_website, url);
      cur_website[strlen(url)] = '/';
      cur_website[strlen(url)+1] = 0;
    } else {
      if (cur_website != NULL)
        free(cur_website);
      cur_website = malloc(strlen(url) - (tmp_w - url) + 1);
      strncpy(cur_website, url, strlen(url) - (tmp_w - url));
      cur_website[strlen(url) - (tmp_w - url)] = 0;
    }
  } else {
    if (cur_website == NULL)
      err("%s: bad url %s", __FILE__, url);
    else {
      actual_url = malloc(strlen(url) + strlen(cur_website) + 1);
      strcpy(actual_url, cur_website);
      strcat(actual_url, url);
      url = actual_url;
    }
  }

#ifdef USE_CURL
  info("%s: getting %s using curl", __FILE__, url);
  CURL *c;
  FILE *ret = tmpfile();
  CURLcode res;
  c = curl_easy_init();

  if (c) {   
    curl_easy_setopt(c, CURLOPT_URL, url);
    curl_easy_setopt(c, CURLOPT_WRITEFUNCTION, NULL);
    curl_easy_setopt(c, CURLOPT_WRITEDATA, ret);
    res = curl_easy_perform(c);
    if (res)
      err("%s: curl_easy_perform() failed. bad link? bug?", __FILE__);
    curl_easy_cleanup(c);
  }   

  if (actual_url)
    free(actual_url);
  return ret;
#elif defined(USE_9)
  warn("%s: compiled on plan9* - cannot download %s - not implemented",
      __FILE__, url);
  return open("/dev/null", OREAD);
#else
  warn("%s: compiled without libcurl - alternative methods not implemented "
      "- cannot download %s", __FILE__, url);
  return fopen("/dev/null", "r");
  /* ;=; */
#endif
}
