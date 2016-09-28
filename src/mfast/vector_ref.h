// Copyright (c) 2016, Huang-Ming Huang,  Object Computing, Inc.
// All rights reserved.
//
// This file is part of mFAST.
// See the file license.txt for licensing information.
#pragma once
#include <cassert>
#include <iterator>
#include <cstring>
#include <vector>
#include <limits>
#include "field_ref.h"
#include "allocator.h"
#include "type_category.h"

namespace mfast {
namespace detail {
class codec_helper;
}

template <typename T> struct vector_instruction_trait {
  typedef vector_field_instruction<T> type;
};

template <> struct vector_instruction_trait<unsigned char> {
  typedef byte_vector_field_instruction type;
};

template <typename T, typename InstructionType>
class vector_cref_base : public field_cref {
public:
  typedef T value_type;
  typedef std::size_t size_t;
  typedef const T *iterator;
  typedef iterator const_iterator;
  typedef const T &const_reference;
  typedef InstructionType instruction_type;
  typedef const instruction_type *instruction_cptr;
  typedef std::reverse_iterator<iterator> reverse_iterator;
  typedef std::reverse_iterator<const_iterator> const_reverse_iterator;

  vector_cref_base() {}
  vector_cref_base(const value_storage *storage, instruction_cptr instruction)
      : field_cref(storage, instruction) {}

  explicit vector_cref_base(const field_cref &other) : field_cref(other) {}
  uint32_t id() const { return instruction_->id(); }
  /**
   * Returns a pointer to an array that contains the same sequence of characters
   * as the characters that make up the value of the string object.
   *
   * Accessing the value at data()+size() produces undefined behavior: There are
   *no
   * guarantees that a null character terminates the character sequence pointed
   *by the
   * value returned by this function.
   **/
  const T *data() const {
    return static_cast<const T *>(storage_->of_array.content_);
  }
  size_t size() const { return storage_->array_length(); }
  size_t length() const { return size(); }
  bool is_initial_value() const {
    return *this == vector_cref_base<T, InstructionType>(
                        &this->instruction()->initial_value(), instruction());
  }

  const_iterator begin() const { return data(); }
  const_iterator end() const { return data() + size(); }
  const_reverse_iterator rbegin() const {
    return const_reverse_iterator(end());
  }
  const_reverse_iterator rend() const {
    return const_reverse_iterator(begin());
  }
  const_iterator cbegin() const { return data(); }
  const_iterator cend() const { return data() + size(); }
  const_reverse_iterator crbegin() const {
    return const_reverse_iterator(cend());
  }
  const_reverse_iterator crend() const {
    return const_reverse_iterator(cbegin());
  }
  T operator[](std::size_t index) const {
    assert(index < size());
    return data()[index];
  }

  instruction_cptr instruction() const {
    return static_cast<instruction_cptr>(field_cref::instruction());
  }

protected:
  friend class mfast::detail::codec_helper;

  void save_to(value_storage &v) const {
    v.of_array.content_ = this->storage()->of_array.content_;
    v.of_array.len_ = this->storage()->of_array.len_;
    v.of_array.capacity_in_bytes_ = 0;
    v.defined(true);
  }

private:
  vector_cref_base &operator=(const vector_cref_base &);
};

template <typename T, typename InstructionType>
bool operator==(const vector_cref_base<T, InstructionType> &lhs,
                const vector_cref_base<T, InstructionType> &rhs) {
  if (lhs.size() != rhs.size())
    return false;
  if (lhs.data() == rhs.data())
    return true;
  return std::memcmp(lhs.data(), rhs.data(), lhs.size()) == 0;
}

template <typename T, typename InstructionType>
bool operator!=(const vector_cref_base<T, InstructionType> &lhs,
                const vector_cref_base<T, InstructionType> &rhs) {
  return !(lhs == rhs);
}

template <typename T>
class vector_cref
    : public vector_cref_base<T, typename vector_instruction_trait<T>::type> {
  typedef vector_cref_base<T, typename vector_instruction_trait<T>::type>
      base_type;

public:
  typedef typename base_type::instruction_cptr instruction_cptr;
  typedef string_type_tag type_category;
  vector_cref() {}
  vector_cref(const value_storage *storage, instruction_cptr instruction)
      : base_type(storage, instruction) {}

  vector_cref(const vector_cref &other) : base_type(other) {}
  explicit vector_cref(const field_cref &other) : base_type(other) {}
};

template <typename T> class int_vector_cref : public vector_cref<T> {
  typedef vector_cref<T> base_type;

public:
  typedef typename base_type::instruction_cptr instruction_cptr;
  typedef int_vector_type_tag type_category;

  int_vector_cref() {}
  int_vector_cref(const value_storage *storage, instruction_cptr instruction)
      : base_type(storage, instruction) {}

  int_vector_cref(const int_vector_cref &other) : base_type(other) {}
  explicit int_vector_cref(const field_cref &other) : base_type(other) {}
};

typedef vector_cref<unsigned char> byte_vector_cref;
typedef int_vector_cref<int32_t> int32_vector_cref;
typedef int_vector_cref<uint32_t> uint32_vector_cref;
typedef int_vector_cref<int64_t> int64_vector_cref;
typedef int_vector_cref<uint64_t> uint64_vector_cref;

template <typename T>
class vector_mref_base : public make_field_mref<vector_cref<T>> {
  typedef make_field_mref<vector_cref<T>> base_type;

public:
  typedef std::size_t size_t;
  typedef typename base_type::value_type value_type;
  typedef value_type &reference;
  typedef value_type *iterator;
  typedef typename base_type::instruction_cptr instruction_cptr;
  typedef std::reverse_iterator<iterator> reverse_iterator;

