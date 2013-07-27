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
#ifndef BYTE_VECTOR_H_RIH6NA7X
#define BYTE_VECTOR_H_RIH6NA7X
#include <cassert>
#include <iterator>
#include <cstring>
#include "mfast/field_ref.h"
#include "mfast/allocator.h"
namespace mfast {

namespace detail {
class codec_helper;
}

template <typename T, bool NotUnicode=true>
class vector_cref
  : public field_cref
{
  public:
    enum {
      is_unicode_string = !NotUnicode
    };

    typedef T value_type;
    typedef std::size_t size_t;
    typedef const T* iterator;
    typedef iterator const_iterator;
    typedef typename instruction_trait<T,NotUnicode>::type instruction_type;
    typedef const instruction_type* instruction_cptr;
    typedef std::reverse_iterator<iterator> reverse_iterator;
    typedef std::reverse_iterator<const_iterator> const_reverse_iterator;

    vector_cref()
    {
    }

    vector_cref(const value_storage* storage,
                instruction_cptr     instruction)
      : field_cref(storage, instruction)
    {
    }

    vector_cref(const vector_cref& other)
      : field_cref(other)
    {
    }

    uint32_t id() const
    {
      return instruction_->id();
    }

    /**
     * Returns a pointer to an array that contains the same sequence of characters
     * as the characters that make up the value of the string object.
     *
     * Accessing the value at data()+size() produces undefined behavior: There are no
     * guarantees that a null character terminates the character sequence pointed by the
     * value returned by this function.
     **/
    const T* data() const
    {
      return static_cast<const T*>(storage_->of_array.content_);
    }

    size_t size() const
    {
      return storage_->array_length();
    }

    size_t length() const
    {
      return size();
    }
    
    bool is_initial_value() const
    {
      return *this == vector_cref<T,NotUnicode>(&this->instruction()->default_value(), instruction());
    }

    const_iterator begin() const
    {
      return data();
    }

    const_iterator end() const
    {
      return data() + size();
    }
    
    const_reverse_iterator rbegin() const {
      return const_reverse_iterator(end());
    }
    
    const_reverse_iterator rend() const {
      return const_reverse_iterator(begin());
    }
    
    const_iterator cbegin() const
    {
      return data();
    }

    const_iterator cend() const
    {
      return data() + size();
    }
    
    const_reverse_iterator crbegin() const {
      return const_reverse_iterator(cend());
    }
    
    const_reverse_iterator crend() const {
      return const_reverse_iterator(cbegin());
    }

    T operator [] (std::size_t index) const
    {
      assert(index < size());
      return data()[index];
    }

    instruction_cptr instruction() const
    {
      return static_cast<instruction_cptr>(field_cref::instruction());
    }

  protected:
    explicit vector_cref(const field_cref& other)
      : field_cref(other)
    {
    }
    
    friend class mfast::detail::codec_helper;
    
    void save_to(value_storage& v) const
    {
      v.of_array.content_ = this->storage()->of_array.content_;
      v.of_array.len_ = this->storage()->of_array.len_;
      v.of_array.capacity_ = 0;
      v.defined(true);
    }

  private:
    vector_cref& operator = (const vector_cref&);
};

template <typename T, bool NotUnicode>
bool operator == (const vector_cref<T,NotUnicode>& lhs,
                  const vector_cref<T,NotUnicode>& rhs)
{
  if (lhs.size() != rhs.size()) return false;
  if (lhs.data() == rhs.data()) return true;
  return std::memcmp(lhs.data(), rhs.data(), lhs.size()) == 0;
}

template <typename T, bool NotUnicode>
bool operator != (const vector_cref<T,NotUnicode>& lhs,
                  const vector_cref<T,NotUnicode>& rhs)
{
  return !(lhs == rhs);
}


typedef vector_cref<unsigned char> byte_vector_cref;




template <typename ConstVectorRef>
class make_vector_mref
  : public make_field_mref<ConstVectorRef>
{
  typedef make_field_mref<ConstVectorRef> base_type;

  public:
    typedef std::size_t size_t;
    typedef typename ConstVectorRef::value_type value_type;
    typedef value_type* iterator;
    typedef typename ConstVectorRef::instruction_cptr instruction_cptr;
    typedef std::reverse_iterator<iterator> reverse_iterator;
    
    make_vector_mref()
    {
    }

    make_vector_mref(allocator*       alloc,
                     value_storage*   storage,
                     instruction_cptr instruction)
      : base_type(alloc, storage, instruction)
    {
    }

    make_vector_mref(const make_vector_mref& other)
      : base_type(other)
    {
    }

    void as_initial_value() const
    {
      copy_from(this->instruction()->default_value());
    }
    
    void as (const ConstVectorRef& cref)
    {
      if (cref.absent()) {
        this->as_absent();
      }
      else {
        assign(cref.begin(), cref.end());
      }
    }

    value_type* data() const
    {
      return static_cast<value_type*>(this->storage()->of_array.content_);
    }

    iterator begin() const
    {
      return data();
    }

    iterator end() const
    {
      return data() + this->size();
    }
    
    reverse_iterator rbegin() const {
      return reverse_iterator(end());
    }
    
    reverse_iterator rend() const {
      return reverse_iterator(begin());
    }

    value_type& operator [] (size_t index) const
    {
      assert(index < this->size() && capacity() != 0);
      return data()[index];
    }

    void resize(size_t n, char c = 0) const
    {
      assert( n < INT32_MAX);
      size_t len = n+1;
      if ( len > this->capacity()) {
        reserve(n);
        int bytes_to_set = n - this->size();
        if (bytes_to_set > 0)
          std::memset(end(), c, bytes_to_set);
      }
      this->storage()->array_length(n);
    }

    void reserve(size_t n) const;

    void push_back(char c) const
    {
      insert(end(), c);
    }

    iterator insert (iterator position, value_type val) const
    {
      iterator itr = this->shift(position, 1);
      *itr = val;
      return itr;
    }

    void insert (iterator position, size_t n, value_type val) const
    {
      assert(position >= begin() && position <= end());
      std::memset(this->shift(position, n), val, n*sizeof(value_type));
    }

    template <class InputIterator>
    void insert (iterator position, InputIterator first, InputIterator last) const
    {
      assert(position >= begin() && position <= end());
      typedef typename std::iterator_traits<InputIterator>::iterator_category category;
      this->insert(position, first, last, category());
    }

    template <class InputIterator>
    void assign (InputIterator first, InputIterator last) const
    {
      resize(0);
      insert(begin(), first, last);
    }

    void assign (size_t n, value_type val) const
    {
      resize(n);
      memcpy(begin(), val, n*sizeof(value_type));
    }

    void shallow_assign(const value_type* addr, size_t n) const
    {
      assert( n < INT32_MAX );
      this->storage()->of_array.content_ = const_cast<char*>(addr);
      this->storage()->array_length(n);
      this->storage()->of_array.capacity_ = 0;
    }

    void replace(size_t pos, size_t count,
                 const value_type* addr, size_t count2 ) const;

  protected:
    explicit make_vector_mref(const field_mref& other)
      : base_type(other)
    {
    }

    friend class mfast::detail::codec_helper;
    void copy_from(const value_storage& v) const
    {
      if (this->capacity()==0 || this->storage()->of_array.content_ != v.of_array.content_) {
        const char* ptr = static_cast<const char*>(v.of_array.content_);
        this->assign(ptr,
                     ptr+ v.array_length());
      }
    }



  private:
    make_vector_mref& operator = (const make_vector_mref&);

    template <class RandomAccessIterator>
    void insert (iterator             position,
                 RandomAccessIterator first,
                 RandomAccessIterator last,
                 std::                random_access_iterator_tag) const
    {
      size_t n = std::distance(first, last);
      std::copy(first, last, this->shift(position, n));
    }

    template <class InputIterator>
    void insert (iterator position, InputIterator first, InputIterator last,
                 std::input_iterator_tag) const
    {
      for (; first != last; ++first) {
        position = insert(position, *first);
        ++position;
      }
    }

    // The reason I decide to hide capacity() is because it may return a value
    // which is less than size() when the data() is not allocated by @c alloc_.
    // This behavior is different from the @c std::vector::capacity().
    size_t capacity() const
    {
      return this->storage()->of_array.capacity_;
    }

    iterator shift(iterator position, size_t n) const;
};


typedef make_vector_mref<byte_vector_cref> byte_vector_mref;

template <>
struct mref_of<byte_vector_cref>
{
  typedef byte_vector_mref type;
};


template <typename ConstVectorRef>
void
make_vector_mref<ConstVectorRef>::reserve(typename make_vector_mref<ConstVectorRef>::size_t n)
const
{
  if (capacity() > n)
    return;
  std::size_t reserve_size = (n+1);

  if (capacity() > 0) {
    this->storage()->of_array.capacity_
      = this->alloc_->reallocate(this->storage()->of_array.content_, capacity(), reserve_size);
  }
  else {
    void* old_addr = this->storage()->of_array.content_;
    this->storage()->of_array.content_ = 0;
    this->storage()->of_array.capacity_
      = this->alloc_->reallocate(this->storage()->of_array.content_, 0, reserve_size);
    // Copy the old content to the new buffer.
    // In the case when the this->capacity == 0 && this->size() > 0,
    // reserve() could be invoked with n < this->size(). Thus, we can
    // only copy min(size(), n) elements to the new buffer.
    if (this->storage()->array_length() > 0 && n > 0) {
      memcpy(this->storage()->of_array.content_,
             old_addr,
             std::min(this->size(), n) );
    }
  }

}

template <typename ConstVectorRef>
typename make_vector_mref<ConstVectorRef>::iterator
make_vector_mref<ConstVectorRef>::shift(
  typename make_vector_mref<ConstVectorRef>::iterator position,
  typename make_vector_mref<ConstVectorRef>::size_t   n)
const
{
  assert(position >= begin() && position <= end());
  std::size_t offset = position  - begin();

  if (capacity() < this->size()+n)
    reserve(this->size()+n);


  iterator src = begin()+offset;

  if (offset < this->size()) {
    std::memmove(src+(n*sizeof(value_type)), src, (this->size()+1-offset) * sizeof(value_type));
  }
  this->storage()->of_array.len_ += n;
  return src;
}

template <typename ConstVectorRef>
void make_vector_mref<ConstVectorRef>::replace(size_t            pos,
                                               size_t            count,
                                               const value_type* addr,
                                               size_t            count2 ) const
{
  if (pos+count >= this->size()) {
    // all the elements starting from pos are to removed from the vector
    this->resize(pos+count2);
  }
  else {
    int diff = count2-count;
    if (diff > 0) {
      // some elements has to be shift back
      this->shift(data()+pos, diff);
    }
    else {
      memmove(data()+ pos + count2, data() + pos + count, -diff);
      this->resize(this->size() + diff);
    }
  }
  memcpy(data()+pos, addr, count2*sizeof(value_type));
}

}

#endif /* end of include guard: BYTE_VECTOR_H_RIH6NA7X */
