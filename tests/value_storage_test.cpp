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
  value_storage_t value;
  
  BOOST_CHECK_EQUAL(value.uint_storage.present_, 0);
  BOOST_CHECK_EQUAL(value.uint_storage.defined_bit_, 0);
  BOOST_CHECK_EQUAL(value.uint_storage.content_, 0LL);
  
  BOOST_CHECK_EQUAL(value.decimal_storage.present_, 0);
  BOOST_CHECK_EQUAL(value.decimal_storage.exponent_, 0);
  BOOST_CHECK_EQUAL(value.decimal_storage.defined_bit_, 0);
  BOOST_CHECK_EQUAL(value.decimal_storage.mantissa_, 0LL);
  
  BOOST_CHECK_EQUAL(value.group_storage.present_, 0);
  BOOST_CHECK_EQUAL(value.group_storage.own_content_, 0);
  BOOST_CHECK_EQUAL(value.group_storage.defined_bit_, 0);
  BOOST_CHECK_EQUAL(value.group_storage.content_, NULLPTR);
  
  BOOST_CHECK_EQUAL(value.array_storage.len_, 0);
  BOOST_CHECK_EQUAL(value.array_storage.capacity_, 0);
  BOOST_CHECK_EQUAL(value.array_storage.defined_bit_, 0);
  BOOST_CHECK_EQUAL(value.array_storage.content_, NULLPTR);
  
  BOOST_CHECK_EQUAL(value.templateref_storage.instruction_storage.instruction_, NULLPTR);
  BOOST_CHECK_EQUAL(value.templateref_storage.content_, NULLPTR);
}

BOOST_AUTO_TEST_CASE(non_empty_constructor_test)
{
  using namespace mfast;
  value_storage_t value(1);

  BOOST_CHECK(value.uint_storage.present_ != 0);
  BOOST_CHECK_EQUAL(value.uint_storage.defined_bit_, 1);
  BOOST_CHECK_EQUAL(value.uint_storage.content_, 0LL);
  
  BOOST_CHECK(value.decimal_storage.present_ != 0);
  BOOST_CHECK_EQUAL(value.decimal_storage.exponent_, 0);
  BOOST_CHECK_EQUAL(value.decimal_storage.defined_bit_, 1);
  BOOST_CHECK_EQUAL(value.decimal_storage.mantissa_, 0LL);
  
  BOOST_CHECK(value.group_storage.present_ != 0);
  BOOST_CHECK_EQUAL(value.group_storage.own_content_, 0);
  BOOST_CHECK_EQUAL(value.group_storage.defined_bit_, 1);
  BOOST_CHECK_EQUAL(value.group_storage.content_, NULLPTR);
  
  BOOST_CHECK(value.array_storage.len_ != 0);
  BOOST_CHECK_EQUAL(value.array_storage.capacity_, 0);
  BOOST_CHECK_EQUAL(value.array_storage.defined_bit_, 1);
  BOOST_CHECK_EQUAL(value.array_storage.content_, NULLPTR);
}

BOOST_AUTO_TEST_SUITE_END()
