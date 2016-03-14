// Copyright (c) 2016, Huang-Ming Huang,  Object Computing, Inc.
// All rights reserved.
//
// This file is part of mFAST.
// See the file license.txt for licensing information.
#pragma once
#include "mfast_export.h"
#include <boost/exception/all.hpp>

#ifdef BOOST_MSVC
#pragma warning(push)
#pragma warning(disable : 4275)
#endif

namespace mfast {
// we should always export exception classes; otherwise, the vtable won't
// be available for application when they are in shared libraries.

class MFAST_EXPORT fast_error : public virtual boost::exception,
                                public virtual std::exception {
public:
  fast_error() {}
  fast_error(const char *error_code);
};

class MFAST_EXPORT fast_static_error : public fast_error {
public:
  fast_static_error(){};
  fast_static_error(const char *error_code) : fast_error(error_code) {}
};

class MFAST_EXPORT fast_dynamic_error : public fast_error {
public:
  fast_dynamic_error(const char *error_code) : fast_error(error_code) {}
};

class MFAST_EXPORT fast_reportable_error : public fast_error {
public:
  fast_reportable_error(const char *error_code) : fast_error(error_code) {}
};
}

#ifdef BOOST_MSVC
#pragma warning(pop)
#endif
