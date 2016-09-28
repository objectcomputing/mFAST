// Copyright (c) 2016, Huang-Ming Huang,  Object Computing, Inc.
// All rights reserved.
//
// This file is part of mFAST.
// See the file license.txt for licensing information.
#pragma once

#include "field_instructions.h"
#include "field_ref.h"
#include "group_ref.h"
#include "aggregate_ref.h"

#include <cassert>
#include <boost/iterator/iterator_facade.hpp>
#include <boost/range.hpp>

namespace mfast {
class sequence_element_cref : public aggregate_cref {
public:
  typedef sequence_field_instruction instruction_type;
  typedef const sequence_field_instruction *instruction_cptr;

  sequence_element_cref(const value_storage *storage,
                        instruction_cptr instruction);

  sequence_element_cref(const sequence_element_cref &) = default;
  instruction_cptr instruction() const;

  bool element_unnamed() const {
    return this->instruction()->subinstructions().size() == 1 &&
           this->instruction()->subinstruction(0)->name()[0] == 0;
  }
};

typedef make_aggregate_mref<sequence_element_cref> sequence_element_mref;

template <> struct mref_of<sequence_element_cref> {
  typedef sequence_element_mref type;
};

template <typename ElementRef, bool IsElementAggregate>
struct sequence_iterator_base;

template <typename ElementRef> struct sequence_iterator_base<ElementRef, true> {
  sequence_iterator_base() {}
  explicit sequence_iterator_base(ElementRef ref) : element_(std::move(ref)) {}
  sequence_iterator_base(const sequence_iterator_base &other)
      : element_(other.element_) {}
  const char *content_storage() const {
    return reinterpret_cast<const char *>(element_.storage_array_);
  }

  void content_storage(const void *ptr) {
    element_.storage_array_ = reinterpret_cast<const value_storage *>(ptr);
  }

  std::size_t element_group_content_byte_count() const {
    return element_.instruction()->group_content_byte_count();
  }

  ElementRef element_;
};

template <typename ElementRef>
struct sequence_iterator_base<ElementRef, false> {
  sequence_iterator_base() {}
  explicit sequence_iterator_base(ElementRef ref) : element_(std::move(ref)) {}
  sequence_iterator_base(const sequence_iterator_base &other)
      : element_(other.element_) {}
  const char *content_storage() const {
    return reinterpret_cast<const char *>(
        field_mref_core_access::storage_of(element_));
  }

  void content_storage(const void *ptr) {
    element_.storage_ = reinterpret_cast<const value_storage *>(ptr);
  }

  std::size_t element_group_content_byte_count() const {
    return sizeof(value_storage);
  }
  ElementRef element_;
};

template <typename ElementRef, typename IsElementAggregate>
class sequence_iterator
    : sequence_iterator_base<ElementRef, IsElementAggregate::value>,
      public boost::iterator_facade<
          sequence_iterator<ElementRef, IsElementAggregate>, ElementRef,
          boost::random_access_traversal_tag, ElementRef> {
public:
  sequence_iterator() {}
  explicit sequence_iterator(const ElementRef &ref)
      : sequence_iterator_base<ElementRef, IsElementAggregate::value>(ref) {}

  sequence_iterator(const sequence_iterator &other)
      : sequence_iterator_base<ElementRef, IsElementAggregate::value>(
            other.element_) {}

  sequence_iterator &operator=(const sequence_iterator &other) {
    this->element_.refers_to(other.element_);
    return *this;
  }

private:
  friend class boost::iterator_core_access;

  bool equal(sequence_iterator const &other) const {
    return this->content_storage() == other.content_storage();
  }

  ElementRef const &dereference() const
  // ElementRef dereference() const
  {
    return this->element_;
  }

  void advance(std::ptrdiff_t n) {
    this->content_storage(this->content_storage() +
                          n * this->element_group_content_byte_count());
  }

  void increment() { advance(1); }
  void decrement() { advance(-1); }
  int distance_to(sequence_iterator const &other) const {
    return (other.content_storage() - this->content_storage()) /
           this->element_group_content_byte_count();
  }
};

///
/// Used for representing sequence where elements are defined in place, such as
///
/// <sequence name="seq">
///    <string name="field1" />
///    <string name="field2" />
/// </sequence>
struct inline_element_sequence_trait {
  typedef boost::true_type is_element_aggregate;
  static const field_instruction *
  element_instruction(const sequence_field_instruction *inst)

