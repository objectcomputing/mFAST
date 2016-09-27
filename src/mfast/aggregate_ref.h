// Copyright (c) 2016, Huang-Ming Huang,  Object Computing, Inc.
// All rights reserved.
//
// This file is part of mFAST.
// See the file license.txt for licensing information.
#pragma once

#include "field_instructions.h"
#include "field_mref.h"
#include "type_category.h"

#include <boost/iterator/iterator_facade.hpp>
#include <boost/range.hpp>

namespace mfast {
class unbouned_templateref_error : public virtual boost::exception,
                                   public virtual std::exception {
public:
  unbouned_templateref_error() {}
};

// forward declaration
template <typename T> class make_aggregate_mref;

struct aggregate_cref_core_access;
class aggregate_cref {
public:
  typedef const group_field_instruction *instruction_cptr;
  static const bool is_mutable = false;
  typedef group_type_tag type_category;

  aggregate_cref(const value_storage *storage_array = nullptr,
                 const group_field_instruction *instruction = nullptr);

  aggregate_cref(const aggregate_cref &other);

  explicit aggregate_cref(const field_cref &other);

  size_t num_fields() const;

  field_cref operator[](size_t index) const;

  /// return -1 if no such field is found
  int field_index_with_id(std::size_t id) const;

  /// return -1 if no such field is found
  int field_index_with_name(const char *name) const;

  const group_field_instruction *instruction() const;

  const field_instruction *subinstruction(size_t index) const;

  const value_storage *field_storage(size_t index) const;

  bool absent() const { return storage_array_ == nullptr; }
  bool present() const { return !absent(); }
  void refers_to(const aggregate_cref &other) {
    this->instruction_ = other.instruction_;
    this->storage_array_ = other.storage_array_;
  }

  class iterator
      : public boost::iterator_facade<iterator, field_cref,
                                      boost::random_access_traversal_tag,
                                      field_cref> {
  public:
    iterator(const value_storage *storage_array = nullptr,
             const const_instruction_ptr_t *instruction_array = nullptr);

  private:
    friend class boost::iterator_core_access;

    bool equal(iterator const &other) const;
    field_cref dereference() const;
    void advance(int n);
    void increment();
    void decrement();
    std::ptrdiff_t distance_to(iterator const &other) const;
    const value_storage *storage_array_;
    const const_instruction_ptr_t *instruction_array_;
  };

  typedef iterator const_iterator;

  iterator begin() const;
  iterator end() const;

protected:
  const value_storage *storage() const { return storage_array_; }
  const group_field_instruction *instruction_;
  const value_storage *storage_array_;

private:
  aggregate_cref &operator=(const aggregate_cref &);

  friend struct aggregate_cref_core_access;

