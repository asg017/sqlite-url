use sqlite_loadable::prelude::*;
use sqlite_loadable::{api, define_scalar_function, Error, FunctionFlags, Result};

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
    let url = url::Url::parse(api::value_text(values.get(0).unwrap()).unwrap())
        .map_err(|e| Error::new_message(format!("Error parsing URL: {}", e)))?;
    match url.host_str() {
        Some(host) => api::result_text(context, host)?,
        None => api::result_null(context),
    };
    Ok(())
}

#[sqlite_entrypoint]
pub fn sqlite3_url_init(db: *mut sqlite3) -> Result<()> {
    let flags = FunctionFlags::UTF8 | FunctionFlags::DETERMINISTIC;
    define_scalar_function(db, "url_version", 0, url_version, flags)?;
    define_scalar_function(db, "url_debug", 0, url_debug, flags)?;
    define_scalar_function(db, "url_host", 1, url_host, flags)?;
    Ok(())
}
