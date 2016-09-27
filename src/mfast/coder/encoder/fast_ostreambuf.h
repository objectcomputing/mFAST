// Copyright (c) 2016, Huang-Ming Huang,  Object Computing, Inc.
// All rights reserved.
//
// This file is part of mFAST.
// See the file license.txt for licensing information.
#pragma once

#include <stdexcept>
#include "../mfast_coder_export.h"
#include "../../exceptions.h"

namespace mfast {
class buffer_overflow_error : public virtual boost::exception,
                              public virtual std::runtime_error {
public:
  buffer_overflow_error() : std::runtime_error("buffer overflow") {}
};

class MFAST_CODER_EXPORT fast_ostreambuf {
public:
  fast_ostreambuf(char *buf, std::size_t size);

  template <std::size_t SIZE> fast_ostreambuf(char(&array)[SIZE]);

  void sputc(char c);
  void sputn(const char *data, std::size_t n);
  void skip(std::size_t n);

  virtual std::size_t length() const;
  virtual void write_bytes_at(const char *data, std::size_t n,
                              std::size_t offset, bool shrink);

  const char *pbase() const { return pbase_; }

protected:
  virtual void overflow(std::size_t n);
  void setp(char *pbase, char *pptr, char *epptr);
  char *pbase_, *pptr_, *epptr_;
};

inline fast_ostreambuf::fast_ostreambuf(char *buf, std::size_t size)
    : pbase_(buf), pptr_(buf), epptr_(buf + size) {}

template <std::size_t SIZE>
fast_ostreambuf::fast_ostreambuf(char(&array)[SIZE])
    : pbase_(array), pptr_(array), epptr_(array + SIZE) {}

inline void fast_ostreambuf::sputc(char c) {
  while (pptr_ >= epptr_)
    overflow(1);

  *pptr_ = c;
  ++pptr_;
}

inline void fast_ostreambuf::sputn(const char *data, std::size_t n) {
  while (pptr_ + n >= epptr_)
    overflow(n);

  std::copy(data, data + n, pptr_);
  pptr_ += n;
}

inline void fast_ostreambuf::skip(std::size_t n) {
  while (pptr_ + n >= epptr_)
    overflow(n);
  pptr_ += n;
}

inline void fast_ostreambuf::setp(char *pbase, char *pptr, char *epptr) {
  pbase_ = pbase;
  pptr_ = pptr;
  epptr_ = epptr;
}
}
