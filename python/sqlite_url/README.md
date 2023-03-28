# The `sqlite-url` Python package

`sqlite-url` is also distributed on PyPi as a Python package, for use in Python applications. It works well with the builtin [`sqlite3`](https://docs.python.org/3/library/sqlite3.html) Python module.

```
pip install sqlite-url
```

## Usage

The `sqlite-url` python package exports two functions: `loadable_url()`, which returns the full path to the loadable extension, and `load(conn)`, which loads the `sqlite-url` extension into the given [sqlite3 Connection object](https://docs.python.org/3/library/sqlite3.html#connection-objects).

```python
import sqlite_url
print(sqlite_url.loadable_url())
# '/.../venv/lib/python3.9/site-packages/sqlite_url/url0'

import sqlite3
conn = sqlite3.connect(':memory:')
sqlite_url.load(conn)
conn.execute('select url_version()').fetchone()
# ('v0.1.0')
```

See [the full API Reference](#api-reference) for the Python API, and [`docs.md`](../../docs.md) for documentation on the `sqlite-url` SQL API.

See [`datasette-sqlite-url`](../datasette_sqlite_url/) for a Datasette plugin that is a light wrapper around the `sqlite-url` Python package.

## Compatibility

Currently the `sqlite-url` Python package is only distributed on PyPi as pre-build wheels, it's not possible to install from the source distribution. This is because the underlying `sqlite-url` extension requires a lot of build dependencies like `make`, `cc`, and `cargo`.

If you get a `unsupported platform` error when pip installing `sqlite-url`, you'll have to build the `sqlite-url` manually and load in the dynamic library manually.

## API Reference

<h3 name="loadable_url"><code>loadable_url()</code></h3>

Returns the full path to the locally-install `sqlite-url` extension, without the filename.

This can be directly passed to [`sqlite3.Connection.load_extension()`](https://docs.python.org/3/library/sqlite3.html#sqlite3.Connection.load_extension), but the [`sqlite_url.load()`](#load) function is preferred.

```python
import sqlite_url
print(sqlite_url.loadable_url())
# '/.../venv/lib/python3.9/site-packages/sqlite_url/url0'
```

> Note: this extension path doesn't include the file extension (`.dylib`, `.so`, `.dll`). This is because [SQLite will infer the correct extension](https://www.sqlite.org/loadext.html#loading_an_extension).

<h3 name="load"><code>load(connection)</code></h3>

Loads the `sqlite-url` extension on the given [`sqlite3.Connection`](https://docs.python.org/3/library/sqlite3.html#sqlite3.Connection) object, calling [`Connection.load_extension()`](https://docs.python.org/3/library/sqlite3.html#sqlite3.Connection.load_extension).

```python
import sqlite_url
import sqlite3
conn = sqlite3.connect(':memory:')

conn.enable_load_extension(True)
sqlite_url.load(conn)
conn.enable_load_extension(False)

conn.execute('select url_version()').fetchone()
# ('v0.1.0')
```
