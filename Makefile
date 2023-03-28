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

ifdef python
PYTHON=$(python)
else
PYTHON=python3
endif

DEFINE_SQLITE_URL_DATE=-DSQLITE_URL_DATE="\"$(DATE)\""
DEFINE_SQLITE_URL_VERSION=-DSQLITE_URL_VERSION="\"v$(VERSION)\""
DEFINE_SQLITE_URL_SOURCE=-DSQLITE_URL_SOURCE="\"$(COMMIT)\""
DEFINE_SQLITE_URL=$(DEFINE_SQLITE_URL_DATE) $(DEFINE_SQLITE_URL_VERSION) $(DEFINE_SQLITE_URL_SOURCE)

prefix=dist
TARGET_LOADABLE=$(prefix)/url0.$(LOADABLE_EXTENSION)
TARGET_WHEELS=$(prefix)/wheels

python: $(TARGET_WHEELS) $(TARGET_LOADABLE) $(TARGET_WHEELS) scripts/rename-wheels.py $(shell find python/sqlite_url -type f -name '*.py')
	cp $(TARGET_LOADABLE) $(INTERMEDIATE_PYPACKAGE_EXTENSION)
	rm $(TARGET_WHEELS)/sqlite_url* || true
	pip3 wheel python/sqlite_url/ -w $(TARGET_WHEELS)
	python3 scripts/rename-wheels.py $(TARGET_WHEELS) $(RENAME_WHEELS_ARGS)
	echo "✅ generated python wheel"

python-versions: python/version.py.tmpl
	VERSION=$(VERSION) envsubst < python/version.py.tmpl > python/sqlite_url/sqlite_url/version.py
	echo "✅ generated python/sqlite_url/sqlite_url/version.py"

	VERSION=$(VERSION) envsubst < python/version.py.tmpl > python/datasette_sqlite_url/datasette_sqlite_url/version.py
	echo "✅ generated python/datasette_sqlite_url/datasette_sqlite_url/version.py"

datasette: $(TARGET_WHEELS) $(shell find python/datasette_sqlite_url -type f -name '*.py')
	rm $(TARGET_WHEELS)/datasette* || true
	pip3 wheel python/datasette_sqlite_url/ --no-deps -w $(TARGET_WHEELS)

npm: VERSION npm/platform-package.README.md.tmpl npm/platform-package.package.json.tmpl npm/sqlite-url/package.json.tmpl scripts/npm_generate_platform_packages.sh
	scripts/npm_generate_platform_packages.sh

deno: VERSION deno/deno.json.tmpl
	scripts/deno_generate_package.sh

version:
	make python-versions
	make python
	make npm
	make deno


TARGET_SQLITE3_EXTRA_C=$(prefix)/sqlite3-extra.c
TARGET_SQLITE3=$(prefix)/sqlite3

INTERMEDIATE_PYPACKAGE_EXTENSION=python/sqlite_url/sqlite_url/url0.$(LOADABLE_EXTENSION)

clean:
	rm dist/*

$(prefix):
	mkdir -p $(prefix)

$(TARGET_WHEELS): $(prefix)
	mkdir -p $(TARGET_WHEELS)

FORMAT_FILES=sqlite-url.h sqlite-url.c core_init.c
format: $(FORMAT_FILES)
	clang-format -i $(FORMAT_FILES)

loadable: $(TARGET_LOADABLE) $(TARGET_LOADABLE_NOFS)
sqlite3: $(TARGET_SQLITE3)


$(TARGET_LOADABLE): sqlite-url.c $(prefix)
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
	make test-python
	make test-npm
	make test-deno
	make test-sqlite3

test-format: SHELL:=/bin/bash
test-format:
	diff -u <(cat $(FORMAT_FILES)) <(clang-format $(FORMAT_FILES))

test-loadable: $(TARGET_LOADABLE)
	$(PYTHON) tests/test-loadable.py

test-python:
	$(PYTHON) tests/test-python.py

test-npm:
	node npm/sqlite-url/test.js

test-deno:
	deno task --config deno/deno.json test

test-loadable-watch: $(TARGET_LOADABLE)
	watchexec -w sqlite-url.c -w $(TARGET_LOADABLE) -w tests/test-loadable.py --clear -- make test-loadable

test-sqlite3: $(TARGET_SQLITE3)
	python3 tests/test-sqlite3.py

test-sqlite3-watch: $(TARAGET_SQLITE3)
	watchexec -w $(TARAGET_SQLITE3) -w tests/test-sqlite3.py --clear -- make test-sqlite3

.PHONY: all clean format \
	python python-versions datasette npm deno version \
	test test-watch test-loadable-watch test-cli-watch test-sqlite3-watch \
	test-format test-loadable test-cli \
	loadable