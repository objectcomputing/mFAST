#include "catch.hpp"
#include <mfast.h>
#include <mfast/field_comparator.h>
#include <mfast/coder/fast_encoder_v2.h>
#include <mfast/coder/fast_decoder_v2.h>
#include <cstring>
#include <stdexcept>
#include <mfast/json/json.h>

#include "simple6.h"
#include "scp.h"

#include "byte_stream.h"
#include "debug_allocator.h"

using namespace mfast;



TEST_CASE("test fast decoding of a template with scp reset", "[scp_reset_test]")
{
  fast_decoder_v2<0> decoder1(simple6::description(), scp::description());
  auto& strm="\xF8\x81\x81\x82\x83\x80\xC0\xF8\xC0\x81";
  const char* first = std::begin(strm);
  const char* last = std::end(strm);

  simple6::Test b1(R"({"field1":1, "field2":2, "field3":3})");
  REQUIRE( decoder1.decode(first, last) == b1.cref() );
  REQUIRE( decoder1.decode(first, last) == b1.cref() );
  REQUIRE( decoder1.decode(first, last).id() == 120 );

  simple6::Test b2(R"({"field1":11, "field2":12, "field3":13})");
  REQUIRE( decoder1.decode(first, last) == b2.cref() );

  debug_allocator alloc;
  fast_decoder_v2<0> decoder2(&alloc, simple6::description(), scp::description());
  first = std::begin(strm);
  decoder2.decode(first, last);
}
