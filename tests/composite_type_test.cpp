// Copyright (c) 2013, 2014, Huang-Ming Huang,  Object Computing, Inc.
// All rights reserved.
//
// This file is part of mFAST.
//
//     mFAST is free software: you can redistribute it and/or modify
//     it under the terms of the GNU Lesser General Public License as published by
//     the Free Software Foundation, either version 3 of the License, or
//     (at your option) any later version.
//
//     mFAST is distributed in the hope that it will be useful,
//     but WITHOUT ANY WARRANTY; without even the implied warranty of
//     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//     GNU General Public License for more details.
//
//     You should have received a copy of the GNU Lesser General Public License
//     along with mFast.  If not, see <http://www.gnu.org/licenses/>.
//

#include "test4.h"
#include <mfast/field_comparator.h>


#define BOOST_TEST_DYN_LINK
#include <boost/test/test_tools.hpp>
#include <boost/test/unit_test.hpp>
#include "debug_allocator.h"
#include <boost/foreach.hpp>
#include <boost/assign/list_of.hpp> // for 'list_of()'
using namespace boost::assign; // bring 'list_of()' into scope

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
  BOOST_CHECK(numbers_holder1.mref() == numbers_holder2.mref());

  // Testing sequence of primitive types

  Emails emails;

  boost::string_ref values[] = {
    "test1@.email.com",
    "test2@.email.com",
    "test3@.email.com",
  };

  emails.mref().assign(values, values+3);

  BOOST_REQUIRE_EQUAL(emails.cref().size(), 3U);
  BOOST_CHECK(emails.cref()[0] == values[0]);
  BOOST_CHECK(emails.cref()[1] == values[1]);
  BOOST_CHECK(emails.cref()[2] == values[2]);

  std::size_t index = 0;

  for (Emails_cref::iterator it = emails.cref().begin(); it != emails.cref().end(); ++it)
  {
    BOOST_CHECK( it->value() == values[index++] );
  }

  BOOST_REVERSE_FOREACH(mfast::ascii_string_cref ref, emails.cref())
  {
    BOOST_CHECK_EQUAL(ref.value(), values[--index] );
  }

  emails.mref().as( list_of("test1")("test2") );
  BOOST_REQUIRE_EQUAL(emails.cref().size(), 2U);
  BOOST_CHECK(emails.cref()[0] == "test1");
  BOOST_CHECK(emails.cref()[1] == "test2");

  BOOST_REVERSE_FOREACH(mfast::ascii_string_mref ref, emails.mref())
  {
    ref.as(values[index++]);
  }

  BOOST_FOREACH(mfast::ascii_string_cref ref, emails.cref())
  {
    BOOST_CHECK_EQUAL(ref.value(), values[--index] );
  }

  // Testing sequence of aggregate types

  Addresses addresses;

  BOOST_FOREACH (Address_mref addr, addresses.mref())
  {
    addr.instruction();
  }

  Addresses_mref::iterator it = addresses.mref().grow_by(2);
  BOOST_CHECK_EQUAL(addresses.cref().size(), 2U);
  BOOST_CHECK(it == addresses.mref().begin());
  index = 0;

  for (Addresses_mref::iterator itr = addresses.mref().begin(); itr != addresses.mref().end(); ++itr)
  {
    itr->set_postalCode().as(static_cast<uint32_t>(index+10000));
    ++index;
  }

  index = 0;


  BOOST_FOREACH (Address_mref addr, addresses.mref())
  {
    addr.set_postalCode().as(static_cast<uint32_t>(index+10000));
    ++index;
  }

  index = 0;

  BOOST_FOREACH(Address_cref addr, addresses.cref())
  {
    BOOST_CHECK_EQUAL(addr.get_postalCode().value(), index+10000);
    ++index;
  }
}

BOOST_AUTO_TEST_CASE(test_template)
{
  debug_allocator alloc;
  using namespace test4;

  Person person1(&alloc);

  Person_mref person_mref(person1.mref());
  BOOST_CHECK_EQUAL(person_mref.get_firstName().instruction()->field_type(), mfast::field_type_unicode_string);
  person_mref.set_firstName().as("John");
  person_mref.set_lastName().as("Doe");

  // testing the default value
  BOOST_CHECK(person_mref.get_gender().is_female());

  person_mref.set_bloodType().as_O();
  BOOST_CHECK_EQUAL(person_mref.get_bloodType().value(), Person_cref::bloodType::O);
  BOOST_CHECK(person_mref.get_bloodType().is_O());
  BOOST_CHECK_EQUAL(strcmp(person_mref.get_bloodType().value_name(), "O"), 0);


  person_mref.set_discrete().as_Five();
  BOOST_CHECK_EQUAL(person_mref.get_discrete().value(), DiscreteEnum::Five);
  BOOST_CHECK(person_mref.get_discrete().is_Five());
  BOOST_CHECK_EQUAL(strcmp(person_mref.get_discrete().value_name(), "Five"), 0);

  person_mref.set_salary().as(20.0);
  BOOST_CHECK_EQUAL(person_mref.get_salary().exponent(),                    -2);
  BOOST_CHECK_EQUAL(person_mref.get_salary().mantissa(),                  2000);


  BOOST_CHECK_EQUAL(person_mref.get_id().size(),                           16U);


  int num_addresses = 0;

  BOOST_FOREACH(Address_cref addr, person_mref.get_addresses())
  {
    num_addresses += 1;
    addr.instruction(); // avoid unused variable warning
  }

  BOOST_CHECK_EQUAL(num_addresses, 0);

  debug_allocator alloc2;

  // testing group assignment
  person1.mref().set_education().set_college().as("SLU");

  Person tmp_person(&alloc2);

  BOOST_CHECK(!tmp_person.cref().get_education().present());

  tmp_person.mref().set_education().set_college().as("SLU");
  tmp_person.mref().set_education().set_high_school().as("Kirkwood");

  BOOST_CHECK(tmp_person.cref().get_education().present());

  person1.mref().set_education().as( tmp_person.cref().get_education() );
  BOOST_CHECK(person1.cref().get_education() == tmp_person.cref().get_education());


  // testing group link
  person1.mref().set_education().set_college().as("UMSTL");
  // tmp_person.mref().link_education( person1.mref().set_education() );
  // BOOST_CHECK_EQUAL( tmp_person.cref().get_education().get_college().value(),
  //                    person1.cref().get_education().get_college().value());


  // testing sequence assignment
  person1.mref().set_addresses().resize(4);
  BOOST_CHECK_EQUAL(person1.cref().get_addresses().size(), 4U);


  unsigned index = 0;
  BOOST_FOREACH (Address_mref addr, person1.mref().set_addresses())
  {
    addr.set_postalCode().as(index+10000);
    ++index;
  }

  index = 0;

  BOOST_FOREACH(Address_cref addr, person1.cref().get_addresses())
  {
    BOOST_CHECK_EQUAL(addr.get_postalCode().value(), index+10000);
    ++index;
  }

  person1.mref().set_emails().as(list_of("test1")("test2"));


  Person person2(person1.cref(), &alloc2);

  BOOST_CHECK(person1.cref() == person2.cref());

  Person person3(&alloc);

  person3.mref().as(person1.cref());

  BOOST_CHECK(person1.cref() == person3.cref());


}


BOOST_AUTO_TEST_SUITE_END()
