#include "test_helper.h"
#include "oscit/root.h"
#include "oscit/file_method.h"

#include <sstream>
#include <fstream>    // file io

#define FILE_METHOD_PATH "simple_view.json"

class FileMethodTest : public TestHelper
{
public:
  FileMethodTest() {
    preserve(fixture_path(FILE_METHOD_PATH));
  }

  void tearDown() {
    restore(fixture_path(FILE_METHOD_PATH));
  }

  void test_read( void ) {
    Root root(false);
    root.adopt(new FileMethod("simple_view", fixture_path(FILE_METHOD_PATH), std::string("Basic synth view.")));
    Value res = root.call("/simple_view");
    assert_true(res.is_string())
    assert_equal("{\n  \"x\":0, \"y\":0, \"width\":500, \"height\":", res.str().substr(0, 40));
  }

  void test_write( void ) {
    Root root(false);
    root.adopt(new FileMethod("simple_view", fixture_path(FILE_METHOD_PATH), std::string("Basic synth view.")));
    Value res = root.call("/simple_view", Value("Yoba"));
    assert_true(res.is_string());
    assert_equal("Yoba", res.str());

    res = root.call("/simple_view");
    assert_true(res.is_string());
    assert_equal("Yoba", res.str());

    std::ifstream in(fixture_path(FILE_METHOD_PATH).c_str(), std::ios::in);
      std::ostringstream oss;
      oss << in.rdbuf();
    in.close();
    assert_equal("Yoba", oss.str());
  }
};

