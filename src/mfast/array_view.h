// Copyright (c) 2016, Huang-Ming Huang,  Object Computing, Inc.
// All rights reserved.
//
// This file is part of mFAST.
// See the file license.txt for licensing information.
#pragma once
#include <boost/iterator/reverse_iterator.hpp>

namespace mfast {
template <typename T> class array_view {
public:
  array_view() : data_(nullptr), sz_(0) {}
  array_view(T *data, std::size_t sz) : data_(data), sz_(sz) {}
  template <int N> array_view(T(&array)[N]) : data_(array), sz_(N) {}

  typedef T *iterator;
  typedef const T *const_iterator;
  typedef T value;
  typedef T &reference;
  typedef const T &const_reference;
  typedef std::size_t size_type;
  typedef std::ptrdiff_t difference_type;
  typedef boost::reverse_iterator<iterator> reverse_iterator;
  typedef reverse_iterator const_reverse_iterator;

  iterator begin() { return data_; }
  iterator end() { return data_ + sz_; }
  const_iterator begin() const { return data_; }
  const_iterator end() const { return data_ + sz_; }
  std::size_t size() const { return sz_; }
  // element access]
  reference operator[](size_type i) { return data_[i]; }
  const_reference operator[](size_type i) const { return data_[i]; }
  reference front() { return data_[0]; }
  const_reference front() const { return data_[0]; }
  reference back() { return data_[sz_ - 1]; }
  const_reference back() const { return data_[sz_ - 1]; }
  T *data() { return data_; }
  const T *data() const { return data_; }
  reverse_iterator rbegin() const {
    return boost::make_reverse_iterator(this->end());
  }
  reverse_iterator rend() const {
    return boost::make_reverse_iterator(this->begin());
  }

private:
  T *data_;
  std::size_t sz_;
};

} /* mfast */
