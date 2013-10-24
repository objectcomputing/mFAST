#include "test3.h"
#include <mfast/json/json.h>
#include <sstream>

#define BOOST_TEST_DYN_LINK
#include <boost/test/test_tools.hpp>
#include <boost/test/unit_test.hpp>
#include "debug_allocator.h"


namespace mfast {
namespace json {
bool get_quoted_string(std::istream& strm, std::string* pstr, bool first_quote_extracted);
bool get_decimal_string(std::istream& strm, std::string& str);
bool skip_value (std::istream& strm);
}
}

BOOST_AUTO_TEST_SUITE( json_test_suite )

BOOST_AUTO_TEST_CASE(json_encode_product_test)
{
  using namespace test3;

  Product product_holder;
  Product_mref product_ref = product_holder.mref();

  product_ref.set_id().as(1);
  product_ref.set_price().as(12356, -2);
  product_ref.set_name().as("Foo");
  Product_mref::tags_mref tags = product_ref.set_tags();
  tags.resize(2);
  tags[0].as("Bar with \"quote\"");
  tags[1].as("Eek with \\");

  Product_mref::stock_mref stock = product_ref.set_stock();
  stock.as_present();
  stock.set_warehouse().as(300);
  stock.set_retail().as(20);

  std::ostringstream ostrm;
  mfast::json::encode(ostrm, product_ref);


  const char* result = "{\"id\":1,\"name\":\"Foo\",\"price\":123.56,\"tags\":[\"Bar with \\\"quote\\\"\",\"Eek with \\\\\"],\"stock\":{\"warehouse\":300,\"retail\":20}}";
  // std::cout << strm.str() << "\n";
  // std::cout << result << "\n";

  BOOST_CHECK_EQUAL(ostrm.str(),
                    std::string(result));

  debug_allocator alloc;
  Product product2_holder(product_ref, &alloc);

  Product product3_holder;
  std::istringstream istrm(result);
  BOOST_CHECK(mfast::json::decode(istrm, product3_holder.mref()));
  //
  // BOOST_CHECK(product3_holder.cref() == product_ref);
}

BOOST_AUTO_TEST_CASE(json_encode_person_test)
{
  using namespace test3;

  Person person_holder;
  Person_mref person_ref = person_holder.mref();

  person_ref.set_firstName().as("John");
  person_ref.set_lastName().as("Smith");
  person_ref.set_age().as(25);
  Person_mref::phoneNumbers_mref phones = person_ref.set_phoneNumbers();
  phones.resize(2);
  phones[0].set_type().as("home");
  phones[0].set_number().as("212 555-1234");

  phones[1].set_type().as("fax");
  phones[1].set_number().as("646 555-4567");

  LoginAccount_mref login = person_ref.set_login().as<LoginAccount>();
  login.set_userName().as("John");
  login.set_password().as("J0hnsm1th");

  person_ref.set_bankAccounts().resize(1);
  BankAccount_mref acct1 = person_ref.set_bankAccounts()[0].as<BankAccount>();
  acct1.set_number().as(12345678);
  acct1.set_routingNumber().as(87654321);

  std::stringstream strm;
  mfast::json::encode(strm, person_ref);
  const char* result = "{\"firstName\":\"John\",\"lastName\":\"Smith\",\"age\":25,"
                       "\"phoneNumbers\":[{\"type\":\"home\",\"number\":\"212 555-1234\"},{\"type\":\"fax\",\"number\":\"646 555-4567\"}],"
                       "\"emails\":[],\"login\":{\"userName\":\"John\",\"password\":\"J0hnsm1th\"},\"bankAccounts\":[{\"number\":12345678,\"routingNumber\":87654321}]}";

  BOOST_CHECK_EQUAL(strm.str(),
                    std::string(result));

  debug_allocator alloc;

  Person person_holder2(person_ref, &alloc);
}



