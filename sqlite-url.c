#include "sqlite3ext.h"

SQLITE_EXTENSION_INIT1

#include <ctype.h>
#include <curl/curl.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#pragma region meta functions

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

#pragma endregion

#pragma region library functions

// Used in most extraction functions.
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

  uc =
      curl_url_set(h, CURLUPART_URL, (const char *)sqlite3_value_text(urlValue),
                   CURLU_NON_SUPPORT_SCHEME);
  if (uc) {
    curl_url_cleanup(h);
    sqlite3_result_null(context);
    return;
  }

  uc = curl_url_get(h, upart, &part, CURLU_NON_SUPPORT_SCHEME);
  if (uc) {
    curl_url_cleanup(h);
    sqlite3_result_null(context);
    return;
  }
  sqlite3_result_text(context, part, -1, SQLITE_TRANSIENT);
  curl_free(part);
  curl_url_cleanup(h);
}

/** url(url [, name1, value1], [...])
 * Generate a URL. The first "url" parameter is a base URL that is parsed, and
 *can be overwritten by the other parameters. If "url" is null or the empty
 *string, Then only the other parameters are used. "nameN" parameters must be
 *one of the following:
 *  - "scheme":
 *  - "host":
 *  - "path":
 *  - "query":
 *  - "fragment":
 **/
