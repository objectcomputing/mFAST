#include "catch.hpp"
#include <mfast.h>

#include "fast_test_coding_case_v2.hpp"

#include "simple15.h"

using namespace test::coding;

TEST_CASE("full pmap occupied encoder_v2/decoder_v2","[full_pmap_occupied_encoder_v2_decoder_v2]")
{
    fast_test_coding_case_v2<simple15::templates_description> test_case;

    SECTION("encode fields")
    {
        simple15::Test_1 test_1;
        simple15::Test_1_mref test_1_mref = test_1.mref();
        test_1_mref.set_field_1_1().as(1);
        test_1_mref.set_field_1_2().as(2);
        test_1_mref.set_field_1_3().as(3);
        test_1_mref.set_field_1_4().as(4);
        test_1_mref.set_field_1_5().as(5);
        test_1_mref.set_field_1_6().as(6);
        test_1_mref.set_field_1_7().as(7);
        REQUIRE(test_case.encoding(test_1.cref(),"\x3F\xC0\x82\x83\x84\x85\x86\x87\x88",true));
        REQUIRE(test_case.decoding("\x3F\xC0\x82\x83\x84\x85\x86\x87\x88",test_1.cref(),true));
    }
}
