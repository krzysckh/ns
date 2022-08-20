#include "ns.h"

#ifdef USE_CURL
#include <curl/curl.h>
#endif

FILE *download_file(char *url) {
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
      warn("%s: curl_easy_perform() failed lol", __FILE__);
    curl_easy_cleanup(c);
  }   

  return ret;
#elif defined(USE_9)
  warn("%s: compiled on plan9* - cannot download %s - not implemented",
      __FILE__, url);
  return fopen("/dev/null", "r");
#else
  warn("%s: compiled without libcurl - alternative methods not implemented "
      "- cannot download %s", __FILE__, url);
  return fopen("/dev/null", "r");
  /* ;=; */
#endif
}
