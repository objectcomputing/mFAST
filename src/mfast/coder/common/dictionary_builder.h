// Copyright (c) 2016, Huang-Ming Huang,  Object Computing, Inc.
// All rights reserved.
//
// This file is part of mFAST.
// See the file license.txt for licensing information.
#pragma once

#include <tuple>
#include "../mfast_coder_export.h"
#include "../../field_instructions.h"
#include "../../arena_allocator.h"
#include <vector>
#include <map>
#include <string>
#include <stdexcept>

#ifdef _MSC_VER
#pragma warning(disable : 4251) // non dll-interface class used as a member for
                                // dll-interface class
#endif                          //_MSC_VER

namespace mfast {
template <typename... Args> inline void pass(Args &&...) {}

template <class T, T... Ints> struct integer_sequence {};

template <class S> struct next_integer_sequence;

template <class T, T... Ints>
struct next_integer_sequence<integer_sequence<T, Ints...>> {
  using type = integer_sequence<T, Ints..., sizeof...(Ints)>;
};

template <class T, T I, T N> struct make_int_seq_impl;

template <class T, T N>
using make_integer_sequence = typename make_int_seq_impl<T, 0, N>::type;

template <class T, T I, T N> struct make_int_seq_impl {
  using type = typename next_integer_sequence<
      typename make_int_seq_impl<T, I + 1, N>::type>::type;
};

template <class T, T N> struct make_int_seq_impl<T, N, N> {
  using type = integer_sequence<T>;
};

template <std::size_t... Ints>
using index_sequence = integer_sequence<std::size_t, Ints...>;

template <std::size_t N>
using make_index_sequence = make_integer_sequence<std::size_t, N>;

class template_repo_base;
class MFAST_CODER_EXPORT dictionary_builder
    : private field_instruction_visitor {
public:
  dictionary_builder(template_repo_base &repo_base);

  template <typename Operation>
  void build(const Operation &op, const templates_description *def) {
    current_ns_ = def->template_ns();
    current_dictionary_ =
        (def->dictionary()[0] == 0) ? "global" : def->dictionary();
    for (auto &elem : *def) {
      if (elem->id() > 0) {
        op(this->clone_instruction(elem));
      }
    }
  }

  template <typename Operation, typename... T>
  void build_from_descriptions(const Operation &op, T... descs) {
    pass(build(op, descs)...);
  }

private:
  template <typename Operation, typename Description>
  int build(const Operation &op, const Description *desc) {
    this->current_ns_ = desc->template_ns();
    this->current_dictionary_ =
        (desc->dictionary()[0] == 0) ? "global" : desc->dictionary();

    // using MessagePointers = mp_transform<typename Description::types,
    // std::add_pointer_t>;
    using Messages = typename Description::types;
    this->build_message_infos<Messages>(
        op, make_index_sequence<std::tuple_size<Messages>::value>{});
    return 0;
  }

  template <typename Tuple, typename Operation, std::size_t... I>
  void build_message_infos(const Operation &op, const index_sequence<I...> &) {
    pass(this->build_message_info<typename std::tuple_element<I, Tuple>::type>(
        op)...);
  }

  template <typename Message, typename Operation>
  int build_message_info(const Operation &op) {
    op(this->clone_instruction(Message::instruction()),
       static_cast<Message *>(nullptr));
    return 0;
  }

  virtual void visit(const int32_field_instruction *, void *) override;
  virtual void visit(const uint32_field_instruction *, void *) override;
  virtual void visit(const int64_field_instruction *, void *) override;
  virtual void visit(const uint64_field_instruction *, void *) override;
  virtual void visit(const decimal_field_instruction *, void *) override;
  virtual void visit(const ascii_field_instruction *, void *) override;
  virtual void visit(const unicode_field_instruction *, void *) override;
  virtual void visit(const byte_vector_field_instruction *, void *) override;
  virtual void visit(const int32_vector_field_instruction *, void *) override;
  virtual void visit(const uint32_vector_field_instruction *, void *) override;
  virtual void visit(const int64_vector_field_instruction *, void *) override;
  virtual void visit(const uint64_vector_field_instruction *, void *) override;
  virtual void visit(const group_field_instruction *, void *) override;
  virtual void visit(const sequence_field_instruction *, void *) override;
  virtual void visit(const template_instruction *, void *) override;
  virtual void visit(const templateref_instruction *, void *) override;

  virtual void visit(const enum_field_instruction *, void *) override;

  template_instruction *clone_instruction(const template_instruction *);

  void build_group(const field_instruction *fi,
                   const group_field_instruction *src,
                   group_field_instruction *dest);

  value_storage *get_dictionary_storage(const char *key, const char *ns,
                                        const op_context_t *op_context,
                                        field_type_enum_t field_type,
                                        value_storage *candidate_storage,
                                        field_instruction *instruction);

  template_instruction *find_template(uint32_t template_id);

  struct indexer_value_type {
    field_type_enum_t field_type_;
    field_instruction *instruction_;
    value_storage *storage_;
  };

  typedef std::map<std::string, indexer_value_type> indexer_t;
  indexer_t indexer_;
  std::string current_template_;
  std::string current_type_;
  const char *current_ns_;
  const char *current_dictionary_;

  template_repo_base &repo_base_;
  arena_allocator &alloc_;
};
}
