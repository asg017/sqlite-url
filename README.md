# sqlite-url

A SQLite extension for parsing and generating URLs and query strings. Based on libcurl's [URL API](https://curl.se/libcurl/c/libcurl-url.html)

## Usage

```sql
.load ./url0
select url_scheme('https://www.sqlite.org/vtab.html#usage'); -- 'https'
select url_host('https://www.sqlite.org/vtab.html#usage'); -- 'www.sqlite.org'
select url_path('https://www.sqlite.org/vtab.html#usage'); -- '/vtab.html'
select url_fragment('https://www.sqlite.org/vtab.html#usage'); -- 'usage'

select url(null,
  'scheme', 'https',
  'host', 'alexgarcia.xyz',
  'fragment', 'yeet'
); -- 'https://alexgarcia.xyz/#yeet'
```

Iterate through all parameters in a URL's query string.

```sql

*/
```

Use with sqlite-http to

```sql
select

/*
*/
```

```sql

select
```

## Documentation

See [`docs.md`](./docs.md) for a full API reference.

## Installing

The [Releases page](https://github.com/asg017/sqlite-url/releases) contains pre-built binaries for Linux amd6 and MacOS amd64 (no arm).

### As a loadable extension

If you want to use `sqlite-url` as a [Runtime-loadable extension](https://www.sqlite.org/loadext.html), Download the `url0.dylib` (for MacOS) or `url0.so` (Linux) file from a release and load it into your SQLite environment.

> **Note:**
> The `0` in the filename (`url0.dylib`/ `url0.so`) denotes the major version of `sqlite-url`. Currently `sqlite-url` is pre v1, so expect breaking changes in future versions.

For example, if you are using the [SQLite CLI](https://www.sqlite.org/cli.html), you can load the library like so:

```sql
.load ./url0
select url_version();
-- v0.0.1
```

Or in Python, using the builtin [sqlite3 module](https://docs.python.org/3/library/sqlite3.html):

```python
import sqlite3

con = sqlite3.connect(":memory:")

con.enable_load_extension(True)
con.load_extension("./url0")

print(con.execute("select url_version()").fetchone())
# ('v0.0.1',)
```

Or in Node.js using [better-sqlite3](https://github.com/WiseLibs/better-sqlite3):

```javascript
const Database = require("better-sqlite3");
const db = new Database(":memory:");

db.loadExtension("./url0");

console.log(db.prepare("select url_version()").get());
// { 'html_version()': 'v0.0.1' }
```

Or with [Datasette](https://datasette.io/):

```
datasette data.db --load-extension ./url0
```

## See also

- [sqlite-oath](https://github.com/asg017/sqlite-path), parsing/generating paths (pairs well with `url_path()` and `url()`)
- [sqlite-http](https://github.com/asg017/sqlite-http), for making HTTP requests in SQLite
- [sqlite-html](https://github.com/asg017/sqlite-html), for parsing HTML documents
- [sqlite-lines](https://github.com/asg017/sqlite-lines), for reading large files line-by-line
- [nalgeon/sqlean](https://github.com/nalgeon/sqlean), several pre-compiled handy SQLite functions, in C
