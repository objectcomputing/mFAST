#include <boost/test/test_tools.hpp>
#include <boost/test/unit_test.hpp>

#include <mfast.h>
#include <mfast/field_comparator.h>
#include <mfast/coder/fast_encoder_v2.h>
#include <mfast/coder/fast_decoder_v2.h>
#include <cstring>
#include <stdexcept>


#include "simple6.h"
#include "scp.h"

#include "byte_stream.h"
#include "debug_allocator.h"

using namespace mfast;



BOOST_AUTO_TEST_CASE(scp_reset_test)
{
  fast_decoder_v2<0> decoder1(simple6::description(), scp::description());
  byte_stream strm("\xB8\x81\x82\x82\x39\x45\xA3\x41\x42\xc3\x84\x41\x42\x43");
  const char* start = strm.data();

  decoder1.decode(start, start+strm.size());


  debug_allocator alloc;
  fast_decoder_v2<0> decoder2(&alloc, simple6::description(), scp::description());
  decoder2.decode(start, start+strm.size());
}
