#include "sqlite3ext.h"

SQLITE_EXTENSION_INIT1

#include <ctype.h>
#include <curl/curl.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/** url_version()
 * Gets the version string of the sqlite-url library.
 * @
 * @example
 * ```sql
 * select url_version(); --> "v0.0.0"
 * ```
 */
static void urlVersionFunc(sqlite3_context *context, int argc,
                           sqlite3_value **argv) {
  sqlite3_result_text(context, SQLITE_URL_VERSION, -1, SQLITE_STATIC);
}

/** url_debug()
 * Get debug info of the sqlite-url library.
 * @
 * @example
 * ```sql
 * select url_debug(); --> "Version: v0.0.0\nDate: 2022...."
 * ```
 */
static void urlDebugFunc(sqlite3_context *context, int argc,
                         sqlite3_value **arg) {
  const char *debug = sqlite3_mprintf(
      "Version: %s\nDate: %s\nSource: %s\nlibcurl: %s", SQLITE_URL_VERSION,
      SQLITE_URL_DATE, SQLITE_URL_SOURCE, curl_version());
  if (debug == NULL) {
    sqlite3_result_error_nomem(context);
    return;
  }
  sqlite3_result_text(context, debug, -1, SQLITE_TRANSIENT);
  sqlite3_free((void *)debug);
}

static void resultPart(sqlite3_context *context, sqlite3_value *urlValue,
                       CURLUPart upart) {
  const char *url = (const char *)sqlite3_value_text(urlValue);
  char *part;
  CURLU *h;
  CURLUcode uc;
  h = curl_url();
  if (!h) {
    sqlite3_result_error_nomem(context);
    return;
  }

  uc = curl_url_set(h, CURLUPART_URL,
                    (const char *)sqlite3_value_text(urlValue), 0);
  if (uc) {
    curl_url_cleanup(h);
    sqlite3_result_null(context);
    // sqlite3_result_error(context, curl_url_strerror(uc), -1);
    return;
  }

  uc = curl_url_get(h, upart, &part, 0);
  if (uc) {
    curl_url_cleanup(h);
    sqlite3_result_null(context);
    // sqlite3_result_error(context, curl_url_strerror(uc), -1);
    return;
  }
  sqlite3_result_text(context, part, -1, SQLITE_TRANSIENT);
  curl_free(part);
  curl_url_cleanup(h);
}

/** url_valid(url, [strict])
 ** Returns 1 if url is a well-formed URL according to libcurl.
 ** Returns 0 otherwise.
 ** @arg url - URL to check validness
 ** @example
 ** ```sql
 ** select url_valid('https://google.com'); // -> 1
 ** select url_valid('invalid'); // -> 0
 ** ```
 **/
static void urlValidFunc(sqlite3_context *context, int argc,
                         sqlite3_value **argv) {
  const char *url = (const char *)sqlite3_value_text(argv[0]);
  CURLU *h;
  CURLUcode uc;
  h = curl_url();
  if (!h) {
    sqlite3_result_error_nomem(context);
    return;
  }

  uc = curl_url_set(h, CURLUPART_URL, url, CURLU_NON_SUPPORT_SCHEME);
  if (uc) {
    sqlite3_result_int(context, 0);
  } else {
    sqlite3_result_int(context, 1);
  }

  curl_url_cleanup(h);
}

/** url_host(url)
 * Returns the host portion of the given URL. 
*/
static void urlHostFunc(sqlite3_context *context, int argc,
                        sqlite3_value **argv) {
  resultPart(context, argv[0], CURLUPART_HOST);
}

/** url_scheme(url)
 * Returns the scheme portion of the given URL. 
*/
static void urlSchemeFunc(sqlite3_context *context, int argc,
                          sqlite3_value **argv) {
  resultPart(context, argv[0], CURLUPART_SCHEME);
}

/** url_path(url)
 * Returns the path portion of the given URL. 
*/
static void urlPathFunc(sqlite3_context *context, int argc,
                        sqlite3_value **argv) {
  resultPart(context, argv[0], CURLUPART_PATH);
}

/** url_query(url)
 * Returns the query portion of the given URL. 
*/
static void urlQueryFunc(sqlite3_context *context, int argc,
                         sqlite3_value **argv) {
  resultPart(context, argv[0], CURLUPART_QUERY);
}

/** url_fragment(url)
 * Returns the fragment portion of the given URL. 
*/
static void urlFragmentFunc(sqlite3_context *context, int argc,
                            sqlite3_value **argv) {
  resultPart(context, argv[0], CURLUPART_FRAGMENT);
}

