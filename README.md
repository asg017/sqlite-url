## TODO

```
cd curl
autoreconf -fi
./configure  --without-brotli --without-libpsl --without-nghttp2 --without-ngtcp2 --without-zstd --without-libidn2 --without-librtmp --without-ssl --without-zlib
make
```

- [ ] how statically include CURL
- [ ] sqlite3 target
- [ ] wasm target
- [ ] CURLUPART_USER, CURLUPART_PASSWORD, CURLUPART_OPTIONS, CURLUPART_PORT
- [ ] GH actions
  - [ ] test on platforms
  - [ ] release script
- [ ] README cleanup

- [ ] `select name, value from url_query_each(querystring)` https://url.spec.whatwg.org/#urlencoded-parsing

```sql
select name, value
from url_query_each(
  url_query('https://api.census.gov/data/2020/acs/acs5?get=B01001_001E&for=county:*&in=state:06')
);
/*
get|B01001_001E
for|county:*
in|state:06
*/
```

- [ ] `url_querystring(name1, value1, [...])`

- [ ] `url(url, name1, value1, [...])`

```sql
select url(
  "https://github.com",
  'path', '/asg017/sqlite-path',
  'fragment', 'path_segment_at'
);
```

apt-get update && apt-get install make libcurl4-openssl-dev gcc

gcc -Isqlite \
-fPIC -shared \
-DSQLITE_URL_DATE="\"2022-08-20T05:54:46Z+0000\"" -DSQLITE_URL_VERSION="\"v0.0.0\"" -DSQLITE_URL_SOURCE="\"\"" \
-I/usr/include/x86_64-linux-gnu/curl/ -L/usr/lib/x86_64-linux-gnu/ -lcurl \
sqlite-url.c -o dist/url0.so
