// Copyright (c) 2016, Huang-Ming Huang,  Object Computing, Inc.
// All rights reserved.
//
// This file is part of mFAST.
// See the file license.txt for licensing information.
#include "field_ref.h"

namespace mfast {
namespace detail {
const value_storage null_storage;
}

bad_optional_access::bad_optional_access()
    : std::logic_error("Attempted to access the value of an uninitialized "
                       "optional mfast object.") {}
}
