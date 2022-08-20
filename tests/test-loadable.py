import sqlite3
import unittest

EXT_PATH="./dist/url0"

def connect(ext):
  db = sqlite3.connect(":memory:")

  db.execute("create table base_functions as select name from pragma_function_list")
  db.execute("create table base_modules as select name from pragma_module_list")

  db.enable_load_extension(True)
  db.load_extension(ext)

  db.execute("create temp table loaded_functions as select name from pragma_function_list where name not in (select name from base_functions) order by name")
  db.execute("create temp table loaded_modules as select name from pragma_module_list where name not in (select name from base_modules) order by name")

  db.row_factory = sqlite3.Row
  return db


db = connect(EXT_PATH)

def explain_query_plan(sql):
  return db.execute("explain query plan " + sql).fetchone()["detail"]

def execute_all(sql, args=None):
  if args is None: args = []
  results = db.execute(sql, args).fetchall()
  return list(map(lambda x: dict(x), results))

def spread_args(args):
  return ",".join(['?'] * len(args))

FUNCTIONS = [
  "url_debug",
  "url_escape",
  "url_fragment",
  "url_host",
  "url_path",
  "url_query",
  "url_scheme",
  "url_unescape",
  "url_valid",
  "url_version",
]

MODULES = []

TEST_URL = 'https://api.github.com/repos/uscensusbureau/citysdk?sort=asc#ayoo'

class TestUrl(unittest.TestCase):
  def test_funcs(self):
    funcs = list(map(lambda a: a[0], db.execute("select name from loaded_functions").fetchall()))
    self.assertEqual(funcs, FUNCTIONS)

  def test_modules(self):
    modules = list(map(lambda a: a[0], db.execute("select name from loaded_modules").fetchall()))
    self.assertEqual(modules, MODULES)

    
  def test_url_version(self):
    with open("./VERSION") as f:
      version = f.read()
    
    self.assertEqual(db.execute("select url_version()").fetchone()[0], version)

  def test_url_debug(self):
    debug = db.execute("select url_debug()").fetchone()[0].split('\n')
    self.assertEqual(len(debug), 4)

    self.assertTrue(debug[0].startswith("Version: v"))
    self.assertTrue(debug[1].startswith("Date: "))
    self.assertTrue(debug[2].startswith("Source: "))
    self.assertTrue(debug[3].startswith("libcurl"))
  
  def test_url_valid(self):
    url_valid = lambda arg: db.execute("select url_valid(?)", [arg]).fetchone()[0]
    self.assertEqual(url_valid("https://t.me"), 1)
    self.assertEqual(url_valid("not"), 0)
    self.assertEqual(url_valid("wss://kasldjf.c"), 1)
    self.assertEqual(url_valid("http://a"), 1)
  
  def test_url_escape(self):
    url_escape = lambda arg: db.execute("select url_escape(?)", [arg]).fetchone()[0]
    self.assertEqual(url_escape("alex garcia, &="), "alex%20garcia%2C%20%26%3D")
  
  def test_url_unescape(self):
    url_unescape = lambda arg: db.execute("select url_unescape(?)", [arg]).fetchone()[0]
    self.assertEqual(url_unescape("alex%20garcia%2C%20%26%3D"), "alex garcia, &=")
  
  def test_url_scheme(self):
    url_scheme = lambda arg: db.execute("select url_scheme(?)", [arg]).fetchone()[0]
    self.assertEqual(url_scheme(TEST_URL), "https")
  
  def test_url_host(self):
    url_host = lambda arg: db.execute("select url_host(?)", [arg]).fetchone()[0]
    self.assertEqual(url_host(TEST_URL), "api.github.com")
  
  def test_url_path(self):
    url_path = lambda arg: db.execute("select url_path(?)", [arg]).fetchone()[0]
    self.assertEqual(url_path(TEST_URL), "/repos/uscensusbureau/citysdk")
  
  def test_url_query(self):
    url_query = lambda arg: db.execute("select url_query(?)", [arg]).fetchone()[0]
    self.assertEqual(url_query(TEST_URL), "sort=asc")
  
  def test_url_fragment(self):
    url_fragment = lambda arg: db.execute("select url_fragment(?)", [arg]).fetchone()[0]
    self.assertEqual(url_fragment(TEST_URL), "ayoo")
  
class TestCoverage(unittest.TestCase):                                      
  def test_coverage(self):                                                      
    test_methods = [method for method in dir(TestUrl) if method.startswith('test_url')]
    funcs_with_tests = set([x.replace("test_", "") for x in test_methods])
    for func in FUNCTIONS:
      self.assertTrue(func in funcs_with_tests, f"{func} does not have cooresponding test in {funcs_with_tests}")

if __name__ == '__main__':
    unittest.main()