static void urlFunc(sqlite3_context *context, int argc, sqlite3_value **argv) {
  CURLU *h;
  CURLUcode uc;
  h = curl_url();
  if (!h) {
    sqlite3_result_error_nomem(context);
    return;
  }
  if (argc % 2 != 1) {
    sqlite3_result_error(context, "url() requires odd number of arguments", -1);
    return;
  }

  if (sqlite3_value_bytes(argv[0]) > 0 &&
      sqlite3_value_type(argv[0]) != SQLITE_NULL) {
    const char *url = (const char *)sqlite3_value_text(argv[0]);
    uc = curl_url_set(h, CURLUPART_URL, url, CURLU_NON_SUPPORT_SCHEME);
    if (uc) {
      curl_url_cleanup(h);
      sqlite3_result_error(context, "Error initializating URL in the first argument", -1);
      return;
    }
  }
  for (int i = 1; i < argc; i += 2) {
    char *partName = (char *)sqlite3_value_text(argv[i]);
    char *partValue = (char *)sqlite3_value_text(argv[i + 1]);

    if (sqlite3_stricmp(partName, "host") == 0) {
      uc = curl_url_set(h, CURLUPART_HOST, partValue, CURLU_NON_SUPPORT_SCHEME);
      if (uc) {
        curl_url_cleanup(h);
        sqlite3_result_error(context, "Invalid 'host' value", -1);
        return;
      }
    } else if (sqlite3_stricmp(partName, "path") == 0) {
      uc = curl_url_set(h, CURLUPART_PATH, partValue, CURLU_NON_SUPPORT_SCHEME);
      if (uc) {
        curl_url_cleanup(h);
        sqlite3_result_error(context, "Invalid 'path' value", -1);
        return;
      }
    } else if (sqlite3_stricmp(partName, "scheme") == 0) {
      uc = curl_url_set(h, CURLUPART_SCHEME, partValue,
                        CURLU_NON_SUPPORT_SCHEME);
      if (uc) {
        curl_url_cleanup(h);
        sqlite3_result_error(context, curl_url_strerror(uc), -1);
        return;
      }
    } else if (sqlite3_stricmp(partName, "query") == 0) {
      uc =
          curl_url_set(h, CURLUPART_QUERY, partValue, CURLU_NON_SUPPORT_SCHEME);
      if (uc) {
        curl_url_cleanup(h);
        sqlite3_result_error(context, "Invalid 'query' value", -1);
        return;
      }
    } else if (sqlite3_stricmp(partName, "fragment") == 0) {
      uc = curl_url_set(h, CURLUPART_FRAGMENT, partValue,
                        CURLU_NON_SUPPORT_SCHEME);
      if (uc) {
        curl_url_cleanup(h);
        sqlite3_result_error(context, "Invalid 'fragment' value", -1);
        return;
      }
    } else if (sqlite3_stricmp(partName, "user") == 0) {
      uc = curl_url_set(h, CURLUPART_USER, partValue,
                        CURLU_NON_SUPPORT_SCHEME);
      if (uc) {
        curl_url_cleanup(h);
        sqlite3_result_error(context, "Invalid 'user' value", -1);
        return;
      }
    } else if (sqlite3_stricmp(partName, "password") == 0) {
      uc = curl_url_set(h, CURLUPART_PASSWORD, partValue,
                        CURLU_NON_SUPPORT_SCHEME);
      if (uc) {
        curl_url_cleanup(h);
        sqlite3_result_error(context, "Invalid 'password' value", -1);
        return;
      }
    } else if (sqlite3_stricmp(partName, "options") == 0) {
      uc = curl_url_set(h, CURLUPART_OPTIONS, partValue,
                        CURLU_NON_SUPPORT_SCHEME);
      if (uc) {
        curl_url_cleanup(h);
        sqlite3_result_error(context, "Invalid 'options' value", -1);
        return;
      }
    } else if (sqlite3_stricmp(partName, "zoneid") == 0) {
      uc = curl_url_set(h, CURLUPART_ZONEID, partValue,
                        CURLU_NON_SUPPORT_SCHEME);
      if (uc) {
        curl_url_cleanup(h);
        sqlite3_result_error(context, "Invalid 'zoneid' value", -1);
        return;
      }
    } else {
      sqlite3_result_error(
          context, sqlite3_mprintf("unknown url part '%s'", partName), -1);
      return;
    }
  }
  char *out;
  int rc = curl_url_get(h, CURLUPART_URL, &out, CURLU_NON_SUPPORT_SCHEME);
  if (rc) {
    sqlite3_result_error(context, curl_url_strerror(rc), -1);

  } else {
    sqlite3_result_text(context, out, -1, SQLITE_TRANSIENT);
    curl_free(out);
  }

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

/** url_user(url)
 * Returns the user portion of the given URL.
 */
static void urlUserFunc(sqlite3_context *context, int argc,
                        sqlite3_value **argv) {
  resultPart(context, argv[0], CURLUPART_USER);
}

/** url_password(url)
 * Returns the password portion of the given URL.
 */
static void urlPasswordFunc(sqlite3_context *context, int argc,
                            sqlite3_value **argv) {
  resultPart(context, argv[0], CURLUPART_PASSWORD);
}

/** url_options(url)
 * Returns the options portion of the given URL.
 */
static void urlOptionsFunc(sqlite3_context *context, int argc,
                           sqlite3_value **argv) {
  resultPart(context, argv[0], CURLUPART_OPTIONS);
}

/** url_port(url)
 * Returns the port portion of the given URL.
 */
static void urlPortFunc(sqlite3_context *context, int argc,
                        sqlite3_value **argv) {
  resultPart(context, argv[0], CURLUPART_PORT);
}

/** url_zoneid(url)
 * Returns the zoneid portion of the given URL.
 */
static void urlZoneidFunc(sqlite3_context *context, int argc,
                          sqlite3_value **argv) {
  resultPart(context, argv[0], CURLUPART_ZONEID);
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
    sqlite3_result_error(context, "Error escaping argument", -1);
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
    sqlite3_result_error(context, "Error unescaping argument", -1);
  }
  curl_easy_cleanup(curl);
}

/** url_querystring(name1, value1, [...])
 * Generate a query string with the given names and values.
 * Individual segments are automatically escaped.
 * https://url.spec.whatwg.org/#urlencoded-serializing
 */
