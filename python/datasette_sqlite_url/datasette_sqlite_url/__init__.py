from datasette import hookimpl
import sqlite_url

from datasette_sqlite_url.version import __version_info__, __version__ 

@hookimpl
def prepare_connection(conn):
    conn.enable_load_extension(True)
    sqlite_url.load(conn)
    conn.enable_load_extension(False)