// Copyright (c) 2013, 2014, Huang-Ming Huang,  Object Computing, Inc.
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
#include <boost/utility/string_ref.hpp>

namespace mfast {


  template <typename Instruction>
  class string_cref_base
    : public vector_cref_base<char, Instruction>
  {
  public:
    string_cref_base()
    {
    }

    string_cref_base(const string_cref_base& other)
      : vector_cref_base<char, Instruction>(other)
    {
    }

    string_cref_base(const value_storage* storage,
                     const Instruction*   instruction)
      : vector_cref_base<char, Instruction>(storage, instruction)
    {
    }

    explicit string_cref_base(const field_cref& other)
      : vector_cref_base<char, Instruction>(other)
    {
    }

    boost::string_ref value() const
    {
      return boost::string_ref(this->data(), this->size());
    }

    bool operator == (const char* other) const
    {
      return compare(other) == 0;
    }

    bool operator == (const std::string& other) const
    {
      return compare(other) == 0;
    }

    template <typename OtherIntruction>
    bool operator == (const string_cref_base<OtherIntruction>& other) const
    {
      return compare(other) == 0;
    }

    bool operator != (const char* other) const
    {
      return compare(other) != 0;
    }

    bool operator != (const std::string& other) const
    {
      return compare(other) != 0;
    }

    template <typename OtherIntruction>
    bool operator != (const string_cref_base<OtherIntruction>& other) const
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

    template <typename OtherIntruction>
    bool operator > (const string_cref_base<OtherIntruction>& other) const
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

    template <typename OtherIntruction>
    bool operator >=(const string_cref_base<OtherIntruction>& other) const
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

    template <typename OtherIntruction>
    bool operator < (const string_cref_base<OtherIntruction>& other) const
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

    template <typename OtherIntruction>
    bool operator <=(const string_cref_base<OtherIntruction>& other) const
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

    template <typename OtherIntruction>
    int compare(const string_cref_base<OtherIntruction>& other) const
    {
      int result = strncmp(this->data(), other.data(), std::min(this->size(), other.size()));
      if (result != 0) return result;
      return this->size()-other.size();
    }

    const char* c_str() const
    {
      if (this->storage()->of_array.capacity_in_bytes_ > 0) {
        const_cast<char&>(*this->end()) = '\0';
        return this->data();
      }
      return this->data() ? this->data() : "";
    }

  };





  template <>
  class vector_cref<char>
    : public string_cref_base<ascii_field_instruction>
  {
  public:
    vector_cref()
    {
    }

    vector_cref(const vector_cref<char>& other)
      : string_cref_base<ascii_field_instruction>(other)
    {
    }

    vector_cref(const value_storage*           storage,
                const ascii_field_instruction* instruction)
      : string_cref_base<ascii_field_instruction>(storage, instruction)
    {
    }

    explicit vector_cref(const field_cref& other)
      : string_cref_base<ascii_field_instruction>(other)
    {
    }

  };


  struct utf8_char
  {
    char value;
  };

  template <>
  class vector_cref<utf8_char>
    : public string_cref_base<unicode_field_instruction>
  {
  public:
    vector_cref()
    {
    }

    vector_cref(const vector_cref<utf8_char>& other)
      : string_cref_base<unicode_field_instruction>(other)
    {
    }

    vector_cref(const value_storage*             storage,
                const unicode_field_instruction* instruction)
      : string_cref_base<unicode_field_instruction>(storage, instruction)
    {
    }

    explicit vector_cref(const field_cref& other)
      : string_cref_base<unicode_field_instruction>(other)
    {
    }

  };

  template <typename T>
  class string_cref
    : public vector_cref<T>
  {
  public:
    typedef typename vector_cref<T>::instruction_cptr instruction_cptr;
    string_cref()
    {
    }

    string_cref(const string_cref<T>& other)
      : vector_cref<T>(other)
    {
    }

    string_cref(const value_storage* storage,
                instruction_cptr     instruction)
      : vector_cref<T>(storage, instruction)
    {
    }

    explicit string_cref(const field_cref& other)
      : vector_cref<T>(other)
    {
    }

  };