  vector_mref_base() {}
  vector_mref_base(mfast::allocator *alloc, value_storage *storage,
                   instruction_cptr instruction)
      : base_type(alloc, storage, instruction) {}

  vector_mref_base(const vector_mref_base &) = default;

  explicit vector_mref_base(const field_mref_base &other) : base_type(other) {}
  void to_initial_value() const {
    if (this->instruction()->initial_value().is_empty())
      this->omit();
    else
      copy_from(this->instruction()->initial_value());
  }

  void as(const vector_cref<T> &cref) const {
    if (cref.absent()) {
      this->omit();
    } else {
      assign(cref.begin(), cref.end());
    }
  }

  void as(const std::vector<T> &value) const {
    assign(value.begin(), value.end());
  }
  template <int SIZE> void as(const T(&array)[SIZE]) const {
    assign(array, array + SIZE);
  }

  value_type *data() const {
    return static_cast<value_type *>(this->storage()->of_array.content_);
  }

  iterator begin() const { return data(); }
  iterator end() const { return data() + this->size(); }
  reverse_iterator rbegin() const { return reverse_iterator(end()); }
  reverse_iterator rend() const { return reverse_iterator(begin()); }
  value_type &operator[](size_t index) const {
    assert(index < this->size() && capacity() != 0);
    return data()[index];
  }

  void resize(size_t n, char c = 0) const {
    assert(n < static_cast<size_t>((std::numeric_limits<int32_t>::max)()));
    size_t len = n + 1;
    if (len > this->capacity()) {
      reserve(n);
      std::make_signed<std::size_t>::type bytes_to_set = n - this->size();
      if (bytes_to_set > 0)
        std::memset(end(), c, static_cast<size_t>(bytes_to_set));
    }
    this->storage()->array_length(static_cast<uint32_t>(n));
  }

  void reserve(size_t n) const;

  void push_back(char c) const { insert(end(), c); }
  void pop_back() const {
    if (this->size())
      this->storage()->of_array.len_ -= 1;
  }

  iterator insert(iterator position, value_type val) const {
    iterator itr = this->shift(position, 1);
    *itr = val;
    return itr;
  }

  void insert(iterator position, size_t n, value_type val) const {
    assert(position >= begin() && position <= end());
    iterator itr = this->shift(position, n);
    std::fill(itr, itr + n, val);
  }

  template <class InputIterator>
  void insert(iterator position, InputIterator first,
              InputIterator last) const {
    assert(position >= begin() && position <= end());
    typedef typename std::iterator_traits<InputIterator>::iterator_category
        category;
    this->insert(position, first, last, category());
  }

  template <class InputIterator>
  void assign(InputIterator first, InputIterator last) const {
    typedef typename std::iterator_traits<InputIterator>::iterator_category
        category;
    this->assign(first, last, category());
  }

  void assign(size_t n, value_type val) const {
    resize(n);
    std::fill(begin(), end(), val);
  }