static void urlQuerystringFunc(sqlite3_context *context, int argc,
                               sqlite3_value **argv) {
  sqlite3 *db = sqlite3_context_db_handle(context);
  char * errmsg;
  if (argc < 2) {
    sqlite3_result_error(
        context, "at least 2 arguments are required for url_querystring", -1);
    return;
  }
  sqlite3_str *result = sqlite3_str_new(db);
  CURL *curl = curl_easy_init();
  if (!curl) {
    sqlite3_result_error_nomem(context);
    return;
  }

  for (int i = 0; i < argc; i += 2) {
    char *name = (char *)sqlite3_value_text(argv[i]);
    int nName = sqlite3_value_bytes(argv[i]);
    if (i != 0) {
      sqlite3_str_appendchar(result, 1, '&');
    }
    char *nameOut = curl_easy_escape(curl, name, nName);
    if (!nameOut) {
      if((errmsg = sqlite3_mprintf("Error escaping name in argument %d", i))) {
        sqlite3_result_error(context, errmsg, -1);
      } else {
        sqlite3_result_error_nomem(context);
      }
      
      curl_easy_cleanup(curl);
      return;
    } else {
      sqlite3_str_appendall(result, nameOut);
      curl_free(nameOut);
    }
    sqlite3_str_appendchar(result, 1, '=');
    char *value = (char *)sqlite3_value_text(argv[i + 1]);
    int nValue = sqlite3_value_bytes(argv[i + 1]);

    char *valueOut = curl_easy_escape(curl, value, nValue);
    if (!valueOut) {
      if((errmsg = sqlite3_mprintf("Error escaping value in argument %d", i+1))) {
        sqlite3_result_error(context, errmsg, -1);
      } else {
        sqlite3_result_error_nomem(context);
      }
      curl_easy_cleanup(curl);
      return;
    } else {
      sqlite3_str_appendall(result, valueOut);
      curl_free(valueOut);
    }
  }

  char *res = sqlite3_str_finish(result);
  if (res == 0) {
    sqlite3_result_error_code(context, sqlite3_str_errcode(result));
  } else {
    sqlite3_result_text(context, res, -1, SQLITE_TRANSIENT);
  }

  curl_easy_cleanup(curl);
}

#pragma endregion

#pragma region table functions

#pragma region url_query_each

/** select * from url_query_each(query)
 * Table function that returns each sequence in the given
 * query string. Every sequence in the query string returns
 * its own row, and every name and value are pre-escaped.
 * https://url.spec.whatwg.org/#urlencoded-parsing
 */

#define URL_QUERY_EACH_COLUMN_ROWID -1
#define URL_QUERY_EACH_COLUMN_QUERY 0
#define URL_QUERY_EACH_COLUMN_RAWSEQUENCE 1
#define URL_QUERY_EACH_COLUMN_NAME 2
#define URL_QUERY_EACH_COLUMN_VALUE 3

typedef struct url_query_each_cursor url_query_each_cursor;
struct url_query_each_cursor {
  // Base class - must be first
  sqlite3_vtab_cursor base;
  sqlite3_int64 iRowid;
  // curl client to use, to escape names/values
  CURL *curl;
  // raw string of the query string to parse
  // note: might replace '+' with ' '
  char *querystring;
  // length of querystring
  int querystringLength;
  // boolean, if all sequences in querystring have been read
  int complete;
  // pointer?
  int i;
  // pointer?
  int seqStart;
};

/*
** The urlQueryEachReadConnect() method is invoked to create a new
** urlQueryEach_vtab that describes the urlQueryEach virtual table.
**
** Think of this routine as the constructor for urlQueryEach_vtab objects.
**
** All this routine needs to do is:
**
**    (1) Allocate the urlQueryEach_vtab object and initialize all fields.
**
**    (2) Tell SQLite (via the sqlite3_declare_vtab() interface) what the
**        result set of queries against urlQueryEach will look like.
*/
static int urlQueryEachConnect(sqlite3 *db, void *pUnused, int argcUnused,
                               const char *const *argvUnused,
                               sqlite3_vtab **ppVtab, char **pzErrUnused) {
  sqlite3_vtab *pNew;
  int rc;
  (void)pUnused;
  (void)argcUnused;
  (void)argvUnused;
  (void)pzErrUnused;
  rc = sqlite3_declare_vtab(db, "CREATE TABLE x(query hidden, raw_sequence "
                                "text hidden, name text, value text)");
  if (rc == SQLITE_OK) {
    pNew = *ppVtab = sqlite3_malloc(sizeof(*pNew));
    if (pNew == 0)
      return SQLITE_NOMEM;
    memset(pNew, 0, sizeof(*pNew));
    sqlite3_vtab_config(db, SQLITE_VTAB_INNOCUOUS);
  }
  return rc;
}

