#include "test_helper.h"
#include "oscit/values.h"

class ListValueTest : public TestHelper
{  
public:
  void test_create( void ) {
    Value v(TypeTag("H"));
    Value res;
    
    assert_false(v.is_nil());
    assert_false(v.is_real());
    assert_false(v.is_string());
    assert_false(v.is_list());
    assert_false(v.is_error());
    assert_true(v.is_hash());
    
    // 0
    assert_false(v.get("foo", &res));
    v.set("foo",3.5);
    assert_true(v.get("foo", &res));
    assert_true(v.is_real());
    assert_equal(3.5, res.r);
  }
  
  void test_create_set( void ) {
    HashValue v;
    
    assert_true(v.is_hash());
    
    assert_equal("H", v.type_tag());
  }
  
  void test_copy( void ) {
    Value v(TypeTag("H"));
    v.set(1,"one");
    v.set(2,"two");
    
    Value v2(v);
    Value v3;
    
    Value res;
    
    assert_true(v2.is_hash());
    assert_true(v2.get(1, &res));
    assert_equal("one", res.s);
    
    v.set("1","un");
    v.set("2","deux");
    assert_true(v.get("1", &res));
    assert_equal("un", res.s);
    
    // change in v did not change v2
    assert_true(v2.get("1", &res));
    assert_equal("one", res.s);
    
    assert_true(v3.is_nil());
    
    v3 = v;
    
    assert_true(v3.is_hash());
    
    assert_true(v3.get("1", &res));
    assert_equal("un", res.s);
    assert_true(v3.get("2", &res));
    assert_equal("deux", res.s);
    
    v.set("1", "uno");
    
    // change in v did not change v3
    assert_true(v3.get("1", &res));
    assert_equal("un", res.s);
    
  }
  
  void test_set( void ) {
    Value v;
    
    assert_true(v.is_nil());
    
    v.set("nice", "friends");
    
    assert_true(v.is_hash());
  }
  
  void test_set_tag( void ) {
    Value v;
    
    v.set_type_tag("H");
    assert_true(v.is_hash());
  }
  
  void test_set_type( void ) {
    Value v;
    
    v.set_type(HASH_VALUE);
    assert_true(v.is_hash());
  }
  
  void test_key_iterator( void ) {
    Value v;
    Value res;
    
    HashIterator it  = v.begin();
    HashIterator end = v.end();
    
    assert_true(it == end);
    
    v.set("a", 1);
    v.set("c", 3);
    v.set("b", 2);
    
    it  = v.begin();
    end = v.end();
    
    while( it != end ) {
      std::string key(*it++);
      assert_true(v.get(key, &res));
      assert_true(res.is_real());
      if (key == "a") {
        assert_equal(1, res.r);
      } else if (key == "b") {
        assert_equal(2, res.r);
      } else if (key == "c") {
        assert_equal(3, res.r);
      } else {
        assert_equal("wrong key !", key);
      }
    }
  }
  
  void test_read( void ) {
    Value v(TypeTag("H"));
    Hash &l = *v.hash_;
    Value res;
    
    assert_false(v.get("one", &res));
    
    l.set("one", Value(1.0));
    
    assert_true(v.get("one", &res));
    assert_equal(1.0, res.r);
  }
  
  void test_set_real( void ) {
    Value v;
    v.set("one", 1.34);
    assert_true(v.is_hash());
    assert_equal(1.34, v["one"].r); // this is a dangerous syntax: v["real"] can return gNilValue...
    
    v.set("two", 3.45);
    assert_true(v["two"].is_real());
  }
  
  void test_set_string( void ) {
    Value v(1.0);
    v.set("one", "first");
    assert_true(v.is_hash());
    assert_true(v["one"].is_string());
    assert_equal("first", v["one"].s);
  }
  
  void test_set_list( void ) {
    Value v;
    Value l;
    l.push_back("one").push_back(2.0);
    v.set("list", l);
    assert_true(v.is_hash());
    
    assert_true(v["list"].is_list());
    assert_equal(2, v["list"].size());
    
    assert_true(v["list"][0].is_string());
    assert_equal("one", v["list"][0].s);
    
    assert_true(v["list"][1].is_real());
    assert_equal(2, v["list"][1].r);
  }
  
  void test_stream( void ) {
    Value v;
    Value jobs;
    v.set("name", "Joe");
    v.set("age", 34);
    jobs.push_back("dad").push_back("husband").push_back("lover").push_back(-666);
    v.set("job", jobs);
    
    std::ostringstream os(std::ostringstream::out);
    os << v;
    assert_equal("...", os.str());
  }
};
