/*
  This file is appended to the end of a sqlite3.c amalgammation
  file to include sqlite3_url functions/tables statically in
  a build. This is used for the demo CLI and WASM implementations.
*/
#include "sqlite-url.h"
int core_init(const char *dummy) {
  return sqlite3_auto_extension((void *)sqlite3_url_init);
}