#include "test4.h"
#include <mfast/field_comparator.h>


#define BOOST_TEST_DYN_LINK
#include <boost/test/test_tools.hpp>
#include <boost/test/unit_test.hpp>
#include "debug_allocator.h"


BOOST_AUTO_TEST_SUITE( composite_test_suite )

BOOST_AUTO_TEST_CASE(test_group)
{
  debug_allocator alloc;
  using namespace test4;

  Address addr1(&alloc);

  Address_mref addr_mref(addr1.mref());
  addr_mref.set_streetAddress().as("12140 Woodcrest Executive Dr.");
  addr_mref.set_city().as("Saint Louis");
  addr_mref.set_state().as("MO");
  addr_mref.set_postalCode().as(63141);

  debug_allocator alloc2;

  Address addr2(addr1.cref(), &alloc2);

  BOOST_CHECK(addr1.cref() == addr2.cref());
}

BOOST_AUTO_TEST_CASE(test_sequence)
{
  debug_allocator alloc1;
  using namespace test4;

  PhoneNumbers numbers_holder1(&alloc1);

  PhoneNumbers_mref numbers_mref (numbers_holder1.mref());

  numbers_mref.resize(2);
  numbers_mref[0].set_type().as("cell");
  numbers_mref[0].set_number().as("314-1234567");
  numbers_mref[1].set_type().as("home");
  numbers_mref[1].set_number().as("314-7654321");

  debug_allocator alloc2;
  PhoneNumbers numbers_holder2(numbers_holder1.cref(), &alloc2);

  BOOST_CHECK(numbers_holder1.cref() == numbers_holder2.cref());
}

BOOST_AUTO_TEST_CASE(test_template)
{
  debug_allocator alloc;
  using namespace test4;

  Person persion1(&alloc);

  Person_mref person_mref(persion1.mref());
  BOOST_CHECK_EQUAL(person_mref.get_firstName().instruction()->field_type(), mfast::field_type_unicode_string);
  person_mref.set_firstName().as("John");
  person_mref.set_lastName().as("Doe");

  person_mref.set_bloodType().as_O();
  BOOST_CHECK_EQUAL(person_mref.get_bloodType().value(), Person_cref::bloodType::O);
  BOOST_CHECK(person_mref.get_bloodType().is_O());
  BOOST_CHECK_EQUAL(strcmp(person_mref.get_bloodType().value_name(), "O") , 0);


  person_mref.set_discrete().as_Five();
  BOOST_CHECK_EQUAL(person_mref.get_discrete().value(), DiscreteEnum::Five);
  BOOST_CHECK(person_mref.get_discrete().is_Five());
  BOOST_CHECK_EQUAL(strcmp(person_mref.get_discrete().value_name(), "Five") , 0);

  person_mref.set_salary().as(20.0);
  BOOST_CHECK_EQUAL(person_mref.get_salary().exponent(), -2);
  BOOST_CHECK_EQUAL(person_mref.get_salary().mantissa(), 2000);


}


BOOST_AUTO_TEST_SUITE_END()
