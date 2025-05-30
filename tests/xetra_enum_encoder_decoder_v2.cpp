#include "catch.hpp"
#include <mfast.h>

#include "fast_test_coding_case_v2.hpp"
#include "byte_stream.h"

#include "simple18.h"

using namespace test::coding;

TEST_CASE("xetra enum test encoder_V2/decoder_v2","[xetra_enum_encoder_v2_decoder_v2]")
{
    SECTION("No optional enum / Active")
    {
        fast_test_coding_case_v2<simple18::templates_description> test_case;
        simple18::Test_1 test_1;
        simple18::Test_1_mref test_1_mref = test_1.mref();
        test_1_mref.set_MDStatisticStatus().as_Active();
        REQUIRE(test_case.encoding(test_1.cref(),"\xc0\x81\x80",true));
        REQUIRE(test_case.decoding("\xc0\x81\x80",test_1.cref(),true));
    }

    SECTION("No optional enum / Inactive")
    {
        fast_test_coding_case_v2<simple18::templates_description> test_case;
        simple18::Test_1 test_1;
        simple18::Test_1_mref test_1_mref = test_1.mref();
        test_1_mref.set_MDStatisticStatus().as_Inactive();
        REQUIRE(test_case.encoding(test_1.cref(),"\xe0\x81\x81\x80",true));
        REQUIRE(test_case.decoding("\xe0\x81\x81\x80",test_1.cref(),true));
    }

    SECTION("Optional enum")
    {
        fast_test_coding_case_v2<simple18::templates_description> test_case;
        simple18::Test_1 test_1;
        simple18::Test_1_mref test_1_mref = test_1.mref();
        test_1_mref.set_MDStatisticStatus().as_Inactive();
        test_1_mref.set_MDStatisticFrequencyUnit().as_Minutes();
        REQUIRE(test_case.encoding(test_1.cref(),"\xe0\x81\x81\x83",true));
        REQUIRE(test_case.decoding("\xe0\x81\x81\x83",test_1.cref(),true));
    }
}