  void refers_to(const value_type *addr, size_t n) const {
    assert(n < static_cast<size_t>((std::numeric_limits<int32_t>::max)()));
    if (this->storage()->of_array.capacity_in_bytes_ > 0 &&
        this->storage()->of_array.content_ != nullptr) {
      this->allocator()->deallocate(
          this->storage()->of_array.content_,
          this->storage()->of_array.capacity_in_bytes_);
    }
    this->storage()->of_array.content_ =
        const_cast<void *>(static_cast<const void *>(addr));
    this->storage()->array_length(static_cast<uint32_t>(n));
    this->storage()->of_array.capacity_in_bytes_ = 0;
  }

  void shallow_assign(const value_type *addr, size_t n) const {
    this->refers_to(addr, n);
  }
  void replace(size_t pos, size_t count, const value_type *addr,
               size_t count2) const;

protected:
  void swap(const vector_mref_base &other) const {
    assert(this->allocator() == other.allocator());
    std::swap(*this->storage(), *other.storage());
  }

  friend class mfast::detail::codec_helper;
  void copy_from(const value_storage &v) const {
    const char *ptr = static_cast<const char *>(v.of_array.content_);
    this->assign(ptr, ptr + v.array_length());
  }

private:
  vector_mref_base &operator=(const vector_mref_base &);

  template <class RandomAccessIterator>
  void insert(iterator position, RandomAccessIterator first,
              RandomAccessIterator last,
              std::random_access_iterator_tag) const {
    size_t n = std::distance(first, last);
    std::copy(first, last, this->shift(position, n));
  }

  template <class InputIterator>
  void insert(iterator position, InputIterator first, InputIterator last,
              std::input_iterator_tag) const {
    for (; first != last; ++first) {
      position = insert(position, *first);
      ++position;
    }
  }

  template <class RandomAccessIterator>
  void assign(RandomAccessIterator first, RandomAccessIterator last,
              std::random_access_iterator_tag) const {
    size_t n = std::distance(first, last);
    this->reserve(n);
    std::copy(first, last, this->begin());
    this->storage()->array_length(static_cast<uint32_t>(n));
  }

  template <class InputIterator>
  void assign(InputIterator first, InputIterator last,
              std::input_iterator_tag) const {
    this->resize(0);
    this->insert(this->begin(), first, last, std::input_iterator_tag());
  }

