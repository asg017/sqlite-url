.bail on
.load target/debug/libsqlite_url sqlite3_url_init

select url_version();

select url_host("https://docs.rs/url/latest/url/");
