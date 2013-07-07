#include <mfast/value_storage.h>


#define BOOST_TEST_DYN_LINK
#include <boost/test/test_tools.hpp>
#include <boost/test/unit_test.hpp>

using namespace mfast;

static const void* NULLPTR = 0;

BOOST_AUTO_TEST_SUITE( test_value_storage )

BOOST_AUTO_TEST_CASE(default_constructor_test)
{
  using namespace mfast;
  value_storage value;
  
  BOOST_CHECK_EQUAL(value.of_uint.present_, 0);
  BOOST_CHECK_EQUAL(value.of_uint.defined_bit_, 0);
  BOOST_CHECK_EQUAL(value.of_uint.content_, 0LL);
  
  BOOST_CHECK_EQUAL(value.of_decimal.present_, 0);
  BOOST_CHECK_EQUAL(value.of_decimal.exponent_, 0);
  BOOST_CHECK_EQUAL(value.of_decimal.defined_bit_, 0);
  BOOST_CHECK_EQUAL(value.of_decimal.mantissa_, 0LL);
  
  BOOST_CHECK_EQUAL(value.of_group.present_, 0);
  BOOST_CHECK_EQUAL(value.of_group.own_content_, 0);
  BOOST_CHECK_EQUAL(value.of_group.defined_bit_, 0);
  BOOST_CHECK_EQUAL(value.of_group.content_, NULLPTR);
  
  BOOST_CHECK_EQUAL(value.of_array.len_, 0);
  BOOST_CHECK_EQUAL(value.of_array.capacity_, 0);
  BOOST_CHECK_EQUAL(value.of_array.defined_bit_, 0);
  BOOST_CHECK_EQUAL(value.of_array.content_, NULLPTR);
  
  BOOST_CHECK_EQUAL(value.of_templateref.of_instruction.instruction_, NULLPTR);
  BOOST_CHECK_EQUAL(value.of_templateref.content_, NULLPTR);
}

BOOST_AUTO_TEST_CASE(non_empty_constructor_test)
{
  using namespace mfast;
  value_storage value(1);

  BOOST_CHECK(value.of_uint.present_ != 0);
  BOOST_CHECK_EQUAL(value.of_uint.defined_bit_, 1);
  BOOST_CHECK_EQUAL(value.of_uint.content_, 0LL);
  
  BOOST_CHECK(value.of_decimal.present_ != 0);
  BOOST_CHECK_EQUAL(value.of_decimal.exponent_, 0);
  BOOST_CHECK_EQUAL(value.of_decimal.defined_bit_, 1);
  BOOST_CHECK_EQUAL(value.of_decimal.mantissa_, 0LL);
  
  BOOST_CHECK(value.of_group.present_ != 0);
  BOOST_CHECK_EQUAL(value.of_group.own_content_, 0);
  BOOST_CHECK_EQUAL(value.of_group.defined_bit_, 1);
  BOOST_CHECK_EQUAL(value.of_group.content_, NULLPTR);
  
  BOOST_CHECK(value.of_array.len_ != 0);
  BOOST_CHECK_EQUAL(value.of_array.capacity_, 0);
  BOOST_CHECK_EQUAL(value.of_array.defined_bit_, 1);
  BOOST_CHECK_EQUAL(value.of_array.content_, NULLPTR);
}

BOOST_AUTO_TEST_SUITE_END()
