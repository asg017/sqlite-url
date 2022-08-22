# sqlite-path Documentation

A full reference to every function and module that sqlite-url offers.

As a reminder, sqlite-url follows semver and is pre v1, so breaking changes are to be expected.

## API Reference

<h3 name="url_version"><code>url_version()</code></h3>

Gets the version string of the sqlite-url library.

```sql
select url_version(); --> "v0.0.0"
```

<h3 name="url_debug"><code>url_debug()</code></h3>

Get debug info of the sqlite-url library.

```sql

```

<h3 name="url"><code>url(url [, name1, value1], [...])</code></h3>

Generate a URL. The first "url" parameter is a base URL that is parsed, and can be overwritten by the other parameters. If "url" is null or the empty string, Then only the other parameters are used. "name" parameters must be one of the following:

- "scheme":
- "host":
- "path":
- "query":
- "fragment":

```sql

```

<h3 name="url_valid"><code>url_valid(url, [strict])</code></h3>

Returns 1 if url is a well-formed URL according to libcurl, 0 otherwise.

````sql
select url_valid('https://google.com'); -- 1
select url_valid('invalid'); -- 0


<h3 name="url_host"><code>url_host(url)</code></h3>

Returns the host portion of the given URL.

```sql

````

<h3 name="url_scheme"><code>url_scheme(url)</code></h3>

Returns the scheme portion of the given URL.

```sql

```

<h3 name="url_path"><code>url_path(url)</code></h3>

Returns the path portion of the given URL.

```sql

```

<h3 name="url_query"><code>url_query(url)</code></h3>

Returns the query portion of the given URL.

```sql

```

<h3 name="url_fragment"><code>url_fragment(url)</code></h3>

Returns the fragment portion of the given URL.

```sql

```

<h3 name="url_escape"><code>url_escape(url)</code></h3>

Escape the given text.

```sql

```

<h3 name="url_unescape"><code>url_unescape(contents)</code></h3>

Unescape the given text.

```sql

```

<h3 name="url_querystring"><code>url_querystring(name1, value1, [...])</code></h3>

Generate a query string with the given names and values.

Individual segments are automatically escaped.
https://url.spec.whatwg.org/#urlencoded-serializing

```sql
select url_querystring(
  'q', 'memes',
  'tag', 'Bug Fix'
);
-- 'q=memes&tag=Bug%20Fix'

select url_querystring(
  'q', 'memes',
  'filters', '[{"type":"videos"}]',
  'after', '2022-08-22T16:14:29.077Z'
);
-- 'q=memes&filters=%5B%7B%22type%22%3A%22videos%22%7D%5D&after=2022-08-22T16%3A14%3A29.077Z'
```

<h3 name="url_query_each"><code>select * from url_query_each(query)</code></h3>

Table function that returns each sequence in the given
query string. Every sequence in the query string returns
its own row, and every name and value are pre-escaped.
https://url.spec.whatwg.org/#urlencoded-parsing

```sql

select *
from url_query_each('q=memes&filters=%5B%7B%22type%22%3A%22videos%22%7D%5D&after=2022-08-22T16%3A14%3A29.077Z');
/*
┌─────────┬──────────────────────────┐
│  name   │          value           │
├─────────┼──────────────────────────┤
│ q       │ memes                    │
│ filters │ [{"type":"videos"}]      │
│ after   │ 2022-08-22T16:14:29.077Z │
└─────────┴──────────────────────────┘
*/

-- use url_query() to extract the query string from a full URL.
select *
from url_query_each(
  url_query('https://www.google.com/search?q=memes&rlz=1C5CHFA_enUS915US915&oq=memes&aqs=chrome..69i57j0i433i512j0i131i433i512j0i512j46i433i512j0i433i512j0i512l2j0i131i433i512j0i512.817j0j7&sourceid=chrome&ie=UTF-8')
);

/*
┌──────────┬──────────────────────────────────────────────────────────────┐
│   name   │                            value                             │
├──────────┼──────────────────────────────────────────────────────────────┤
│ q        │ memes                                                        │
├──────────┼──────────────────────────────────────────────────────────────┤
│ rlz      │ 1C5CHFA_enUS915US915                                         │
├──────────┼──────────────────────────────────────────────────────────────┤
│ oq       │ memes                                                        │
├──────────┼──────────────────────────────────────────────────────────────┤
│ aqs      │ chrome..69i57j0i433i512j0i131i433i512j0i512j46i433i512j0i433 │
│          │ i512j0i512l2j0i131i433i512j0i512.817j0j7                     │
├──────────┼──────────────────────────────────────────────────────────────┤
│ sourceid │ chrome                                                       │
├──────────┼──────────────────────────────────────────────────────────────┤
│ ie       │ UTF-8                                                        │
└──────────┴──────────────────────────────────────────────────────────────┘
*/

```