  template <typename T>
  class string_mref_base
    : public vector_mref_base<T>
  {
    typedef vector_mref_base<T> base_type;

  public:
    typedef typename base_type::instruction_cptr instruction_cptr;

    string_mref_base()
      : base_type()
    {
    }

    string_mref_base(mfast::allocator* alloc,
                     value_storage*    storage,
                     instruction_cptr  instruction)
      : base_type(alloc, storage, instruction)
    {
    }

    string_mref_base(const string_mref_base& other)
      : base_type(other)
    {
    }

    explicit string_mref_base(const field_mref_base& other)
      : base_type(other)
    {
    }

    void as (const vector_cref<char>& s)
    {
      if (s.absent())
        this->omit();
      else
        this->assign(s.begin(), s.end());
    }

    void as (const vector_cref<utf8_char>& s)
    {
      if (s.absent())
        this->omit();
      else
        this->assign(s.begin(), s.end());
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

    void swap(const string_mref_base<T>& other) const
    {
      base_type::swap(other);
    }

  };

  template <>
  class vector_mref<char>
    : public string_mref_base<char>
  {
  private:
    typedef string_mref_base<char> base_type;

  public:

    typedef base_type::instruction_cptr instruction_cptr;
    vector_mref()
      : base_type()
    {
    }

    vector_mref(mfast::allocator* alloc,
                value_storage*    storage,
                instruction_cptr  instruction)
      : base_type(alloc, storage, instruction)
    {
    }

    vector_mref(const vector_mref<char>& other)
      : base_type(other)
    {
    }

    explicit vector_mref(const field_mref_base& other)
      : base_type(other)
    {
    }

    vector_mref& operator = (const char* s)
    {
      this->assign(s, s+strlen(s));
      return *this;
    }

    vector_mref& operator = (const std::string& s)
    {
      this->assign(s.begin(), s.end());
      return *this;
    }

    const vector_mref& append (const std::string& str) const
    {
      this->insert(this->end(), str.begin(), str.end());
      return *this;
    }

    const vector_mref& append (const std::string& str, size_t subpos, size_t sublen) const
    {
      this->insert(this->end(), &str[subpos], &str[subpos+sublen]);
      return *this;
    }

    const vector_mref& append (const char* s) const
    {
      return this->append(s, std::strlen(s));
    }

    const vector_mref& append (const char* s, size_t n) const
    {
      this->insert(this->end(), s, s+n);
      return *this;
    }

    const vector_mref& append (size_t n, char c) const
    {
      this->insert(this->end(), n, c);
      return *this;
    }

    template <class InputIterator>
    const vector_mref& append (InputIterator first, InputIterator last) const
    {
      this->insert(this->end(), first, last);
      return *this;
    }

    const vector_mref& operator+= (const std::string& str) const
    {
      return this->append(str);
    }

    const vector_mref& operator+= (const char* s) const
    {
      return this->append(s);
    }

    const vector_mref& operator+= (char c) const
    {
      return this->append(1, c);
    }

  };

  template <>
  class vector_mref<utf8_char>
    : public string_mref_base<utf8_char>
  {
  private:
    typedef string_mref_base<utf8_char> base_type;

  public:
    typedef base_type::instruction_cptr instruction_cptr;

    vector_mref()
      : base_type()
    {
    }

    vector_mref(mfast::allocator* alloc,
                value_storage*    storage,
                instruction_cptr  instruction)
      : base_type(alloc, storage, instruction)
    {
    }

    vector_mref(const vector_mref<utf8_char> &other)
      : base_type(other)
    {
    }

    explicit vector_mref(const field_mref_base& other)
      : base_type(other)
    {
    }

    vector_mref& operator = (const char* s)
    {
      this->assign(s, s+strlen(s));
      return *this;
    }

    vector_mref& operator = (const std::string& s)
    {
      this->assign(s.begin(), s.end());
      return *this;
    }

    const vector_mref& append (const std::string& str) const
    {
      this->insert(this->end(), str.begin(), str.end());
      return *this;
    }

    const vector_mref& append (const std::string& str, size_t subpos, size_t sublen) const
    {
      this->insert(this->end(), &str[subpos], &str[subpos+sublen]);
      return *this;
    }

    const vector_mref& append (const char* s) const
    {
      return this->append(s, std::strlen(s));
    }

    const vector_mref& append (const char* s, size_t n) const
    {
      this->insert(this->end(), s, s+n);
      return *this;
    }

    const vector_mref& append (size_t n, char c) const
    {
      this->insert(this->end(), n, c);
      return *this;
    }

    template <class InputIterator>
    const vector_mref& append (InputIterator first, InputIterator last) const
    {
      this->insert(this->end(), first, last);
      return *this;
    }

    const vector_mref& operator+= (const std::string& str) const
    {
      return this->append(str);
    }

    const vector_mref& operator+= (const char* s) const
    {
      return this->append(s);
    }

    const vector_mref& operator+= (char c) const
    {
      return this->append(1, c);
    }

  };

  template <typename T>
  class string_mref
    : public vector_mref<T>
  {
  public:
    typedef vector_mref<T> base_type;
    typedef typename base_type::instruction_cptr instruction_cptr;
    typedef string_cref<T> cref_type;

    string_mref()
      : base_type()
    {
    }

    string_mref(mfast::allocator* alloc,
                value_storage*    storage,
                instruction_cptr  instruction)
      : base_type(alloc, storage, instruction)
    {
    }

    string_mref(const vector_mref<utf8_char> &other)
      : base_type(other)
    {
    }

    explicit string_mref(const field_mref_base& other)
      : base_type(other)
    {
    }

    operator string_cref<T> () const
    {
      return string_cref<T>(this->storage(), this->instruction());
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
      return this->append(1, c);
    }

  };

  typedef string_cref<char> ascii_string_cref;
  typedef string_cref<utf8_char> unicode_string_cref;
  typedef string_mref<char> ascii_string_mref;
  typedef string_mref<utf8_char> unicode_string_mref;


  template <typename T>
  struct mref_of<string_cref<T> >
  {
    typedef string_mref<T> type;
  };

  typedef vector_type<ascii_string_cref> ascii_string_type;
  typedef vector_type<unicode_string_cref> unicode_string_type;

}

#endif /* end of include guard: STRING_H_KP519AYB */
