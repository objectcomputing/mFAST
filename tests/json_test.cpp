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

#include "test3.h"
#include <mfast/json/json.h>
#include <sstream>

#define BOOST_TEST_DYN_LINK
#include <boost/test/test_tools.hpp>
#include <boost/test/unit_test.hpp>
#include "debug_allocator.h"


namespace mfast {
namespace json {
bool get_quoted_string(std::istream&                  strm,
                       std::string*                   pstr,
                       const mfast::byte_vector_mref* pref,
                       bool                           first_quote_extracted=false);
bool skip_value (std::istream& strm);


namespace encode_detail {

struct quoted_string {
  quoted_string(const char* str)
    : str_(str)
  {
  }

  const char* str_;
};


std::ostream& operator << (std::ostream& os, quoted_string str);


}
}
}


BOOST_AUTO_TEST_SUITE( json_test_suite )

BOOST_AUTO_TEST_CASE(json_encode_product_test)
{
  using namespace test3;

  Product product_holder;
  Product_mref product_ref = product_holder.mref();

  // product_ref.set_id().as(1);
  product_ref.set_price().as(12356, -2);
  product_ref.set_name().as("Foo");
  Product_mref::tags_mref tags = product_ref.set_tags();
    BOOST_CHECK_EQUAL(tags.instruction()->field_type(),             mfast::field_type_sequence);
    BOOST_CHECK_EQUAL(tags.instruction()->subinstructions().size(), 1U);

  tags.resize(2);
    BOOST_CHECK_EQUAL(tags.size(),                                  2U);

  mfast::ascii_string_mref tag0 = tags[0];
    BOOST_CHECK_EQUAL(tag0.instruction()->field_type(),             mfast::field_type_ascii_string);


  tags[0].as("Bar with \"quote\"");
    BOOST_CHECK_EQUAL(strcmp(tags[0].c_str(), "Bar with \"quote\""),0);
  tags[1].as("Eek with \\");
    BOOST_CHECK_EQUAL(strcmp(tags[1].c_str(), "Eek with \\"),       0);

  Product_mref::stock_mref stock = product_ref.set_stock();
  stock.set_warehouse().as(300);
  stock.set_retail().as(20);

  const unsigned char ext_data[] = "{\"test1\":1}";
  // product_ref.set_ext().assign(ext_data, ext_data+sizeof(ext_data)-1);

  product_ref.set_ext().refers_to(ext_data, sizeof(ext_data)-1);
  const char* result = "{\"name\":\"Foo\",\"price\":123.56,\"tags\":[\"Bar with \\\"quote\\\"\",\"Eek with \\\\\"],\"stock\":{\"warehouse\":300,\"retail\":20},\"ext\":{\"test1\":1}}";

  {
    std::ostringstream ostrm;
    mfast::json::encode(ostrm,
                        product_ref,
                        mfast_tag::JSON_UNKNOWN);


    // std::cout << strm.str() << "\n";
    // std::cout << result << "\n";

    BOOST_CHECK_EQUAL(ostrm.str(),
                      std::string(result));
  }
  {

    std::ostringstream ostrm;
    mfast::json::encode(ostrm,
                        product_ref,
                        mfast_tag::JSON_UNKNOWN,
                        mfast_tag::JSON_IGNORE);

    const char* result = "{\"name\":\"Foo\",\"price\":123.56,\"stock\":{\"warehouse\":300,\"retail\":20},\"ext\":{\"test1\":1}}";

    BOOST_CHECK_EQUAL(ostrm.str(),
                      std::string(result));
  }

  debug_allocator alloc;
  Product product2_holder(product_ref, &alloc);

  Product product3_holder;
  std::istringstream istrm(result);
  mfast::json::decode(istrm,
                      product3_holder.mref(),
                      mfast_tag::JSON_UNKNOWN);
  //
  BOOST_CHECK(product3_holder.cref() == product_ref);

  product_ref.omit_stock();
  BOOST_CHECK(product_ref.get_stock().absent());
  BOOST_CHECK_THROW(product_ref.try_get_stock(), mfast::bad_optional_access);
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

  BOOST_CHECK(person_ref.get_login().present());

  person_ref.set_bankAccounts().grow_by(1);
  BOOST_CHECK_EQUAL(person_ref.get_bankAccounts().size(), 1U);

  BankAccount_mref acct0 = person_ref.set_bankAccounts()[0].as<BankAccount>();
  acct0.set_number().as(12345678);
  acct0.set_routingNumber().as(87654321);


  BOOST_CHECK_EQUAL(person_ref.get_bankAccounts().size(), 1U);
  mfast::nested_message_cref n0 = person_ref.get_bankAccounts()[0];
  BankAccount_cref acct0_read = static_cast<BankAccount_cref>(n0.target());

  BOOST_CHECK_EQUAL(acct0_read.get_number().value(),        12345678U);
  BOOST_CHECK_EQUAL(acct0_read.get_routingNumber().value(), 87654321U);


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

BOOST_AUTO_TEST_CASE(json_decode_null_test)
{
  using namespace test3;

  try {
    LoginAccount account_holder;

    const char result[] = "{\"userName\":\"test\",\"password\": null}";
    std::stringstream strm(result);

    mfast::json::decode(strm, account_holder.mref());
    BOOST_CHECK_EQUAL(account_holder.cref().get_userName().value(), boost::string_ref("test"));
    BOOST_CHECK( account_holder.cref().get_password().absent() );
  }
  catch (boost::exception& ex)
  {
    std::cerr << diagnostic_information(ex);
  }
}



BOOST_AUTO_TEST_CASE(test_encode_quoted_string)
{
  {
    std::stringstream stream;
    mfast::json::encode_detail::quoted_string str("abcd\x01\b\t\f\n\r\\\"");
    stream << str;
    BOOST_CHECK_EQUAL(stream.str(), std::string("\"abcd\\u0001\\b\\t\\f\\n\\r\\\\\\\"\"") );
  }

  {
    std::stringstream stream;
    mfast::json::encode_detail::quoted_string str("\xE2\x80\x93");
    stream << str;
    BOOST_CHECK_EQUAL(stream.str(), std::string("\"\xE2\x80\x93\"") );
  }
}

BOOST_AUTO_TEST_CASE(test_get_quoted_string)
{
  using namespace mfast;
  using namespace mfast::json;
  debug_allocator alloc;

  std::string str;

  const byte_vector_field_instruction byte_vector_field_instruction_prototype(operator_none,presence_mandatory,0,0,"",0, string_value_storage(), 0, "", "");
  value_storage storage;

  byte_vector_field_instruction_prototype.construct_value(storage, &alloc);

  byte_vector_mref bv_ref(&alloc,
                          &storage,
                          &byte_vector_field_instruction_prototype);


  {
    const char data[] = "\"abcd\",";
    std::stringstream strm(data);
    BOOST_CHECK(get_quoted_string(strm, &str, &bv_ref, false));
    BOOST_CHECK_EQUAL(str,           std::string("abcd"));

    BOOST_CHECK_EQUAL(bv_ref.size(), sizeof(data)-2 );
    BOOST_CHECK(memcmp(data, bv_ref.data(), sizeof(data)-2 ) ==0);

    strm >> str;
    BOOST_CHECK_EQUAL(str, std::string(","));
  }
  {
    bv_ref.clear();
    const char data[] = "\"abc\\\"d\",";
    std::stringstream strm(data);
    BOOST_CHECK(get_quoted_string(strm, &str, &bv_ref, false));
    BOOST_CHECK_EQUAL(str,           std::string("abc\"d"));

    BOOST_CHECK_EQUAL(bv_ref.size(), sizeof(data)-2 );
    BOOST_CHECK(memcmp(data, bv_ref.data(), sizeof(data)-2) == 0);

    strm >> str;
    BOOST_CHECK_EQUAL(str, std::string(","));
  }
  {
    bv_ref.clear();
    const char data[] = "\"\\u4e2d\\u83EF\\u6c11\\u570B\",";
    std::stringstream strm(data);
    BOOST_CHECK(get_quoted_string(strm, &str, &bv_ref, false));
    BOOST_CHECK_EQUAL(str,           std::string("中華民國"));

    BOOST_CHECK_EQUAL(bv_ref.size(), sizeof(data)-2 );
    BOOST_CHECK(memcmp(data, bv_ref.data(), sizeof(data)-2 ) ==0);

    strm >> str;
    BOOST_CHECK_EQUAL(str, std::string(","));
  }

  {
    bv_ref.clear();
    // U+10437 U+24B62
    const char data[] = "\"\\uD801\\uDC37\\uD852\\uDF62\",";
    std::stringstream strm(data);
    BOOST_CHECK(get_quoted_string(strm, &str, &bv_ref, false));
    BOOST_CHECK_EQUAL(str,           std::string("\xF0\x90\x90\xB7\xF0\xA4\xAD\xA2"));

    BOOST_CHECK_EQUAL(bv_ref.size(), sizeof(data)-2 );
    BOOST_CHECK(memcmp(data, bv_ref.data(), sizeof(data)-2 ) ==0);

    strm >> str;
    BOOST_CHECK_EQUAL(str, std::string(","));
  }

  // {
  //     bv_ref.clear();
  //     const char data[] = "\"Konu\\u0015fal\\u00131m bir gün\",";
  //     std::stringstream strm(data);
  //     BOOST_CHECK(get_quoted_string(strm, &str, &bv_ref, false));
  // }
  {
    bv_ref.clear();
    const char data[] = "\"abc\\nd\\/\",";
    std::stringstream strm(data);
    BOOST_CHECK(get_quoted_string(strm, &str, &bv_ref, false));
    BOOST_CHECK_EQUAL(str,           std::string("abc\nd/"));

    BOOST_CHECK_EQUAL(bv_ref.size(), sizeof(data)-2);
    BOOST_CHECK(memcmp(data, bv_ref.data(), sizeof(data)-2) ==0);

    strm >> str;
    BOOST_CHECK_EQUAL(str, std::string(","));
  }
  byte_vector_field_instruction_prototype.destruct_value(storage, &alloc);

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

BOOST_AUTO_TEST_CASE(test_seq_codegen)
{
  using namespace test3;

  const UsingSeqTemplates::instruction_type* top_inst = UsingSeqTemplates::instruction();
  BOOST_CHECK_EQUAL(top_inst->subinstructions().size(), 3U);

  const mfast::sequence_field_instruction* seq1_inst = dynamic_cast<const mfast::sequence_field_instruction*>(top_inst->subinstruction(1));
  BOOST_REQUIRE(seq1_inst);
  BOOST_CHECK(strcmp(seq1_inst->name(), "seq1")==0);
  BOOST_CHECK_EQUAL(seq1_inst->subinstructions().size(), 2U);
  BOOST_CHECK_EQUAL(seq1_inst->ref_instruction(),        SeqTemplate1::instruction());
  BOOST_CHECK_EQUAL(seq1_inst->element_instruction(),    (const mfast::group_field_instruction*) 0);


  const mfast::sequence_field_instruction* seq2_inst = dynamic_cast<const mfast::sequence_field_instruction*>(top_inst->subinstruction(2));
  BOOST_REQUIRE(seq2_inst);
  BOOST_CHECK(strcmp(seq2_inst->name(), "seq2")==0);
  BOOST_CHECK_EQUAL(seq2_inst->subinstructions().size(), 4U);
  BOOST_CHECK_EQUAL(seq2_inst->ref_instruction(),        SeqTemplate2::instruction());
  BOOST_CHECK_EQUAL(seq2_inst->element_instruction(),    BankAccount::instruction());
}


BOOST_AUTO_TEST_CASE(test_decimal_output)
{
  using mfast::decimal_value_storage;

  {
    std::stringstream strm;
    strm << decimal_value_storage(12345, 0);
    BOOST_CHECK_EQUAL(strm.str(), std::string("12345"));
  }
  {
    std::stringstream strm;
    strm << decimal_value_storage(-12345, 0);
    BOOST_CHECK_EQUAL(strm.str(), std::string("-12345"));
  }
  {
    std::stringstream strm;
    strm << decimal_value_storage(12345, 2);
    BOOST_CHECK_EQUAL(strm.str(), std::string("1234500"));
  }
  {
    std::stringstream strm;
    strm << decimal_value_storage(-12345, 2);
    BOOST_CHECK_EQUAL(strm.str(), std::string("-1234500"));
  }
  {
    std::stringstream strm;
    strm << decimal_value_storage(12345, -2);
    BOOST_CHECK_EQUAL(strm.str(), std::string("123.45"));
  }
  {
    std::stringstream strm;
    strm << decimal_value_storage(-12345, -2);
    BOOST_CHECK_EQUAL(strm.str(), std::string("-123.45"));
  }
  {
    std::stringstream strm;
    strm << decimal_value_storage(12345, -5);
    BOOST_CHECK_EQUAL(strm.str(), std::string("0.12345"));
  }
  {
    std::stringstream strm;
    strm << decimal_value_storage(-12345, -5);
    BOOST_CHECK_EQUAL(strm.str(), std::string("-0.12345"));
  }
  {
    std::stringstream strm;
    strm << decimal_value_storage(12345, -7);
    BOOST_CHECK_EQUAL(strm.str(), std::string("0.0012345"));
  }
  {
    std::stringstream strm;
    strm << decimal_value_storage(-12345, -7);
    BOOST_CHECK_EQUAL(strm.str(), std::string("-0.0012345"));
  }
}

BOOST_AUTO_TEST_CASE(test_decimal_input)
{
  using namespace mfast::json;
  std::string str;
  mfast::decimal_value_storage storage;
  {
    std::stringstream strm(" 123.45,");
    strm >> storage;
    BOOST_CHECK_EQUAL(storage.mantissa(), 12345LL);
    BOOST_CHECK_EQUAL(storage.exponent(), -2);
    strm >> str;
    BOOST_CHECK_EQUAL(str,                std::string(","));
  }
  {
    std::stringstream strm(" -123.45,");
    strm >> storage;
    BOOST_CHECK_EQUAL(storage.mantissa(), -12345LL);
    BOOST_CHECK_EQUAL(storage.exponent(), -2);
    strm >> str;
    BOOST_CHECK_EQUAL(str,                std::string(","));
  }
  {
    std::stringstream strm(" 1.2345E+12,");
    strm >> storage;
    BOOST_CHECK_EQUAL(storage.mantissa(), 12345LL);
    BOOST_CHECK_EQUAL(storage.exponent(), 8);
    strm >> str;
    BOOST_CHECK_EQUAL(str,                std::string(","));
  }
  {
    std::stringstream strm(" 1.2345E-12,");
    strm >> storage;
    BOOST_CHECK_EQUAL(storage.mantissa(), 12345LL);
    BOOST_CHECK_EQUAL(storage.exponent(), -16);
    strm >> str;
    BOOST_CHECK_EQUAL(str,                std::string(","));
  }
  {
    std::stringstream strm(" 0.123,");
    strm >> storage;
    BOOST_CHECK_EQUAL(storage.mantissa(), 123LL);
    BOOST_CHECK_EQUAL(storage.exponent(), -3);
    strm >> str;
    BOOST_CHECK_EQUAL(str,                std::string(","));
  }
}


BOOST_AUTO_TEST_SUITE_END()