  // The reason I decide to hide capacity() is because it may return a value
  // which is less than size() when the data() is not allocated by @c alloc_.
  // This behavior is different from the @c std::vector::capacity().
  size_t capacity() const {
    return this->storage()->of_array.capacity_in_bytes_ / sizeof(T);
  }
  iterator shift(iterator position, size_t n) const;
};

template <typename T> class vector_mref : public vector_mref_base<T> {
  typedef vector_mref_base<T> base_type;

public:
  typedef typename base_type::instruction_cptr instruction_cptr;
  vector_mref() {}
  vector_mref(mfast::allocator *alloc, value_storage *storage,
              instruction_cptr instruction)
      : base_type(alloc, storage, instruction) {}

  vector_mref(const vector_mref &other) : base_type(other) {}
  explicit vector_mref(const field_mref_base &other) : base_type(other) {}
  void swap(const vector_mref<T> &other) const { base_type::swap(other); }
};

template <typename T> class int_vector_mref : public vector_mref<T> {
  typedef vector_mref<T> base_type;

public:
  typedef typename base_type::instruction_cptr instruction_cptr;
  int_vector_mref() {}
  int_vector_mref(mfast::allocator *alloc, value_storage *storage,
                  instruction_cptr instruction)
      : base_type(alloc, storage, instruction) {}

  int_vector_mref(const int_vector_mref &other) : base_type(other) {}
  explicit int_vector_mref(const field_mref_base &other) : base_type(other) {}
  operator int_vector_cref<T>() const {
    return int_vector_cref<T>(this->storage(), this->instruction());
  }

  void swap(const int_vector_cref<T> &other) const { base_type::swap(other); }
};

typedef vector_mref<unsigned char> byte_vector_mref;
typedef int_vector_mref<int32_t> int32_vector_mref;
typedef int_vector_mref<uint32_t> uint32_vector_mref;
typedef int_vector_mref<int64_t> int64_vector_mref;
typedef int_vector_mref<uint64_t> uint64_vector_mref;

template <typename T> struct mref_of<vector_cref<T>> {
  typedef vector_mref<T> type;
};

template <typename T> struct mref_of<int_vector_cref<T>> {
  typedef int_vector_mref<T> type;
};

template <typename CREF> class vector_type {
public:
  typedef typename CREF::instruction_type instruction_type;
  typedef typename CREF::instruction_cptr instruction_cptr;
  typedef CREF cref_type;
  typedef typename mref_of<CREF>::type mref_type;

  vector_type(
      mfast::allocator *alloc = nullptr,
      instruction_cptr instruction = instruction_type::default_instruction())
      : alloc_(alloc), instruction_(instruction) {
    instruction_->construct_value(my_storage_, alloc);
  }

  ~vector_type() { instruction_->destruct_value(my_storage_, alloc_); }
  mref_type ref() { return mref_type(alloc_, &my_storage_, instruction_); }
  mref_type mref() { return mref_type(alloc_, &my_storage_, instruction_); }
  cref_type ref() const { return cref_type(&my_storage_, instruction_); }
  cref_type cref() const { return cref_type(&my_storage_, instruction_); }
  instruction_cptr instruction() const { return instruction_; }

private:
  mfast::allocator *alloc_;
  instruction_cptr instruction_;
  value_storage my_storage_;
};

typedef vector_type<byte_vector_cref> byte_vector_type;
typedef int_vector_mref<int32_vector_cref> int32_vector_type;
typedef int_vector_mref<uint32_vector_cref> uint32_vector_type;
typedef int_vector_mref<int64_vector_cref> int64_vector_type;
typedef int_vector_mref<uint64_vector_cref> uint64_vector_type;

template <typename T>
void vector_mref_base<T>::reserve(
    typename vector_mref_base<T>::size_t n) const {
  if (capacity() > n)
    return;
  std::size_t reserve_size = (n + 1) * sizeof(value_type);

  if (capacity() > 0) {
    this->storage()->of_array.capacity_in_bytes_ = this->alloc_->reallocate(
        this->storage()->of_array.content_, capacity(), reserve_size);
  } else {
    void *old_addr = this->storage()->of_array.content_;
    this->storage()->of_array.content_ = nullptr;
    this->storage()->of_array.capacity_in_bytes_ = this->alloc_->reallocate(
        this->storage()->of_array.content_, 0, reserve_size);
    // Copy the old content to the new buffer.
    // In the case when the this->capacity == 0 && this->size() > 0,
    // reserve() could be invoked with n < this->size(). Thus, we can
    // only copy min(size(), n) elements to the new buffer.
    if (this->storage()->of_array.len_ > 1) {
      if (n > 0)
        std::memcpy(this->storage()->of_array.content_, old_addr,
                    std::min<size_t>(this->size(), n) * sizeof(value_type));
    } else {
      // if this->storage()->of_array.len_ was 0, it needs to be set to 1 to
      // indicate
      // buffer has been allocated yet no data be written to it yet.
      this->storage()->of_array.len_ = 1;
    }
  }
}

template <typename T>
typename vector_mref_base<T>::iterator
vector_mref_base<T>::shift(typename vector_mref_base<T>::iterator position,
                           typename vector_mref_base<T>::size_t n) const {
  assert(position >= begin() && position <= end());
  std::size_t offset = position - begin();

  if (capacity() < this->size() + n)
    reserve(this->size() + n);

  iterator src = begin() + offset;

  if (offset < this->size()) {
    std::memmove(src + (n * sizeof(value_type)), src,
                 (this->size() + 1 - offset) * sizeof(value_type));
  }

  this->storage()->of_array.len_ += static_cast<uint32_t>(n);
  return src;
}

template <typename T>
void vector_mref_base<T>::replace(size_t pos, size_t count,
                                  const value_type *addr, size_t count2) const {
  if (pos + count >= this->size()) {
    // all the elements starting from pos are to removed from the vector
    this->resize(pos + count2);
  } else {
    std::ptrdiff_t diff = count2 - count;
    if (diff > 0) {
      // some elements has to be shift back
      this->shift(data() + pos, diff);
    } else {
      std::memmove(data() + pos + count2, data() + pos + count,
                   -diff * sizeof(value_type));
      this->resize(this->size() + diff);
    }
  }
  std::memcpy(data() + pos, addr, count2 * sizeof(value_type));
}
}
