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
LOAD_FLAGS=-lldap -framework CoreFoundation -framework SystemConfiguration
LOADABLE_EXTENSION=dylib
endif

ifdef CONFIG_LINUX
LOAD_FLAGS=
LOADABLE_EXTENSION=so
endif

DEFINE_SQLITE_URL_DATE=-DSQLITE_URL_DATE="\"$(DATE)\""
DEFINE_SQLITE_URL_VERSION=-DSQLITE_URL_VERSION="\"$(VERSION)\""
DEFINE_SQLITE_URL_SOURCE=-DSQLITE_URL_SOURCE="\"$(COMMIT)\""
DEFINE_SQLITE_URL=$(DEFINE_SQLITE_URL_DATE) $(DEFINE_SQLITE_URL_VERSION) $(DEFINE_SQLITE_URL_SOURCE)

prefix=dist

TARGET_LOADABLE=dist/url0.$(LOADABLE_EXTENSION)
TARGET_SQLITE3_EXTRA_C=$(prefix)/sqlite3-extra.c
TARGET_SQLITE3=$(prefix)/sqlite3

clean:
	rm dist/*

$(prefix):
	mkdir -p $(prefix)

FORMAT_FILES=sqlite-url.h sqlite-url.c core_init.c
format: $(FORMAT_FILES)
	clang-format -i $(FORMAT_FILES)

loadable: $(TARGET_LOADABLE) $(TARGET_LOADABLE_NOFS)
sqlite3: $(TARGET_SQLITE3)


$(TARGET_LOADABLE): sqlite-url.c
	gcc -Isqlite -I. \
	$(LOADABLE_CFLAGS) \
	$(DEFINE_SQLITE_URL) \
	-Icurl/include \
	$< \
	curl/lib/.libs/libcurl.a $(LOAD_FLAGS) \
	-o $@

$(TARGET_SQLITE3_EXTRA_C): sqlite/sqlite3.c core_init.c
	cat sqlite/sqlite3.c core_init.c > $@

$(TARGET_SQLITE3): $(prefix) $(TARGET_SQLITE3_EXTRA_C) sqlite/shell.c sqlite-url.c
	gcc \
	$(DEFINE_SQLITE_URL) \
	-DSQLITE_THREADSAFE=0 -DSQLITE_OMIT_LOAD_EXTENSION=1 \
	-DSQLITE_EXTRA_INIT=core_init \
	-I./ -I./sqlite -Icurl/include \
	$(TARGET_SQLITE3_EXTRA_C) sqlite/shell.c sqlite-url.c curl/lib/.libs/libcurl.a $(LOAD_FLAGS) \
	-o $@

test: 
	make test-format
	make test-loadable
	make test-sqlite3

test-format: SHELL:=/bin/bash
test-format:
	diff -u <(cat $(FORMAT_FILES)) <(clang-format $(FORMAT_FILES))

test-loadable: $(TARGET_LOADABLE)
	python3 tests/test-loadable.py

test-sqlite3: $(TARGET_SQLITE3)
	python3 tests/test-sqlite3.py

test-sqlite3-watch: $(TARAGET_SQLITE3)
	watchexec -w $(TARAGET_SQLITE3) -w tests/test-sqlite3.py --clear -- make test-sqlite3

.PHONY: all clean format \
	test test-watch test-loadable-watch test-cli-watch test-sqlite3-watch \
	test-format test-loadable test-cli \
	loadable