/*
** This method is the destructor for url_query_each_cursor objects.
*/
static int urlQueryEachDisconnect(sqlite3_vtab *pVtab) {
  sqlite3_free(pVtab);
  return SQLITE_OK;
}

/*
** Constructor for a new url_query_each_cursor object.
*/
static int urlQueryEachOpen(sqlite3_vtab *pUnused,
                            sqlite3_vtab_cursor **ppCursor) {
  url_query_each_cursor *pCur;
  (void)pUnused;
  pCur = sqlite3_malloc(sizeof(*pCur));
  if (pCur == 0)
    return SQLITE_NOMEM;
  memset(pCur, 0, sizeof(*pCur));
  *ppCursor = &pCur->base;
  pCur->curl = curl_easy_init();
  if (!pCur->curl) {
    return SQLITE_NOMEM;
  }
  
  return SQLITE_OK;
}

/*
** Destructor for a url_query_each_cursor.
*/
static int urlQueryEachClose(sqlite3_vtab_cursor *cur) {
  url_query_each_cursor *pCur = (url_query_each_cursor *)cur;
  sqlite3_free(cur);
  curl_easy_cleanup(pCur->curl);
  return SQLITE_OK;
}

/*
** Advance a url_query_each_cursor to its next row of output.
*/
static int urlQueryEachNext(sqlite3_vtab_cursor *cur) {
  url_query_each_cursor *pCur = (url_query_each_cursor *)cur;
  pCur->iRowid++;
  while(pCur->querystring[pCur->i] == '&' && pCur->i < pCur->querystringLength) {
    pCur->i++;
  }
  if (pCur->i >= pCur->querystringLength) {
    pCur->complete = 1;
    return SQLITE_OK;
  }
  pCur->seqStart = pCur->i;

  // find the end of the current sequence, either end-of-querystring
  // or '&'
  while (pCur->i < pCur->querystringLength) {
    // "Replace any 0x2B (+) in name and value with 0x20 (SP)."
    if (pCur->querystring[pCur->i] == '+') {
      pCur->querystring[pCur->i] = 0x20;
    }
    if (pCur->querystring[pCur->i] == '&') {
      pCur->i++;
      break;
    }
    pCur->i++;
  }
  return SQLITE_OK;
}
/*
** Return TRUE if the cursor has been moved off of the last
** row of output.
*/
static int urlQueryEachEof(sqlite3_vtab_cursor *cur) {
  url_query_each_cursor *pCur = (url_query_each_cursor *)cur;
  return pCur->complete;
}

