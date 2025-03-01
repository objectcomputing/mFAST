#include "catch.hpp"
#include <mfast.h>
#include <mfast/coder/fast_encoder_v2.h>
#include <mfast/coder/fast_decoder_v2.h>

#include "byte_stream.h"

#include "simple12.h"

namespace
{
class fast_test_coding_case_v2
{
    public:
        fast_test_coding_case_v2():
            encoder_v2_(simple12::description()),
            decoder_v2_(simple12::description())
        {}

        bool
        encoding(const mfast::message_cref& msg_ref, const byte_stream& result, bool reset=false)
        {
            const int buffer_size = 128;
            char buffer[buffer_size];
      
            std::size_t encoded_size = encoder_v2_.encode(msg_ref, buffer, buffer_size, reset);
            if (result == byte_stream(buffer, encoded_size))
              return true;

            INFO(  "Got \"" << byte_stream(buffer, encoded_size) << "\" instead." );
            return false;
        }

        bool
        decoding(const byte_stream& bytes, const mfast::message_cref& result, bool reset=false)
        {
          const char* first = bytes.data();
          mfast::message_cref msg = decoder_v2_.decode(first, first+bytes.size(), reset);

          return (msg == result);
        }

    private:
        mfast::fast_encoder_v2 encoder_v2_;
        mfast::fast_decoder_v2<0> decoder_v2_;
};
}

TEST_CASE("simple field and sequence optional encoder_v2","[field_sequence_optional_sequence_encoder_v2]")
{
    fast_test_coding_case_v2 test_case;

    SECTION("encode field")
    {
        simple12::Test_1 test_1;
        simple12::Test_1_mref test_1_mref = test_1.mref();
        test_1_mref.set_field_1_1().as(1);
        // \xE0: 1110 : OK
        // 1 : Stop Bit
        // 1 : Set Template Id.
        // 1 : Set Field field_1_1
        // 0 : XXX
        REQUIRE(test_case.encoding(test_1.cref(),"\xE0\x81\x82\x81",true));
        REQUIRE(test_case.decoding("\xE0\x81\x82\x80",test_1.cref(),true));
    }

    SECTION("encode sequence")
    {
        simple12::Test_1 test_1;
        simple12::Test_1_mref test_1_mref = test_1.mref();

        auto sequence_1_mref = test_1_mref.set_sequence_1();
        sequence_1_mref.resize(1);
    
        auto element_sequence = sequence_1_mref.front();
        element_sequence.set_field_1_2().as(10);
        element_sequence.set_field_1_3().as(50);
        // \xC0: 1100 : OK
        // 1 : Stop Bit
        // 1 : Set Template Id.
        // 0 : Set Field field_1_1
        // 0 : XXX
        REQUIRE(test_case.encoding(test_1.cref(),"\xC0\x81\x82\x8B\xB3",true));
        REQUIRE(test_case.decoding("\xC0\x81\x82\x8B\xB3",test_1.cref(),true));
    }

    SECTION("encode field and sequence")
    {
        simple12::Test_1 test_1;
        simple12::Test_1_mref test_1_mref = test_1.mref();
        test_1_mref.set_field_1_1().as(1);
    
        auto sequence_1_mref = test_1_mref.set_sequence_1();
        sequence_1_mref.resize(1);

        auto element_sequence = sequence_1_mref.front();
        element_sequence.set_field_1_2().as(10);
        element_sequence.set_field_1_3().as(50);
        // \xE0: 1110 : OK
        // 1 : Stop Bit
        // 1 : Set Template Id.
        // 1 : Set Field field_1_1
        // 0 : XXX
        REQUIRE(test_case.encoding(test_1.cref(),"\xE0\x81\x82\x82\x8b\xB3",true));
        REQUIRE(test_case.decoding("\xE0\x81\x82\x82\x8b\xB3",test_1.cref(),true));
    }
}

TEST_CASE("simple field encoder_v2","[field_optional_encoder_v2]")
{
    fast_test_coding_case_v2 test_case;

    SECTION("encode fields")
    {
        simple12::Test_2 test_2;
        simple12::Test_2_mref test_2_mref = test_2.mref();
        test_2_mref.set_field_2_1().as(1);
        test_2_mref.set_field_2_2().as(2);
        // \xF0: 1111 : OK
        // 1 : Stop Bit
        // 1 : Set Template Id.
        // 1 : Set Field field_2_1
        // 1 : Set Field field_2_2
        REQUIRE(test_case.encoding(test_2.cref(),"\xF0\x82\x81\x82",true));
        REQUIRE(test_case.decoding("\xF0\x82\x81\x82",test_2.cref(),true));
    }
}

TEST_CASE("simple field and sequence encoder_v2","[field_sequence_encoder_v2]")
{
    fast_test_coding_case_v2 test_case;

    SECTION("encode fields")
    {
        simple12::Test_3 test_3;
        simple12::Test_3_mref test_3_mref = test_3.mref();
        test_3_mref.set_field_3_1().as(1);

        auto sequence_3_mref = test_3_mref.set_sequence_3();
        sequence_3_mref.resize(1);

        auto element_sequence = sequence_3_mref.front();
        element_sequence.set_field_3_2().as(10);
        element_sequence.set_field_3_3().as(50);
        // \xE0: 1110 : OK
        // 1 : Stop Bit
        // 1 : Set Template Id.
        // 1 : Set Field field_3_1
        // 0 : XXX
        REQUIRE(test_case.encoding(test_3.cref(),"\xE0\x83\x81\x81\x8A\xB2",true));
        REQUIRE(test_case.decoding("\xE0\x83\x81\x81\x8A\xB2",test_3.cref(),true));
    }
}

