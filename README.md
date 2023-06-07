# sqlite-url

A SQLite extension for parsing and generating URLs and query strings. Based on libcurl's [URL API](https://curl.se/libcurl/c/libcurl-url.html)

Try it out in your browser and learn more in [Introducing sqlite-url: A SQLite extension for parsing and generating URLs](https://observablehq.com/@asg017/introducing-sqlite-url) (September 2022)

## Usage

```sql
.load ./url0
select url_valid('https://sqlite.org'); -- 1
```

Extract specific parts from a given URL.

```sql
select url_scheme('https://www.sqlite.org/vtab.html#usage'); -- 'https'
select url_host('https://www.sqlite.org/vtab.html#usage'); -- 'www.sqlite.org'
select url_path('https://www.sqlite.org/vtab.html#usage'); -- '/vtab.html'
select url_fragment('https://www.sqlite.org/vtab.html#usage'); -- 'usage'
```

Generate a URL programmatically.

```sql
select url(null,
  'scheme', 'https',
  'host', 'alexgarcia.url',
  'fragment', 'yeet'
); -- 'https://alexgarcia.url/#yeet'
```

Iterate through all parameters in a URL's query string.

```sql

select *
from url_query_each(
  url_query('https://api.census.gov/data/2020/acs/acs5?get=B01001_001E&for=county:*&in=state:06')
);
/*
┌──────┬─────────────┐
│ name │    value    │
├──────┼─────────────┤
│ get  │ B01001_001E │
│ for  │ county:*    │
│ in   │ state:06    │
└──────┴─────────────┘
*/
```

Use with [sqlite-http](https://github.com/asg017/sqlite-http) to generate URLs to request.

```sql
select http_get_body(
  url(
    'https://api.census.gov',
    'path', '/data/2020/acs/acs5',
    'query', url_querystring(
      'get', 'B01001_001E',
      'for', 'county:*',
      'in', 'state:06'
    )
  )
);
/*
┌────────────────────────────────────┐
│           http_get_body(           │
├────────────────────────────────────┤
│ [["B01001_001E","state","county"], │
│ ["1661584","06","001"],            │
│ ["1159","06","003"],               │
│ ["223344","06","007"],             │
│ ["21491","06","011"],              │
│ ["1147788","06","013"],            │
│ ["190345","06","017"],             │
│               ...                  │
│ ["845599","06","111"]]             │
└────────────────────────────────────┘
*/
```

Use with [sqlite-path](https://github.com/asg017/sqlite-path) to safely generate paths for a URL.

```sql

select url(
  'https://github.com',
  'path', path_join('/', 'asg017', 'sqlite-url', 'issues', '1')
);
-- 'https://github.com/asg017/sqlite-url/issues/1'
```

## Documentation

See [`docs.md`](./docs.md) for a full API reference.

## Installing

| Language       | Install                                                    |                                                                                                                                                                                           |
| -------------- | ---------------------------------------------------------- | ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| Python         | `pip install sqlite-url`                                   | [![PyPI](https://img.shields.io/pypi/v/sqlite-url.svg?color=blue&logo=python&logoColor=white)](https://pypi.org/project/sqlite-url/)                                                      |
| Datasette      | `datasette install datasette-sqlite-url`                   | [![Datasette](https://img.shields.io/pypi/v/datasette-sqlite-url.svg?color=B6B6D9&label=Datasette+plugin&logoColor=white&logo=python)](https://datasette.io/plugins/datasette-sqlite-url) |
| Node.js        | `npm install sqlite-url`                                   | [![npm](https://img.shields.io/npm/v/sqlite-url.svg?color=green&logo=nodedotjs&logoColor=white)](https://www.npmjs.com/package/sqlite-url)                                                |
| Deno           | [`deno.land/x/sqlite_url`](https://deno.land/x/sqlite_url) | [![deno.land/x release](https://img.shields.io/github/v/release/asg017/sqlite-url?color=fef8d2&include_prereleases&label=deno.land%2Fx&logo=deno)](https://deno.land/x/sqlite_url)        |
| Ruby           | `gem install sqlite-url`                                   | ![Gem](https://img.shields.io/gem/v/sqlite-url?color=red&logo=rubygems&logoColor=white)                                                                                                   |
| Github Release |                                                            | ![GitHub tag (latest SemVer pre-release)](https://img.shields.io/github/v/tag/asg017/sqlite-url?color=lightgrey&include_prereleases&label=Github+release&logo=github)                     |

<!--
| Elixir         | [`hex.pm/packages/sqlite_url`](https://hex.pm/packages/sqlite_url) | [![Hex.pm](https://img.shields.io/hexpm/v/sqlite_url?color=purple&logo=elixir)](https://hex.pm/packages/sqlite_url)                                                                       |
| Go             | `go get -u github.com/asg017/sqlite-url/bindings/go`               | [![Go Reference](https://pkg.go.dev/badge/github.com/asg017/sqlite-url/bindings/go.svg)](https://pkg.go.dev/github.com/asg017/sqlite-url/bindings/go)                                     |
| Rust           | `cargo add sqlite-url`                                             | [![Crates.io](https://img.shields.io/crates/v/sqlite-url?logo=rust)](https://crates.io/crates/sqlite-url)                                                                                 |
-->

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

- [sqlite-path](https://github.com/asg017/sqlite-path), parsing/generating paths (pairs well with `url_path()` and `url()`)
- [sqlite-http](https://github.com/asg017/sqlite-http), for making HTTP requests in SQLite
- [sqlite-html](https://github.com/asg017/sqlite-html), for parsing HTML documents
- [sqlite-lines](https://github.com/asg017/sqlite-lines), for reading large files line-by-line
- [nalgeon/sqlean](https://github.com/nalgeon/sqlean), several pre-compiled handy SQLite functions, in C