/** url_escape(url)
 * Escape the given text. 
*/
static void urlEscapeFunc(sqlite3_context *context, int argc,
                          sqlite3_value **argv) {
  int n = sqlite3_value_bytes(argv[0]);
  char *s = (char *)sqlite3_value_text(argv[0]);
  /*if(n > CURL_MAX_INPUT_LENGTH) {
    sqlite3_result_error(context, sqlite3_mprintf("curl input limit is %d",
  CURL_MAX_INPUT_LENGTH), -1); return;
  }*/
  CURL *curl = curl_easy_init();
  if (!curl) {
    sqlite3_result_error_nomem(context);
  }

  char *output = curl_easy_escape(curl, s, n);
  if (output) {
    sqlite3_result_text(context, output, -1, SQLITE_TRANSIENT);
    curl_free(output);
  } else {
    sqlite3_result_error(context, "TODO", -1);
  }
  curl_easy_cleanup(curl);
}

/** url_unescape(contents)
 * Unescape the given text. 
*/
static void urlUnescapeFunc(sqlite3_context *context, int argc,
                            sqlite3_value **argv) {
  int n = sqlite3_value_bytes(argv[0]);
  char *s = (char *)sqlite3_value_text(argv[0]);
  /*if(n > CURL_MAX_INPUT_LENGTH) {
    sqlite3_result_error(context, sqlite3_mprintf("curl input limit is %d",
  CURL_MAX_INPUT_LENGTH), -1); return;
  }*/
  CURL *curl = curl_easy_init();
  if (!curl) {
    sqlite3_result_error_nomem(context);
  }
  int len;
  char *output = curl_easy_unescape(curl, s, n, &len);
  if (output) {
    sqlite3_result_text(context, output, len, SQLITE_TRANSIENT);
    curl_free(output);
  } else {
    sqlite3_result_error(context, "TODO", -1);
  }
  curl_easy_cleanup(curl);
}

#ifdef _WIN32
__declspec(dllexport)
#endif
    int sqlite3_url_init(sqlite3 *db, char **pzErrMsg,
                         const sqlite3_api_routines *pApi) {
  int rc = SQLITE_OK;
  SQLITE_EXTENSION_INIT2(pApi);

  (void)pzErrMsg; /* Unused parameter */
  if (rc == SQLITE_OK)
    rc = sqlite3_create_function(db, "url_version", 0,
                                 SQLITE_UTF8 | SQLITE_INNOCUOUS |
                                     SQLITE_DETERMINISTIC,
                                 0, urlVersionFunc, 0, 0);
  if (rc == SQLITE_OK)
    rc = sqlite3_create_function(db, "url_debug", 0,
                                 SQLITE_UTF8 | SQLITE_INNOCUOUS |
                                     SQLITE_DETERMINISTIC,
                                 0, urlDebugFunc, 0, 0);
  if (rc == SQLITE_OK)
    rc = sqlite3_create_function(db, "url_host", 1,
                                 SQLITE_UTF8 | SQLITE_INNOCUOUS |
                                     SQLITE_DETERMINISTIC,
                                 0, urlHostFunc, 0, 0);
  if (rc == SQLITE_OK)
    rc = sqlite3_create_function(db, "url_valid", 1,
                                 SQLITE_UTF8 | SQLITE_INNOCUOUS |
                                     SQLITE_DETERMINISTIC,
                                 0, urlValidFunc, 0, 0);
  if (rc == SQLITE_OK)
    rc = sqlite3_create_function(db, "url_scheme", 1,
                                 SQLITE_UTF8 | SQLITE_INNOCUOUS |
                                     SQLITE_DETERMINISTIC,
                                 0, urlSchemeFunc, 0, 0);
  if (rc == SQLITE_OK)
    rc = sqlite3_create_function(db, "url_path", 1,
                                 SQLITE_UTF8 | SQLITE_INNOCUOUS |
                                     SQLITE_DETERMINISTIC,
                                 0, urlPathFunc, 0, 0);
  if (rc == SQLITE_OK)
    rc = sqlite3_create_function(db, "url_query", 1,
                                 SQLITE_UTF8 | SQLITE_INNOCUOUS |
                                     SQLITE_DETERMINISTIC,
                                 0, urlQueryFunc, 0, 0);
  if (rc == SQLITE_OK)
    rc = sqlite3_create_function(db, "url_fragment", 1,
                                 SQLITE_UTF8 | SQLITE_INNOCUOUS |
                                     SQLITE_DETERMINISTIC,
                                 0, urlFragmentFunc, 0, 0);
  if (rc == SQLITE_OK)
    rc = sqlite3_create_function(db, "url_escape", 1,
                                 SQLITE_UTF8 | SQLITE_INNOCUOUS |
                                     SQLITE_DETERMINISTIC,
                                 0, urlEscapeFunc, 0, 0);
  if (rc == SQLITE_OK)
    rc = sqlite3_create_function(db, "url_unescape", 1,
                                 SQLITE_UTF8 | SQLITE_INNOCUOUS |
                                     SQLITE_DETERMINISTIC,
                                 0, urlUnescapeFunc, 0, 0);
  return rc;
}
