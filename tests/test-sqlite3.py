import unittest
import subprocess 

class Results:
  def __init__(self, stdout, stderr):
    self.stdout = stdout
    self.stderr = stderr

def run_sqlite3(input):
  if type(input) is list:
    args = ["dist/sqlite3", ":memory:"] + input
  else:
    args = ["dist/sqlite3", ":memory:"] + [input]
  
  proc = subprocess.run(args, stdin=subprocess.PIPE, stdout=subprocess.PIPE)
  out = proc.stdout.decode('utf8') if type(proc.stdout) is bytes else None
  err = proc.stderr.decode('utf8') if type(proc.stderr) is bytes else None
  return Results(out, err)

class TestSqliteLinesCli(unittest.TestCase):
  def test_cli_scalar(self):
    self.assertEqual(run_sqlite3('select 1').stdout,  '1\n')
    self.assertEqual(
      run_sqlite3(['select name from pragma_function_list where name like "url%" order by 1']).stdout,  
      "url\nurl_debug\nurl_escape\nurl_fragment\nurl_host\nurl_path\nurl_query\nurl_querystring\nurl_scheme\nurl_unescape\nurl_valid\nurl_version\n"
    )
    self.assertEqual(
      run_sqlite3(['select name from pragma_module_list where name like "url_%" order by 1']).stdout,  
      "url_query_each\n"
    )
    self.assertEqual(
      run_sqlite3(['select * from url_query_each("a=b&x=w%20h+y");']).stdout,  
      "a|b\nx|w h y\n"
    )

if __name__ == '__main__':
    unittest.main()