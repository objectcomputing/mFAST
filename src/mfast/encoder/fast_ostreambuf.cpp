
#include <cstring>
#include <algorithm>
#include "fast_ostreambuf.h"

namespace mfast {


void fast_ostreambuf::overflow(std::size_t)
{
  throw buffer_overflow_error();
}

std::size_t
fast_ostreambuf::length() const
{
  return pptr_ - pbase_;
}

void 
fast_ostreambuf::write_bytes_at(const char* data, std::size_t n, std::size_t offset)
{
  assert ( (pbase_ + offset +n) <= pptr_);
  std::copy(data, data+n, pbase_);
}

void 
fast_ostreambuf::shrink(std::size_t offset, std::size_t nbytes)
{
  assert ( (pbase_ + offset + nbytes) <= pptr_);
  std::memmove(pbase_, pbase_ + nbytes, pptr_- (pbase_ + offset + nbytes) );
  pptr_ -= nbytes;
}

}
