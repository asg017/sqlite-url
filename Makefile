COMMIT=$(shell git rev-parse HEAD)
VERSION=$(shell cat VERSION)
DATE=$(shell date +'%FT%TZ%z')

LOADABLE_CFLAGS=-fPIC -shared

ifeq ($(shell uname -s),Darwin)
CONFIG_DARWIN=y
else
CONFIG_LINUX=y
endif

ifdef CONFIG_DARWIN
#CURL_FLAGS=-I/usr/local/opt/curl/include -L/usr/local/opt/curl/lib -lcurl
CURL_FLAGS=-Icurl/include/curl -Lcurl/lib/.libs/ -lcurl
LOADABLE_EXTENSION=dylib
endif

ifdef CONFIG_LINUX
CURL_FLAGS=-I/usr/include/x86_64-linux-gnu/curl/ -L/usr/lib/x86_64-linux-gnu/ -llibcurl
LOADABLE_EXTENSION=so
endif
TARGET_LOADABLE=dist/url0.$(LOADABLE_EXTENSION)

DEFINE_SQLITE_URL_DATE=-DSQLITE_URL_DATE="\"$(DATE)\""
DEFINE_SQLITE_URL_VERSION=-DSQLITE_URL_VERSION="\"$(VERSION)\""
DEFINE_SQLITE_URL_SOURCE=-DSQLITE_URL_SOURCE="\"$(COMMIT)\""
DEFINE_SQLITE_URL=$(DEFINE_SQLITE_URL_DATE) $(DEFINE_SQLITE_URL_VERSION) $(DEFINE_SQLITE_URL_SOURCE)

clean:
	rm dist/*

FORMAT_FILES=sqlite-url.h sqlite-url.c core_init.c
format: $(FORMAT_FILES)
	clang-format -i $(FORMAT_FILES)

loadable: $(TARGET_LOADABLE) $(TARGET_LOADABLE_NOFS)

$(TARGET_LOADABLE): sqlite-url.c
	gcc -Isqlite \
	$(LOADABLE_CFLAGS) \
	$(DEFINE_SQLITE_URL) \
	-Icurl/include\
	$< \
	-Lcurl/lib/.libs/ -lcurl \
	-o $@

dist/sqlite3-extra.c: sqlite/sqlite3.c core_init.c
	cat sqlite/sqlite3.c core_init.c > $@

test: 
	make test-format
	make test-loadable

test-format: SHELL:=/bin/bash
test-format:
	diff -u <(cat $(FORMAT_FILES)) <(clang-format $(FORMAT_FILES))

test-loadable: $(TARGET_LOADABLE)
	python3 tests/test-loadable.py

.PHONY: all clean format \
	test test-watch test-loadable-watch test-cli-watch test-sqlite3-watch \
	test-format test-loadable test-cli \
	loadable
