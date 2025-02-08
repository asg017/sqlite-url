use sqlite_loadable::prelude::*;
use sqlite_loadable::{api, define_scalar_function, Error, FunctionFlags, Result};
use url::Url;

pub fn url_version(context: *mut sqlite3_context, _values: &[*mut sqlite3_value]) -> Result<()> {
    api::result_text(context, format!("xv{}", env!("CARGO_PKG_VERSION")))?;
    Ok(())
}

pub fn url_debug(context: *mut sqlite3_context, _values: &[*mut sqlite3_value]) -> Result<()> {
    api::result_text(
        context,
        format!(
            "Version: v{}
Source: {}
",
            env!("CARGO_PKG_VERSION"),
            env!("GIT_HASH")
        ),
    )?;
    Ok(())
}
pub fn url_host(context: *mut sqlite3_context, values: &[*mut sqlite3_value]) -> Result<()> {
    match url_from_value(&values[0])?.host_str() {
        Some(host) => api::result_text(context, host)?,
        None => api::result_null(context),
    };
    Ok(())
}

fn url_from_value(value: &*mut sqlite3_value)-> Result<Url> {
  let contents = api::value_text(value)?;
  Url::parse(contents).map_err(|e| Error::new_message(format!("Error parsing URL: {}", e)))
}

pub fn url(context: *mut sqlite3_context, _values: &[*mut sqlite3_value]) -> Result<()> {
  todo!();
}
pub fn url_valid(context: *mut sqlite3_context, values: &[*mut sqlite3_value]) -> Result<()> {
  api::result_bool(context, Url::parse(api::value_text(&values[0])?).is_ok());
  Ok(())
}

pub fn url_scheme(context: *mut sqlite3_context, values: &[*mut sqlite3_value]) -> Result<()> {
  api::result_text(context, url_from_value(&values[0])?.scheme())?;
  Ok(())
}

pub fn url_path(context: *mut sqlite3_context, values: &[*mut sqlite3_value]) -> Result<()> {
  api::result_text(context, url_from_value(&values[0])?.path())?;
  Ok(())
}
pub fn url_query(context: *mut sqlite3_context, values: &[*mut sqlite3_value]) -> Result<()> {
  match url_from_value(&values[0])?.query() {
    Some(query) => api::result_text(context, query)?,
    None => api::result_null(context)
  }
  Ok(())
}
pub fn url_fragment(context: *mut sqlite3_context, values: &[*mut sqlite3_value]) -> Result<()> {
  match url_from_value(&values[0])?.fragment() {
    Some(fragment) => api::result_text(context, fragment)?,
    None => api::result_null(context)
  }
  Ok(())
}
pub fn url_port(context: *mut sqlite3_context, values: &[*mut sqlite3_value]) -> Result<()> {
  match url_from_value(&values[0])?.port() {
    Some(port) => api::result_int(context, port.into()),
    None => api::result_null(context)
  }
  Ok(())
}
pub fn url_escape(context: *mut sqlite3_context, values: &[*mut sqlite3_value]) -> Result<()> {
  let encoded: String = url::form_urlencoded::byte_serialize(api::value_blob(&values[0])).collect();
  api::result_text(context, encoded)?;
  Ok(())
}
pub fn url_unescape(context: *mut sqlite3_context, values: &[*mut sqlite3_value]) -> Result<()> {
  todo!();
}
pub fn url_querystring(context: *mut sqlite3_context, _values: &[*mut sqlite3_value]) -> Result<()> {
  todo!();
}

// TODO url_query_each

#[sqlite_entrypoint]
pub fn sqlite3_url_init(db: *mut sqlite3) -> Result<()> {
    let flags = FunctionFlags::UTF8 | FunctionFlags::DETERMINISTIC;
    define_scalar_function(db, "url_version", 0, url_version, flags)?;
    define_scalar_function(db, "url_debug", 0, url_debug, flags)?;

    define_scalar_function(db, "url_scheme", 1, url_scheme, flags)?;
    define_scalar_function(db, "url_host", 1, url_host, flags)?;
    define_scalar_function(db, "url_valid", 1, url_valid, flags)?;
    define_scalar_function(db, "url_path", 1, url_path, flags)?;
    define_scalar_function(db, "url_query", 1, url_query, flags)?;
    define_scalar_function(db, "url_fragment", 1, url_fragment, flags)?;
    define_scalar_function(db, "url_port", 1, url_port, flags)?;
    define_scalar_function(db, "url_escape", 1, url_escape, flags)?;
    Ok(())
}