BOOST_AUTO_TEST_CASE(test_get_quoted_string)
{
  using namespace mfast::json;
  std::string str;
  {
    std::stringstream strm("\"abcd\",");
    BOOST_CHECK(get_quoted_string(strm, &str, false));
    BOOST_CHECK_EQUAL(str, std::string("abcd"));
    strm >> str;
    BOOST_CHECK_EQUAL(str, std::string(","));
  }
  {
    std::stringstream strm("\"abc\\\"d\",");
    BOOST_CHECK(get_quoted_string(strm, &str, false));
    BOOST_CHECK_EQUAL(str, std::string("abc\"d"));
    strm >> str;
    BOOST_CHECK_EQUAL(str, std::string(","));
  }
  {
    std::stringstream strm("\"abc\\nd\",");
    BOOST_CHECK(get_quoted_string(strm, &str, false));
    BOOST_CHECK_EQUAL(str, std::string("abc\nd"));
    strm >> str;
    BOOST_CHECK_EQUAL(str, std::string(","));
  }
}

BOOST_AUTO_TEST_CASE(test_get_decimal_string)
{
  using namespace mfast::json;
  std::string str;
  {
    std::stringstream strm(" 123.45,");
    BOOST_CHECK(get_decimal_string(strm, str));
    BOOST_CHECK_EQUAL(str, std::string("123.45"));
    strm >> str;
    BOOST_CHECK_EQUAL(str, std::string(","));
  }
  {
    std::stringstream strm(" -123.45,");
    BOOST_CHECK(get_decimal_string(strm, str));
    BOOST_CHECK_EQUAL(str, std::string("-123.45"));
    strm >> str;
    BOOST_CHECK_EQUAL(str, std::string(","));
  }
  {
    std::stringstream strm(" 123.45E+12,");
    BOOST_CHECK(get_decimal_string(strm, str));
    BOOST_CHECK_EQUAL(str, std::string("123.45E+12"));
    strm >> str;
    BOOST_CHECK_EQUAL(str, std::string(","));
  }
  {
    std::stringstream strm(" 123.45E-12,");
    BOOST_CHECK(get_decimal_string(strm, str));
    BOOST_CHECK_EQUAL(str, std::string("123.45E-12"));
    strm >> str;
    BOOST_CHECK_EQUAL(str, std::string(","));
  }
  {
    std::stringstream strm(" 0.123,");
    BOOST_CHECK(get_decimal_string(strm, str));
    BOOST_CHECK_EQUAL(str, std::string("0.123"));
    strm >> str;
    BOOST_CHECK_EQUAL(str, std::string(","));
  }
}

BOOST_AUTO_TEST_CASE(test_skip_value)
{
  using namespace mfast::json;
  std::string str;
  {
    std::stringstream strm(" 123.45,");
    BOOST_CHECK(skip_value(strm));
    strm >> str;
    BOOST_CHECK_EQUAL(str, std::string(","));
  }
  {
    std::stringstream strm(" \"ab\\ncd\",");
    BOOST_CHECK(skip_value(strm));
    strm >> str;
    BOOST_CHECK_EQUAL(str, std::string(","));
  }
  {
    std::stringstream strm(" null,");
    BOOST_CHECK(skip_value(strm));
    strm >> str;
    BOOST_CHECK_EQUAL(str, std::string(","));
  }
  {
    std::stringstream strm(" true,");
    BOOST_CHECK(skip_value(strm));
    strm >> str;
    BOOST_CHECK_EQUAL(str, std::string(","));
  }
  {
    std::stringstream strm(" false,");
    BOOST_CHECK(skip_value(strm));
    strm >> str;
    BOOST_CHECK_EQUAL(str, std::string(","));
  }
  {
    std::stringstream strm(" [1, 2, 3],");
    BOOST_CHECK(skip_value(strm));
    strm >> str;
    BOOST_CHECK_EQUAL(str, std::string(","));
    BOOST_CHECK(strm.eof());
  }
  {
    std::stringstream strm(" [1, [ \"abc\" ], 3],");
    BOOST_CHECK(skip_value(strm));
    strm >> str;
    BOOST_CHECK_EQUAL(str, std::string(","));
    BOOST_CHECK(strm.eof());
  }
  {
    std::stringstream strm(" [1, { \"f1\":\"abc\" }, 3],");
    BOOST_CHECK(skip_value(strm));
    strm >> str;
    BOOST_CHECK_EQUAL(str, std::string(","));
    BOOST_CHECK(strm.eof());
  }
  {
    std::stringstream strm(" {\"id\":1,\"name\":\"Foo\"},");
    BOOST_CHECK(skip_value(strm));
    strm >> str;
    BOOST_CHECK_EQUAL(str, std::string(","));
    BOOST_CHECK(strm.eof());
  }
}


BOOST_AUTO_TEST_SUITE_END()
