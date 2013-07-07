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
#ifndef STRING_H_KP519AYB
#define STRING_H_KP519AYB

#include <string>
#include "mfast/field_ref.h"
#include "mfast/vector_ref.h"

namespace mfast {

template <bool IsAscii>
class string_mref;

template <bool IsAscii>
class string_cref
  : public vector_cref<char, IsAscii>
{
  public:
    typedef typename instruction_trait<char,IsAscii>::type instruction_type;
    typedef const instruction_type* instruction_cptr;

    string_cref()
    {
    }

    string_cref(const string_cref& other)
      : vector_cref<char, IsAscii>(other)
    {
    }

    string_cref(const string_mref<IsAscii>& other)
      : vector_cref<char, IsAscii>(other)
    {
    }

    string_cref(const value_storage* storage,
                instruction_cptr     instruction)
      : vector_cref<char, IsAscii>(storage, instruction)
    {
    }

    std::string value() const
    {
      return std::string(this->data(), this->size());
    }

    bool operator == (const char* other) const
    {
      return compare(other) == 0;
    }

    bool operator == (const std::string& other) const
    {
      return compare(other) == 0;
    }

    bool operator == (const string_cref<true>& other) const
    {
      return compare(other) == 0;
    }

    bool operator == (const string_cref<false>& other) const
    {
      return this->compare(other) == 0;
    }

    bool operator != (const char* other) const
    {
      return compare(other) != 0;
    }

    bool operator != (const std::string& other) const
    {
      return compare(other) != 0;
    }

    bool operator != (const string_cref<true>& other) const
    {
      return compare(other) != 0;
    }

    bool operator != (const string_cref<false>& other) const
    {
      return compare(other) != 0;
    }

    bool operator > (const char* other) const
    {
      return compare(other) > 0;
    }

    bool operator > (const std::string& other) const
    {
      return compare(other) > 0;
    }

    bool operator > (const string_cref<true>& other) const
    {
      return compare(other) >= 0;
    }

    bool operator > (const string_cref<false>& other) const
    {
      return compare(other) >= 0;
    }

    bool operator >= (const char* other) const
    {
      return compare(other) >= 0;
    }

    bool operator >= (const std::string& other) const
    {
      return compare(other) >= 0;
    }

    bool operator >=(const string_cref<true>& other) const
    {
      return compare(other) >= 0;
    }

    bool operator >=(const string_cref<false>& other) const
    {
      return compare(other) >= 0;
    }

    bool operator < (const char* other) const
    {
      return compare(other) < 0;
    }

    bool operator < (const std::string& other) const
    {
      return compare(other) < 0;
    }

    bool operator < (const string_cref<true>& other) const
    {
      return compare(other) < 0;
    }

    bool operator < (const string_cref<false>& other) const
    {
      return compare(other) < 0;
    }

    bool operator <= (const char* other) const
    {
      return compare(other) <= 0;
    }

    bool operator <= (const std::string& other) const
    {
      return compare(other) <= 0;
    }

    bool operator <=(const string_cref<true>& other) const
    {
      return compare(other) <= 0;
    }

    bool operator <=(const string_cref<false>& other) const
    {
      return compare(other) <= 0;
    }

    int compare(const char* other) const
    {
      int result = strncmp(this->data(), other, this->size());
      if (result != 0 ) return result;
      if (other[this->size()] == '\0') return 0;
      return -1;
    }

    int compare(const std::string& other) const
    {
      return -other.compare(0, other.size(), this->data(), this->size());
    }

    int compare(const string_cref<true>& other) const
    {
      int result = strncmp(this->data(), other.data(), std::min(this->size(), other.size()));
      if (result != 0) return result;
      return this->size()-other.size();
    }

    int compare(const string_cref<false>& other) const
    {
      int result = strncmp(this->data(), other.data(), std::min(this->size(), other.size()));
      if (result != 0) return result;
      return this->size()-other.size();
    }

    instruction_cptr instruction() const
    {
      return static_cast<instruction_cptr>(this->instruction_);
    }

    const char* c_str() const
    {
      if (this->storage()->of_array.capacity_ > 0) {
        const_cast<char&>(*this->end()) = '\0';
      }
      return this->data();
    }

};

typedef string_cref<true> ascii_string_cref;
typedef string_cref<false> unicode_string_cref;

template <bool IsAscii>
class string_mref
  : public make_vector_mref<string_cref<IsAscii> >
{
  typedef make_vector_mref<string_cref<IsAscii> > base_type;

  public:
    typedef typename base_type::instruction_cptr instruction_cptr;

    string_mref()
      : base_type()
    {
    }

    string_mref(allocator*       alloc,
                value_storage*   storage,
                instruction_cptr instruction)
      : base_type(alloc, storage, instruction)
    {
    }

    string_mref(const string_mref& other)
      : base_type(other)
    {
    }

    string_mref& operator = (const char* s)
    {
      this->assign(s, s+strlen(s));
      return *this;
    }

    string_mref& operator = (const std::string& s)
    {
      this->assign(s.begin(), s.end());
      return *this;
    }

    void as (const char* s) const
    {
      this->assign(s, s+strlen(s));
    }

    void as (const std::string& s) const
    {
      this->assign(s.begin(), s.end());
    }

    void shallow_assign (const char* str) const
    {
      base_type::shallow_assign(str, std::strlen(str));
    }

    const string_mref& append (const std::string& str) const
    {
      this->insert(this->end(), str.begin(), str.end());
      return *this;
    }

    const string_mref& append (const std::string& str, size_t subpos, size_t sublen) const
    {
      this->insert(this->end(), &str[subpos], &str[subpos+sublen]);
      return *this;
    }

    const string_mref& append (const char* s) const
    {
      return this->append(s, std::strlen(s));
    }

    const string_mref& append (const char* s, size_t n) const
    {
      this->insert(this->end(), s, s+n);
      return *this;
    }

    const string_mref& append (size_t n, char c) const
    {
      this->insert(this->end(), n, c);
      return *this;
    }

    template <class InputIterator>
    const string_mref& append (InputIterator first, InputIterator last) const
    {
      this->insert(this->end(), first, last);
      return *this;
    }

    const string_mref& operator+= (const std::string& str) const
    {
      return this->append(str);
    }

    const string_mref& operator+= (const char* s) const
    {
      return this->append(s);
    }

    const string_mref& operator+= (char c) const
    {
      return this->append(c);
    }

  protected:
    explicit string_mref(const field_mref& other)
      : base_type(other)
    {
    }

};

typedef string_mref<true> ascii_string_mref;
typedef string_mref<false> unicode_string_mref;


}

#endif /* end of include guard: STRING_H_KP519AYB */