  template <typename ElementRef, bool IsElementAggregate>
  friend struct sequence_iterator_base;
};

struct aggregate_cref_core_access {
  static const value_storage *storage_of(const aggregate_cref &r) {
    return r.storage_array_;
  }
};

struct aggregate_mref_core_access;

template <typename T, typename U> class make_message_mref;
class message_mref;

template <typename ConstRef> class make_aggregate_mref : public ConstRef {
public:
  static const bool is_mutable = true;
  typedef ConstRef cref_type;
  typedef typename ConstRef::instruction_cptr instruction_cptr;

  make_aggregate_mref();

  template <typename U>
  make_aggregate_mref(const make_aggregate_mref<U> &other);

  make_aggregate_mref(mfast::allocator *alloc, value_storage *storage,
                      instruction_cptr instruction);

  explicit make_aggregate_mref(const field_mref_base &other);

  field_mref operator[](size_t index) const;

  mfast::allocator *allocator() const;

  void as(const ConstRef &other) const;

  void refers_to(const make_aggregate_mref<ConstRef> &other) {
    ConstRef::refers_to(other);
    this->alloc_ = other.alloc_;
  }

  class iterator
      : public boost::iterator_facade<iterator, field_mref,
                                      boost::random_access_traversal_tag,
                                      field_mref> {
  public:
    iterator(mfast::allocator *alloc, value_storage *storage_array,
             const const_instruction_ptr_t *instruction_array);

  private:
    friend class boost::iterator_core_access;

    bool equal(iterator const &other) const;
    field_mref dereference() const;
    void advance(int n);
    void increment();
    void decrement();
    std::ptrdiff_t distance_to(iterator const &other) const;

    value_storage *storage_array_;
    const const_instruction_ptr_t *instruction_array_;
    mfast::allocator *alloc_;
  };

  typedef iterator const_iterator;

  iterator begin() const;
  iterator end() const;

protected:
  template <class FieldMutator> friend class field_mutator_adaptor;

  value_storage *field_storage(size_t index) const;

  void link_group_at(size_t index,
                     const make_aggregate_mref<aggregate_cref> &ref) const;
  void unlink_group_at(size_t index) const;

private:
  friend struct aggregate_mref_core_access;
  template <typename U> friend class make_aggregate_mref;

  make_aggregate_mref &operator=(const make_aggregate_mref &);
  mfast::allocator *alloc_;
};

typedef make_aggregate_mref<aggregate_cref> aggregate_mref;

struct aggregate_mref_core_access {
  template <typename T>
  static value_storage *storage_of(const make_aggregate_mref<T> &r) {
    return r.field_storage(0);
  }
};

/////////////////////////////////////////////////////////////////

inline aggregate_cref::aggregate_cref(
    const value_storage *storage_array,
    const group_field_instruction *instruction)
    : instruction_(instruction), storage_array_(storage_array) {}

inline aggregate_cref::aggregate_cref(const aggregate_cref &other)
    : instruction_(other.instruction()), storage_array_(other.storage_array_) {}

inline aggregate_cref::aggregate_cref(const field_cref &other)
    : storage_array_(
          field_cref_core_access::storage_of(other)->of_group.content_) {
  if (other.instruction()->field_type() == field_type_templateref) {
    this->instruction_ = static_cast<const group_field_instruction *>(
        field_cref_core_access::storage_of(other)
            ->of_templateref.of_instruction.instruction_);
    if (this->instruction_ == nullptr)
      BOOST_THROW_EXCEPTION(unbouned_templateref_error());
    // if you hit the exeception, you have to use
    // dynamic_nested_message_mref::rebind() to bind
    // a valid
    // template_instruction intead. If this is a static templateRef, please use
    // the resolved
    // templates
    // provided by encoder or those generated by fast_type_gen.

  } else {
    this->instruction_ =
        static_cast<const group_field_instruction *>(other.instruction());
  }
}

inline size_t aggregate_cref::num_fields() const {
  return instruction()->subinstructions().size();
}
inline field_cref aggregate_cref::operator[](size_t index) const {
  return field_cref(&storage_array_[index], subinstruction(index));
}

inline const group_field_instruction *aggregate_cref::instruction() const {
  return instruction_;
}
inline const field_instruction *
aggregate_cref::subinstruction(size_t index) const {
  return instruction()->subinstruction(index);
}

inline const value_storage *aggregate_cref::field_storage(size_t index) const {
  return &storage_array_[index];
}

/// return -1 if no such field is found
inline int aggregate_cref::field_index_with_id(std::size_t id) const {
  return instruction()->find_subinstruction_index_by_id(
      static_cast<uint32_t>(id));
}

/// return -1 if no such field is found
inline int aggregate_cref::field_index_with_name(const char *name) const {
  return instruction()->find_subinstruction_index_by_name(name);
}

inline aggregate_cref::iterator aggregate_cref::begin() const {
  return aggregate_cref::iterator(
      this->storage_array_, this->instruction()->subinstructions().begin());
}

inline aggregate_cref::iterator aggregate_cref::end() const {
  return aggregate_cref::iterator(this->storage_array_ + this->num_fields(),
                                  this->instruction()->subinstructions().end());
}

////////////////////////////////////////////////

inline aggregate_cref::iterator::iterator(
    const value_storage *storage_array,
    const const_instruction_ptr_t *instruction_array)
    : storage_array_(storage_array), instruction_array_(instruction_array) {}

inline bool
aggregate_cref::iterator::equal(aggregate_cref::iterator const &other) const {
  return (other.storage_array_ == this->storage_array_);
}

inline field_cref aggregate_cref::iterator::dereference() const
// ElementRef dereference() const
{
  return field_cref(this->storage_array_, *instruction_array_);
}

inline void aggregate_cref::iterator::advance(int n) {
  storage_array_ += n;
  instruction_array_ += n;
}

inline void aggregate_cref::iterator::increment() { advance(1); }
inline void aggregate_cref::iterator::decrement() { advance(-1); }
inline std::ptrdiff_t aggregate_cref::iterator::distance_to(
    aggregate_cref::iterator const &other) const {
  return (other.storage_array_ - this->storage_array_);
}

///////////////////////////////////////////////////////

template <typename ConstRef>
inline make_aggregate_mref<ConstRef>::make_aggregate_mref()
    : alloc_(nullptr) {}

template <typename ConstRef>
template <typename U>
inline make_aggregate_mref<ConstRef>::make_aggregate_mref(
    const make_aggregate_mref<U> &other)
    : ConstRef(other.field_storage(0),
               static_cast<instruction_cptr>(other.instruction())),
      alloc_(other.allocator()) {}

template <typename ConstRef>
inline make_aggregate_mref<ConstRef>::make_aggregate_mref(
    mfast::allocator *alloc, value_storage *storage_array,
    typename ConstRef::instruction_cptr instruction)
    : ConstRef(storage_array, instruction), alloc_(alloc) {}

template <typename ConstRef>
inline make_aggregate_mref<ConstRef>::make_aggregate_mref(
    const field_mref_base &other)
    : ConstRef(other), alloc_(other.allocator()) {}

template <typename ConstRef>
inline field_mref make_aggregate_mref<ConstRef>::
operator[](size_t index) const {
  assert(index < this->num_fields());

  return field_mref(this->alloc_, this->field_storage(index),
                    this->instruction()->subinstruction(index));
}

template <typename ConstRef>
inline mfast::allocator *make_aggregate_mref<ConstRef>::allocator() const {
  return alloc_;
}

template <typename ConstRef>
inline value_storage *
make_aggregate_mref<ConstRef>::field_storage(size_t index) const {
  return const_cast<value_storage *>(ConstRef::field_storage(index));
}

template <typename ConstRef>
void make_aggregate_mref<ConstRef>::as(const ConstRef &other) const {
  // we can only do the assignment when other is present
  assert(other.present());

  // make sure the type are exactly the same
  assert(typeid(*this->instruction()) == typeid(*other.instruction()));

  std::size_t sz = this->instruction()->group_content_byte_count();

  // first, we need to allocate two chuck of memories of size sz
  char *buf = static_cast<char *>(alloca(2 * sz));
  value_storage *orig_content = reinterpret_cast<value_storage *>(buf);
  value_storage *new_content = reinterpret_cast<value_storage *>(buf + sz);
  // the first chuck for storing the original content
  std::memcpy(orig_content, this->storage_array_, sz);
  value_storage tmp_storage;

  // the second chunk for construct the new aggregate value

  value_storage other_storage;
  other_storage.of_group.own_content_ = false;
  other_storage.of_group.content_ = const_cast<value_storage *>(
      aggregate_cref_core_access::storage_of(other));

  this->instruction()->copy_construct_value(other_storage, tmp_storage,
                                            this->alloc_, new_content);

  // copy the newly construct aggregate value to the old memory chunk
  std::memcpy(const_cast<value_storage *>(this->storage_array_), new_content,
              sz);
  // destruct the subfields on first memory chunk which contains the original
  // value
  this->instruction()->destruct_group_subfields(orig_content, alloc_);
}

template <typename ConstRef>
inline typename make_aggregate_mref<ConstRef>::iterator
make_aggregate_mref<ConstRef>::begin() const {
  return iterator(this->alloc_, this->field_storage(0),
                  this->instruction()->subinstructions().begin());
}

template <typename ConstRef>
inline typename make_aggregate_mref<ConstRef>::iterator
make_aggregate_mref<ConstRef>::end() const {
  return iterator(this->alloc_, this->field_storage(this->num_fields()),
                  this->instruction()->subinstructions().end());
}

template <typename ConstRef>
void make_aggregate_mref<ConstRef>::link_group_at(
    size_t index, const aggregate_mref &ref) const {
  value_storage *storage = this->field_storage(index);
  if (ref.present()) {
    const group_field_instruction *inst =
        static_cast<const group_field_instruction *>(
            this->instruction()->subinstruction(index));
    inst->link_value(*storage, const_cast<value_storage *>(ref.storage_array_),
                     this->alloc_);
  } else {
    storage->of_group.present_ = false;
  }
}

template <typename ConstRef>
inline void make_aggregate_mref<ConstRef>::unlink_group_at(size_t index) const {
  value_storage *storage = this->field_storage(index);
  if (storage->of_group.is_link_) {
    const field_instruction *inst = this->instruction()->subinstruction(index);
    inst->construct_value(*storage, this->alloc_);
  }
}

////////////////////////////////////////////////

template <typename ConstRef>
inline make_aggregate_mref<ConstRef>::iterator::iterator(
    mfast::allocator *alloc, value_storage *storage_array,
    const const_instruction_ptr_t *instruction_array)
    : storage_array_(storage_array), instruction_array_(instruction_array),
      alloc_(alloc) {}

template <typename ConstRef>
inline bool make_aggregate_mref<ConstRef>::iterator::equal(
    typename make_aggregate_mref<ConstRef>::iterator const &other) const {
  return (other.storage_array_ == this->storage_array_);
}

template <typename ConstRef>
inline field_mref make_aggregate_mref<ConstRef>::iterator::dereference() const
// ElementRef dereference() const
{
  return field_mref(this->alloc_, this->storage_array_, *instruction_array_);
}

template <typename ConstRef>
inline void make_aggregate_mref<ConstRef>::iterator::advance(int n) {
  storage_array_ += n;
  instruction_array_ += n;
}

template <typename ConstRef>
inline void make_aggregate_mref<ConstRef>::iterator::increment() {
  advance(1);
}

template <typename ConstRef>
inline void make_aggregate_mref<ConstRef>::iterator::decrement() {
  advance(-1);
}

template <typename ConstRef>
inline std::ptrdiff_t make_aggregate_mref<ConstRef>::iterator::distance_to(
    typename make_aggregate_mref<ConstRef>::iterator const &other) const {
  return (other.storage_array_ - this->storage_array_);
}
}
