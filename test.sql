.bail on
.load target/debug/libsqlite_url sqlite3_url_init

.mode qbox

select url_version(), url_debug();

.param set :test 'https://docs.rs/url/latest/url/'
select 
  url_valid(:test),
  url_scheme(:test),
  url_host(:test),
  url_path(:test),
  url_query(:test),
  url_fragment(:test),
  url_port(:test),
  url_escape('alex garcia');
