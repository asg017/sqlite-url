import unittest
import sqlite3
import sqlite_url

class TestSqliteUrlPython(unittest.TestCase):
  def test_path(self):
    db = sqlite3.connect(':memory:')
    db.enable_load_extension(True)

    self.assertEqual(type(sqlite_url.loadable_path()), str)
    
    sqlite_url.load(db)
    version, = db.execute('select url_version()').fetchone()
    self.assertEqual(version[0], "v")

if __name__ == '__main__':
    unittest.main()