/*
** Return values of columns for the row at which the url_query_each_cursor
** is currently pointing.
*/
static int urlQueryEachColumn(
    sqlite3_vtab_cursor *cur, /* The cursor */
    sqlite3_context *ctx,     /* First argument to sqlite3_result_...() */
    int i                     /* Which column to return */
) {
  url_query_each_cursor *pCur = (url_query_each_cursor *)cur;
  sqlite3_int64 x = 0;
  switch (i) {
  case URL_QUERY_EACH_COLUMN_ROWID: {
    sqlite3_result_int(ctx, pCur->iRowid);
    break;
  }
  case URL_QUERY_EACH_COLUMN_QUERY: {
    sqlite3_result_text(ctx, pCur->querystring, pCur->querystringLength,
                        SQLITE_TRANSIENT);
    break;
  }
  case URL_QUERY_EACH_COLUMN_RAWSEQUENCE: {
    sqlite3_result_text(ctx, pCur->querystring + pCur->seqStart,
                        pCur->i - pCur->seqStart, SQLITE_TRANSIENT);
    break;
  }
  case URL_QUERY_EACH_COLUMN_NAME: {
    int nameEnd = pCur->i;
    for (int c = pCur->seqStart; c < pCur->i; c++) {
      if (pCur->querystring[c] == '=') {
        nameEnd = c;
        break;
      }
    }
    char *start = pCur->querystring + pCur->seqStart;
    int len = nameEnd - pCur->seqStart;
    int outLen;
    if (len == 0) {
      sqlite3_result_text(ctx, start, 0, SQLITE_TRANSIENT);
      return SQLITE_OK;
    }
    char *output = curl_easy_unescape(pCur->curl, start, len, &outLen);
    if (output) {
      sqlite3_result_text(ctx, output, outLen, SQLITE_TRANSIENT);
    } else {
      sqlite3_result_error_nomem(ctx);
    }
    return SQLITE_OK;
  }
  case URL_QUERY_EACH_COLUMN_VALUE: {
    int valueStart = pCur->i;
    for (int c = pCur->seqStart; c < pCur->i; c++) {
      if (pCur->querystring[c] == '=') {
        // "+1" to skip the '='
        valueStart = c + 1;
        break;
      }
    }
    char *start;
    int len;
    int outLen;
    if (pCur->querystring[pCur->i - 1] == '&') {
      // skip starting '=' and the ending '&'
      len = pCur->i - valueStart - 1;
    } else {
      len = pCur->i - valueStart;
    }
    char *output = curl_easy_unescape(pCur->curl, pCur->querystring + valueStart, len, &outLen);
    if (output) {
      sqlite3_result_text(ctx, output, outLen, SQLITE_TRANSIENT);
    } else {
      sqlite3_result_error_nomem(ctx);
    }
    return SQLITE_OK;
  }
  }
  return SQLITE_OK;
}

/*
** Return the rowid for the current row. In this implementation, the
** first row returned is assigned rowid value 1, and each subsequent
** row a value 1 more than that of the previous.
*/
static int urlQueryEachRowid(sqlite3_vtab_cursor *cur, sqlite_int64 *pRowid) {
  url_query_each_cursor *pCur = (url_query_each_cursor *)cur;
  *pRowid = pCur->iRowid;
  return SQLITE_OK;
}

/*
** SQLite will invoke this method one or more times while planning a query
** that uses the urlQueryEach virtual table.  This routine needs to create
** a query plan for each invocation and compute an estimated cost for that
** plan.
*/
static int urlQueryEachBestIndex(sqlite3_vtab *pVTab,
                                 sqlite3_index_info *pIdxInfo) {
  int hasQuery = 0;

  for (int i = 0; i < pIdxInfo->nConstraint; i++) {
    const struct sqlite3_index_constraint *pCons = &pIdxInfo->aConstraint[i];
    switch (pCons->iColumn) {
    case URL_QUERY_EACH_COLUMN_QUERY: {
      if (!hasQuery && !pCons->usable ||
          pCons->op != SQLITE_INDEX_CONSTRAINT_EQ)
        return SQLITE_CONSTRAINT;
      hasQuery = 1;
      pIdxInfo->aConstraintUsage[i].argvIndex = 1;
      pIdxInfo->aConstraintUsage[i].omit = 1;
      break;
    }
    }
  }
  if (!hasQuery) {
    pVTab->zErrMsg = sqlite3_mprintf("query argument is required");
    return SQLITE_ERROR;
  }
  pIdxInfo->idxNum = 1;
  pIdxInfo->estimatedCost = (double)100000;
  pIdxInfo->estimatedRows = 100000;

  return SQLITE_OK;
}

