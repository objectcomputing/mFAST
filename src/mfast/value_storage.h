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

#include "mfast/mfast_export.h"
#include <stdint.h>
#include <cstring>


namespace mfast 
{
  class template_instruction;

  union MFAST_EXPORT value_storage {
    struct {
      uint32_t present_;                ///< indicate if the value is present,
      uint32_t padding_ : 31;
      uint32_t defined_bit_ : 1;
      uint64_t content_;
    } of_uint;

    struct {
      uint32_t present_;                ///< indicate if the value is present,
      int8_t exponent_;
      int8_t padding1_;
      uint16_t padding2_ : 15;
      uint16_t defined_bit_ : 1;
      int64_t mantissa_;
    } of_decimal;

    struct {
      uint32_t present_;                ///< indicate if the value is present,
      uint32_t own_content_ : 1;          ///< indicate if \a content_ should be deallocated
      uint32_t padding_ : 30;
      uint32_t defined_bit_ : 1;
      value_storage* content_;
    } of_group; ///< used for group or template

    struct {
      uint32_t len_; ///< the length+1 of content; it represents null value or content is absent when len==0.
                     /// In the case of empty string, len == 1 and content_[0]= '\0'.
      uint32_t capacity_ : 31; ///< used to track the length of memory that has been reserved
                               ///< for \a content_. if <tt>capacity_ == 0</tt> and <tt>len_ > 0</tt>,
                               ///< it means the object does not own the memory in \a content_.
      uint32_t defined_bit_ : 1; ///< used by FAST coder/encoder/decoder for tracking if a dictionary
                                 ///< value is defined or not.
      void* content_;
    } of_array;

    struct
    {
      union {
        const template_instruction* instruction_;
        uint64_t dummy_; // make sure content_ and instruction_ won't be packed together in 32 bits environment
      } of_instruction;

      value_storage* content_;
    } of_templateref;


    // construct an undefined value
    value_storage()
    {
      of_templateref.content_ = 0;
      of_templateref.of_instruction.dummy_ = 0;
    };

    // construct a default numeric value
    value_storage(int)
    {
      of_uint.content_ = 0;
      of_uint.padding_ = 0;
      of_uint.defined_bit_ = 1;
      of_uint.present_ = 1;
    };
    
    // construct a default zero length string value
    value_storage(const char*)
    {
      of_array.content_ = const_cast<char*>("");
      of_array.len_ = 1;
      of_array.capacity_ = 0;
      of_array.defined_bit_ = 1;  
    };

    bool is_defined() const
    {
      return of_array.defined_bit_;
    }

    void defined(bool v)
    {
      of_array.defined_bit_ = v;
    }

    bool is_empty() const
    {
      return of_array.len_ == 0;
    }

    void present(bool p)
    {
      of_array.len_ = p;
    }

    uint32_t array_length() const
    {
      return of_array.len_ == 0 ? 0 : of_array.len_ -1;
    }

    void array_length(uint32_t n)
    {
      of_array.len_ = n+1;
    }
    
    template <typename T>
    T get() const 
    {
      return reinterpret_cast<const T&>(of_uint.content_);
    }
    
    template <typename T>
    void set(T v) 
    {
      reinterpret_cast<T&>(of_uint.content_) = v;
    }
  };
  

  template <typename IntType>
  struct int_value_storage
  {
    value_storage storage_;
    
    int_value_storage(){
      storage_.of_uint.defined_bit_ = 1;
    }
    int_value_storage(IntType v)
    {
      storage_.of_uint.defined_bit_ = 1;
      storage_.of_uint.present_ = 1;
      
      storage_.set<IntType>(v);
    }
  };
  
  struct decimal_value_storage
  {
    value_storage storage_;
    
    decimal_value_storage(){
      storage_.of_decimal.defined_bit_ = 1;
    }
    decimal_value_storage(int64_t mantissa, uint8_t exponent)
    {
      storage_.of_decimal.defined_bit_ = 1;
      storage_.of_decimal.present_ = 1;
      storage_.of_decimal.mantissa_ = mantissa;
      storage_.of_decimal.exponent_ = exponent;
    }
  };
  
  struct string_value_storage
  {
    value_storage storage_;
    
    string_value_storage(){
      storage_.of_array.defined_bit_ = 1;
    }
    
    string_value_storage(const char* v)
    {
      storage_.of_array.defined_bit_ = 1;
      storage_.of_array.len_ = std::strlen(v) +1;
      storage_.of_array.content_ = const_cast<char*>(v);
      storage_.of_array.capacity_ = 0;
    }
    
    string_value_storage(const char* v, std::size_t n)
    {
      storage_.of_array.defined_bit_ = 1;
      storage_.of_array.len_ = n+1;
      storage_.of_array.content_ = const_cast<char*>(v);
      storage_.of_array.capacity_ = 0;
    }
  };
  
  struct byte_vector_value_storage
    : string_value_storage
  {
    
    byte_vector_value_storage(){}
    byte_vector_value_storage(const unsigned char* v, std::size_t n)
      : string_value_storage(reinterpret_cast<const char*>(v), n)
    {
    }
  };
  
}

#endif /* end of include guard: VALUE_STORAGE_H_OMNNMOZX */