  {
    return inst->element_instruction() ? inst->element_instruction() : inst;
  }
};

///
/// Used for representing sequence where elements are defined frome static
/// templateref or an unnamed
/// group, such as
///
/// <sequence name="Seq">
///    <templateRef name="MyTemplate" />
/// </sequence>
///
/// or
///
/// <define name="MyGroup">
///   <group> ...</group>
/// </define>
/// <sequence name="Seq">
///    <field> <type name="MyGroup" /> </field>
/// </sequence>
struct defined_element_sequence_trait {
  typedef boost::true_type is_element_aggregate;
  static const field_instruction *
  element_instruction(const sequence_field_instruction *inst) {
    return inst->element_instruction();
  }
};

///
/// Used for representing sequence with only one unamed field, such as
///
/// <sequence name="Seq">
///    <string />
/// </sequence>
///

struct sole_element_sequence_trait {
  typedef boost::false_type is_element_aggregate;
  static const field_instruction *
  element_instruction(const sequence_field_instruction *inst) {
    return inst->subinstruction(0);
  }
};

namespace detail {
extern MFAST_EXPORT const value_storage default_sequence_storage;
}

template <typename ElementType,
          typename SequenceTrait = inline_element_sequence_trait,
          typename SequenceInstructionType =
              mfast::sequence_instruction_ex<ElementType>>
class make_sequence_cref : public field_cref {
public:
  typedef SequenceInstructionType instruction_type;
  typedef typename ElementType::instruction_type element_instruction_type;
  typedef const SequenceInstructionType *instruction_cptr;
  typedef typename SequenceTrait::is_element_aggregate is_element_aggregate;
  typedef sequence_type_tag type_category;

  typedef ElementType reference;
  typedef sequence_iterator<ElementType, is_element_aggregate> iterator;
  typedef iterator const_iterator;
  typedef boost::reverse_iterator<iterator> reverse_iterator;
  typedef reverse_iterator const_reverse_iterator;

  make_sequence_cref() : field_cref(&detail::default_sequence_storage, 0) {}
  make_sequence_cref(const value_storage *storage, instruction_cptr instruction)
      : field_cref(storage, instruction) {}

  explicit make_sequence_cref(const field_cref &other) : field_cref(other) {}
  reference operator[](size_t index) const {
    assert(index < size());
    return reference(element_storage(index), element_instruction());
  }

  reference front() const { return this->operator[](0); }
  reference back() const { return this->operator[](size() - 1); }
  operator make_sequence_cref<sequence_element_cref,
                              inline_element_sequence_trait,
                              sequence_field_instruction>() const {
    return make_sequence_cref<sequence_element_cref,
                              inline_element_sequence_trait,
                              sequence_field_instruction>(this->storage(),
                                                          this->instruction());
  }

  uint32_t length() const { return size(); }
  uint32_t size() const { return storage_->array_length(); }
  size_t num_fields() const { return instruction()->subinstructions().size(); }
  instruction_cptr instruction() const {
    return static_cast<instruction_cptr>(instruction_);
  }
  iterator begin() const {
    return iterator(reference(element_storage(0), element_instruction()));
  }

  iterator end() const {
    return iterator(
        reference(element_storage(this->size()), element_instruction()));
  }

  iterator cbegin() const { return begin(); }
  iterator cend() const { return end(); }
  reverse_iterator rbegin() const {
    return boost::make_reverse_iterator(this->end());
  }
  reverse_iterator rend() const {
    return boost::make_reverse_iterator(this->begin());
  }
  const_reverse_iterator crbegin() const { return this->rbegin(); }
  const_reverse_iterator crend() const { return this->rbegin(); }
  /**
   * @returns true if the sequence contains only one sub-field and the field is
   *unnamed.
   **/

  bool element_unnamed() const {
    return this->instruction()->subinstructions().size() == 1 &&
           this->instruction()->subinstruction(0)->name()[0] == 0;
  }

protected:
  typename ElementType::instruction_cptr element_instruction() const {
    return static_cast<typename ElementType::instruction_cptr>(
        SequenceTrait::element_instruction(instruction()));
  }

  const field_instruction *subinstruction(std::size_t index) const {
    return instruction()->subinstruction(index);
  }

