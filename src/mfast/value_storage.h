// Copyright (c) 2013, Huang-Ming Huang,  Object Computing, Inc.
// All rights reserved.
//
// This file is part of mFAST.
//
//     mFAST is free software: you can redistribute it and/or modify
//     it under the terms of the GNU Lesser General Public License as published by
//     the Free Software Foundation, either version 3 of the License, or
//     (at your option) any later version.
//
//     mFAST is distributed in the hope that it will be useful,
//     but WITHOUT ANY WARRANTY; without even the implied warranty of
//     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//     GNU General Public License for more details.
//
//     You should have received a copy of the GNU Lesser General Public License
//     along with mFast.  If not, see <http://www.gnu.org/licenses/>.
//

#ifndef VALUE_STORAGE_H_OMNNMOZX
#define VALUE_STORAGE_H_OMNNMOZX

#include <stdint.h>

namespace mfast 
{
  class template_instruction;

  union value_storage_t {
    struct {
      uint32_t present_;                ///< indicate if the value is present,
      uint32_t padding_ : 31;
      uint32_t defined_bit_ : 1;
      uint64_t content_;
    } uint_storage;

    struct {
      uint32_t present_;                ///< indicate if the value is present,
      int16_t exponent_;
      uint16_t padding_ : 15;
      uint16_t defined_bit_ : 1;
      int64_t mantissa_;
    } decimal_storage;

    struct {
      uint32_t present_;                ///< indicate if the value is present,
      uint32_t own_content_ : 1;          ///< indicate if \a content_ should be deallocated
      uint32_t padding_ : 30;
      uint32_t defined_bit_ : 1;
      value_storage_t* content_;
    } group_storage; ///< used for group or template

    struct {
      uint32_t len_; ///< the length+1 of content; it represents null value or content is absent when len==0.
                     /// In the case of empty string, len == 1 and content_[0]= '\0'.
      uint32_t capacity_ : 31; ///< used to track the length of memory that has been reserved
                               ///< for \a content_. if <tt>capacity_ == 0</tt> and <tt>len_ > 0</tt>,
                               ///< it means the object does not own the memory in \a content_.
      uint32_t defined_bit_ : 1; ///< used by FAST encoder/decoder for tracking if a dictionary
                                 ///< value is defined or not.
      void* content_;
    } array_storage;

    struct
    {
      union {
        const template_instruction* instruction_;
        uint64_t dummy; // make sure content_ and instruction_ won't be packed together in 32 bits environment
      } instruction_storage;

      value_storage_t* content_;
    } templateref_storage;


    // construct an undefined value
    value_storage_t()
    {
      uint_storage.content_ = 0;
      uint_storage.padding_ = 0;
      uint_storage.defined_bit_ = 0;
      uint_storage.present_ = 0;
    };

    // construct a defined non-empty value
    value_storage_t(int)
    {
      uint_storage.content_ = 0;
      uint_storage.padding_ = 0;
      uint_storage.defined_bit_ = 1;
      uint_storage.present_ = 1;
    };

    bool is_defined() const
    {
      return array_storage.defined_bit_;
    }

    void defined(bool v)
    {
      array_storage.defined_bit_ = v;
    }

    bool is_empty() const
    {
      return array_storage.len_ == 0;
    }

    void present(bool p)
    {
      array_storage.len_ = p;
    }

    uint32_t array_length() const
    {
      return array_storage.len_ == 0 ? 0 : array_storage.len_ -1;
    }

    void array_length(uint32_t n)
    {
      array_storage.len_ = n+1;
    }
  };
  
}

#endif /* end of include guard: VALUE_STORAGE_H_OMNNMOZX */
