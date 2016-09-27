// Copyright (c) 2016, Huang-Ming Huang,  Object Computing, Inc.
// All rights reserved.
//
// This file is part of mFAST.
// See the file license.txt for licensing information.
#pragma once

#define MFAST_JSON_H

#include "mfast_json_export.h"
#include "../../mfast.h"
#include <iostream>

namespace mfast {
namespace json {
MFAST_JSON_EXPORT bool encode(std::ostream &os,
                              const ::mfast::aggregate_cref &msg,
                              unsigned json_object_tag_mask = 0);

MFAST_JSON_EXPORT bool encode(std::ostream &os,
                              const ::mfast::aggregate_cref &msg,
                              unsigned json_object_tag_mask,
                              unsigned ignore_tag_mask);

MFAST_JSON_EXPORT bool encode(std::ostream &is,
                              const ::mfast::sequence_cref &seq,
                              unsigned json_object_tag_mask = 0);
MFAST_JSON_EXPORT void decode(std::istream &is,
                              const ::mfast::aggregate_mref &msg,
                              unsigned json_object_tag_mask = 0);
MFAST_JSON_EXPORT void decode(std::istream &is,
                              const ::mfast::sequence_mref &seq,
                              unsigned json_object_tag_mask = 0);
} // namespace json
} // namespace mfast
