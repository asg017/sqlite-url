## TODO

- [ ] how statically include CURL
- [ ] sqlite3 target
- [ ] wasm target
- [ ] CURLUPART_USER, CURLUPART_PASSWORD, CURLUPART_OPTIONS, CURLUPART_PORT
- [ ] GH actions
  - [ ] test on platforms
  - [ ] release script
- [ ] README cleanup

- [ ] `url_query_each(querystring)` https://url.spec.whatwg.org/#urlencoded-parsing

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