  const value_storage *element_storage(std::size_t index) const {
    const value_storage *storages =
        static_cast<const value_storage *>(storage_->of_array.content_);
    return &storages[index * num_fields()];
  }
};

typedef make_sequence_cref<sequence_element_cref, inline_element_sequence_trait,
                           sequence_field_instruction> sequence_cref;

namespace detail {
struct MFAST_EXPORT sequence_mref_helper {
  static void reserve(const sequence_field_instruction *instruction,
                      value_storage *storage, allocator *alloc, std::size_t n);
};
}

template <typename ElementType,
          typename SequenceTrait = inline_element_sequence_trait,
          typename SequenceInstructionType =
              mfast::sequence_instruction_ex<typename ElementType::cref_type>>
class make_sequence_mref
    : public make_field_mref<
          make_sequence_cref<typename ElementType::cref_type, SequenceTrait,
                             SequenceInstructionType>> {
  typedef make_field_mref<make_sequence_cref<
      typename ElementType::cref_type, SequenceTrait, SequenceInstructionType>>
      base_type;

public:
  typedef make_sequence_cref<typename ElementType::cref_type, SequenceTrait,
                             SequenceInstructionType> cref_type;

  typedef typename cref_type::is_element_aggregate is_element_aggregate;
  typedef SequenceInstructionType instruction_type;
  typedef const SequenceInstructionType *instruction_cptr;
  typedef ElementType reference;
  typedef sequence_iterator<ElementType, is_element_aggregate> iterator;
  typedef iterator const_iterator; // This is required to make BOOST_FOREACH
  typedef boost::reverse_iterator<iterator> reverse_iterator;
  typedef reverse_iterator const_reverse_iterator;

  // make_sequence_mref()
  // {
  // }

  make_sequence_mref(mfast::allocator *alloc, value_storage *storage,
                     instruction_cptr instruction)
      : base_type(alloc, storage, instruction) {}

  make_sequence_mref(value_storage *storage, mfast::allocator *alloc)
      : base_type(storage, alloc) {}

  explicit make_sequence_mref(const field_mref_base &other)
      : base_type(other) {}
  operator make_sequence_mref<sequence_element_mref,
                              inline_element_sequence_trait,
                              sequence_field_instruction>() const {
    return make_sequence_mref<sequence_element_mref,
                              inline_element_sequence_trait,
                              sequence_field_instruction>(
        this->allocator(), this->storage(), this->instruction());
  }

  reference operator[](size_t index) const {
    assert(index < this->size());
    return reference(this->alloc_, element_storage(index),
                     this->element_instruction());
  }

  reference front() const { return this->operator[](0); }
  reference back() const { return this->operator[](this->size() - 1); }
  void resize(size_t n) const;
  void reserve(size_t n) const;

  instruction_cptr instruction() const {
    return static_cast<instruction_cptr>(base_type::instruction());
  }

  void as(const cref_type &) const;

  template <typename RandomAccessIterator>
  void assign(RandomAccessIterator first, RandomAccessIterator last) const {
    resize(std::distance(first, last));
    RandomAccessIterator itr;
    iterator dest = begin();
    for (itr = first; itr != last; ++itr, ++dest)
      dest->as(*itr);
  }

  template <typename Range> void as(Range r) const {
    assign(boost::begin(r), boost::end(r));
  }

  iterator begin() const {
    return iterator(reference(this->alloc_, element_storage(0),
                              this->element_instruction()));
  }

  iterator end() const {
    return iterator(reference(this->alloc_, element_storage(this->size()),
                              this->element_instruction()));
  }

  reverse_iterator rbegin() const {
    return boost::make_reverse_iterator(this->end());
  }
  reverse_iterator rend() const {
    return boost::make_reverse_iterator(this->begin());
  }
  /**
   * Grows this sequence  by delta elements.
   *
   * @returns An iterator to first item appended.
   **/
  iterator grow_by(std::size_t delta) const {
    assert(delta + this->size() <=
           static_cast<std::size_t>(
               std::numeric_limits<int32_t>::max() /
               this->instruction()->group_content_byte_count()));
    std::size_t old_size = this->size();
    resize(old_size + delta);
    return begin() + static_cast<std::ptrdiff_t>(old_size);
  }

  void swap(const make_sequence_mref &other) const {
    assert(this->allocator() == other.allocator());
    std::swap(*this->storage(), *other.storage());
  }

private:
  value_storage *element_storage(std::size_t index) const {
    return const_cast<value_storage *>(base_type::element_storage(index));
  }
};

typedef make_sequence_mref<sequence_element_mref, inline_element_sequence_trait,
                           sequence_field_instruction> sequence_mref;

template <typename ELEMENT_CREF, typename TRAIT, typename INSTRUCTION>
struct mref_of<make_sequence_cref<ELEMENT_CREF, TRAIT, INSTRUCTION>> {
  typedef make_sequence_mref<typename mref_of<ELEMENT_CREF>::type, TRAIT,
                             INSTRUCTION> type;
};

//////////////////////////////////////////////////////////////

inline sequence_element_cref::sequence_element_cref(
    const value_storage *storage,
    sequence_element_cref::instruction_cptr instruction)
    : aggregate_cref(storage, instruction) {}

inline sequence_element_cref::instruction_cptr
sequence_element_cref::instruction() const {
  return static_cast<instruction_cptr>(aggregate_cref::instruction());
}

template <typename ElementType, typename SequenceTrait,
          typename SequenceInstructionType>
inline void
make_sequence_mref<ElementType, SequenceTrait, SequenceInstructionType>::resize(
    size_t n) const {
  this->reserve(n);
  this->storage()->array_length(static_cast<uint32_t>(n));
}

template <typename ElementType, typename SequenceTrait,
          typename SequenceInstructionType>
inline void
make_sequence_mref<ElementType, SequenceTrait,
                   SequenceInstructionType>::reserve(size_t n) const {
  detail::sequence_mref_helper::reserve(
      static_cast<const sequence_field_instruction *>(this->instruction()),
      this->storage(), this->alloc_, n);
}

template <typename ElementType, typename SequenceTrait,
          typename SequenceInstructionType>
inline void
make_sequence_mref<ElementType, SequenceTrait, SequenceInstructionType>::as(
    const cref_type &src) const {
  value_storage tmp_storage;
  this->instruction()->copy_construct_value(*src.storage_, tmp_storage,
                                            this->alloc_);
  std::swap(tmp_storage, const_cast<value_storage &>(*this->storage_));
  this->instruction()->destruct_value(tmp_storage);
}
}