/*
** This method is called to "rewind" the url_query_each_cursor object back
** to the first row of output.  This method is always called at least
** once prior to any call to xColumn() or xRowid() or xEof().
**
** This routine should initialize the cursor and position it so that it
** is pointing at the first row, or pointing off the end of the table
** (so that xEof() will return true) if the table is empty.
*/
static int urlQueryEachFilter(sqlite3_vtab_cursor *pVtabCursor, int idxNum,
                              const char *idxStr, int argc,
                              sqlite3_value **argv) {
  url_query_each_cursor *pCur = (url_query_each_cursor *)pVtabCursor;
  if(sqlite3_value_type(argv[0]) == SQLITE_NULL) {
    pCur->complete = 1;
    return SQLITE_OK;  
  }
  pCur->querystring = (char *)sqlite3_value_text(argv[0]);
  pCur->querystringLength = sqlite3_value_bytes(argv[0]);
  pCur->i = 0;
  pCur->complete = 0;
  pCur->iRowid = -1;
  pCur->seqStart = 0;
  urlQueryEachNext(pVtabCursor);
  return SQLITE_OK;
}

static sqlite3_module urlQueryEachModule = {
    0,                      /* iVersion */
    0,                      /* xCreate */
    urlQueryEachConnect,    /* xConnect */
    urlQueryEachBestIndex,  /* xBestIndex */
    urlQueryEachDisconnect, /* xDisconnect */
    0,                      /* xDestroy */
    urlQueryEachOpen,       /* xOpen - open a cursor */
    urlQueryEachClose,      /* xClose - close a cursor */
    urlQueryEachFilter,     /* xFilter - configure scan constraints */
    urlQueryEachNext,       /* xNext - advance a cursor */
    urlQueryEachEof,        /* xEof - check for end of scan */
    urlQueryEachColumn,     /* xColumn - read data */
    urlQueryEachRowid,      /* xRowid - read data */
    0,                      /* xUpdate */
    0,                      /* xBegin */
    0,                      /* xSync */
    0,                      /* xCommit */
    0,                      /* xRollback */
    0,                      /* xFindMethod */
    0,                      /* xRename */
    0,                      /* xSavepoint */
    0,                      /* xRelease */
    0,                      /* xRollbackTo */
    0                       /* xShadowName */
};

#pragma endregion

#pragma endregion

#pragma region entrypoints
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
    rc = sqlite3_create_function(
        db, "url", -1, SQLITE_UTF8 | SQLITE_INNOCUOUS | SQLITE_DETERMINISTIC, 0,
        urlFunc, 0, 0);
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
    rc = sqlite3_create_function(db, "url_user", 1,
                                 SQLITE_UTF8 | SQLITE_INNOCUOUS |
                                     SQLITE_DETERMINISTIC,
                                 0, urlUserFunc, 0, 0);
  if (rc == SQLITE_OK)
    rc = sqlite3_create_function(db, "url_password", 1,
                                 SQLITE_UTF8 | SQLITE_INNOCUOUS |
                                     SQLITE_DETERMINISTIC,
                                 0, urlPasswordFunc, 0, 0);
  if (rc == SQLITE_OK)
    rc = sqlite3_create_function(db, "url_options", 1,
                                 SQLITE_UTF8 | SQLITE_INNOCUOUS |
                                     SQLITE_DETERMINISTIC,
                                 0, urlOptionsFunc, 0, 0);
  if (rc == SQLITE_OK)
    rc = sqlite3_create_function(db, "url_port", 1,
                                 SQLITE_UTF8 | SQLITE_INNOCUOUS |
                                     SQLITE_DETERMINISTIC,
                                 0, urlPortFunc, 0, 0);
  if (rc == SQLITE_OK)
    rc = sqlite3_create_function(db, "url_zoneid", 1,
                                 SQLITE_UTF8 | SQLITE_INNOCUOUS |
                                     SQLITE_DETERMINISTIC,
                                 0, urlZoneidFunc, 0, 0);
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
  if (rc == SQLITE_OK)
    rc = sqlite3_create_function(db, "url_querystring", -1,
                                 SQLITE_UTF8 | SQLITE_INNOCUOUS |
                                     SQLITE_DETERMINISTIC,
                                 0, urlQuerystringFunc, 0, 0);
  if (rc == SQLITE_OK)
    rc = sqlite3_create_module(db, "url_query_each", &urlQueryEachModule, 0);
  return rc;
}
#pragma endregion