TEST_CASE("simple field optional encoder_v2","[field__optional_encoder_v2]")
{
    fast_test_coding_case_v2 test_case;

    SECTION("encode fields")
    {
        simple12::Test_4 test_4;
        simple12::Test_4_mref test_4_mref = test_4.mref();
        test_4_mref.set_field_4_1().as(1);
        test_4_mref.set_field_4_2().as(2);
        // \xF0 : 1111 : OK
        // 1 : Stop Bit
        // 1 : Set Template Id.
        // 1 : Set Field field_4_1
        // 1 : Set Field field_4_2
        REQUIRE(test_case.encoding(test_4.cref(),"\xF0\x84\x82\x83",true));
        REQUIRE(test_case.decoding("\xF0\x84\x82\x83",test_4.cref(),true));
    }

    SECTION("encode first optional field")
    {
        simple12::Test_4 test_4;
        simple12::Test_4_mref test_4_mref = test_4.mref();
        test_4_mref.set_field_4_1().as(1);
        // \xE0 : 1110 : OK
        // 1 : Stop Bit.
        // 1 : Set Template Id.
        // 1 : Set Field field_4_1
        // 0 : Set Field field_4_2
        REQUIRE(test_case.encoding(test_4.cref(),"\xE0\x84\x82",true));
        REQUIRE(test_case.decoding("\xE0\x84\x82",test_4.cref(),true));
    }

    SECTION("encode second optional field")
    {
        simple12::Test_4 test_4;
        simple12::Test_4_mref test_4_mref = test_4.mref();
        test_4_mref.set_field_4_2().as(2);
        // \xD0 : 1110 : OK
        // 1 : Stop Bit.
        // 1 : Set Template Id.
        // 0 : Set Field field_4_1
        // 1 : Set Field field_4_2
        REQUIRE(test_case.encoding(test_4.cref(),"\xD0\x84\x83",true));
        REQUIRE(test_case.decoding("\xD0\x84\x83",test_4.cref(),true));
    }
}

TEST_CASE("simple field group optional encoder_v2","[field_group_optional_encoder_v2]")
{
    fast_test_coding_case_v2 test_case;

    SECTION("encode field")
    {
        simple12::Test_5 test_5;
        simple12::Test_5_mref test_5_mref = test_5.mref();
        test_5_mref.set_field_5_1().as(1);
        // Error: Expecting value is \xE0 : 1110
        // 1 : Stop Bit.
        // 1 : Set Template Id.
        // 1 : Set Field field_5_1
        // 0 : Not Set test_5_group
        // Get value: F0, expecting E0
        // 1 : Stop Bit.
        // 1 : Set Template Id.
        // 1 : Set Field field_5_1
        // 1 : Set test_5_group -> False!
        REQUIRE(test_case.encoding(test_5.cref(),"\xE0\x85\x82",true));
        REQUIRE(test_case.decoding("\xE0\x85\x82",test_5.cref(),true));
    }

    SECTION("encode group")
    {
        simple12::Test_5 test_5;
        simple12::Test_5_mref test_5_mref = test_5.mref();
        test_5_mref.omit_field_5_1();
        auto group_test_5_group = test_5_mref.set_test_5_group();
        auto group_test_5_sequence = group_test_5_group.set_sequence_5();
        group_test_5_sequence.resize(1);
        {
            auto element_sequence = group_test_5_sequence.front();
            element_sequence.set_field_5_2().as(1);
            element_sequence.set_field_5_3().as(2);
        }
        group_test_5_group.set_field_5_5().as(10);
        // \xD0 : 1110 : OK
        // 1 : Stop Bit.
        // 1 : Set Template Id.
        // 0 : Not Set Field field_5_1
        // 1 : Set Group test_5_group
        REQUIRE(test_case.encoding(test_5.cref(),"\xD0\x85\xC0\x81\x81\x82\x8B",true));
        REQUIRE(test_case.decoding("\xD0\x85\xC0\x81\x81\x82\x8B",test_5.cref(),true));
    }

    SECTION("encode field and group")
    {
        simple12::Test_5 test_5;
        simple12::Test_5_mref test_5_mref = test_5.mref();
        test_5_mref.set_field_5_1().as(1);
        auto group_test_5_group = test_5_mref.set_test_5_group();
        auto group_test_5_sequence = group_test_5_group.set_sequence_5();
        group_test_5_sequence.resize(1);
        {
            auto element_sequence = group_test_5_sequence.front();
            element_sequence.set_field_5_2().as(1);
            element_sequence.set_field_5_3().as(2);
        }
        group_test_5_group.set_field_5_5().as(10);
        // \xF0 : 1111 : OK
        // 1 : Stop Bit.
        // 1 : Set Template Id.
        // 1 : Set Field field_5_1
        // 1 : Set test_5_group
        REQUIRE(test_case.encoding(test_5.cref(),"\xF0\x85\x82\xC0\x81\x81\x82\x8B",true));
        REQUIRE(test_case.decoding("\xF0\x85\x82\xC0\x81\x81\x82\x8B",test_5.cref(),true));
    }
}
