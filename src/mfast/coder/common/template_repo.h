// Copyright (c) 2016, Huang-Ming Huang,  Object Computing, Inc.
// All rights reserved.
//
// This file is part of mFAST.
// See the file license.txt for licensing information.
#pragma once

#include "dictionary_builder.h"
#include <map>

namespace mfast {
class template_repo_base {
public:
  template_repo_base(mfast::allocator *dictionary_alloc)
      : dictionary_alloc_(dictionary_alloc) {}
  virtual ~template_repo_base() {
    for (auto &elem : vector_enties_) {
      if (elem->of_array.capacity_in_bytes_)
        dictionary_alloc_->deallocate(elem->of_array.content_,
                                      elem->of_array.capacity_in_bytes_);
    }
  }

  void reset_dictionary() {
    for (auto &elem : reset_entries_) {
      elem->defined(false);
    }
  }

  virtual template_instruction *get_template(uint32_t id) = 0;

private:
  void add_reset_entry(value_storage *entry) {
    reset_entries_.push_back(entry);
  }
  void add_vector_entry(value_storage *entry) {
    if (dictionary_alloc_)
      vector_enties_.push_back(entry);
  }

protected:
  friend class dictionary_builder;

  typedef std::vector<value_storage *> value_entries_t;
  value_entries_t reset_entries_;
  value_entries_t vector_enties_; // for string and byteVector
  arena_allocator instruction_alloc_;
  mfast::allocator *dictionary_alloc_;
};

template <typename EntryValueConverter>
class template_repo : public template_repo_base {
  typedef typename EntryValueConverter::repo_mapped_type repo_mapped_type;
  typedef std::map<uint32_t, repo_mapped_type> templates_map_t;

  struct repo_entry_inserter {
    template_repo<EntryValueConverter> *repo_;
    repo_entry_inserter(template_repo<EntryValueConverter> *repo)
        : repo_(repo) {}
    void operator()(template_instruction *inst) const {
      repo_->add_template(inst, (void *)nullptr);
    }

    template <typename Message>
    void operator()(template_instruction *inst, Message *msg) const {
      repo_->add_template(inst, msg);
    }
  };

public:
  template_repo(mfast::allocator *dictionary_alloc = nullptr)
      : template_repo_base(dictionary_alloc) {}

  template_repo(EntryValueConverter converter,
                mfast::allocator *dictionary_alloc = nullptr)
      : template_repo_base(dictionary_alloc), converter_(std::move(converter)) {
  }

  void build(const templates_description *const *descriptions,
             std::size_t description_count) {
    dictionary_builder builder(*this);
    repo_entry_inserter inserter(this);

    for (std::size_t i = 0; i < description_count; ++i)
      builder.build(inserter, descriptions[i]);
  }

  template <typename Iterator> void build(Iterator first, Iterator last) {
    dictionary_builder builder(*this);
    repo_entry_inserter inserter(this);

    for (Iterator i = first; i != last; ++i)
      builder.build(inserter,
                    static_cast<const templates_description *>(&(*i)));
  }

  template <typename... T> void build(T... desc) {
    dictionary_builder builder(*this);
    builder.build_from_descriptions(repo_entry_inserter(this), desc...);
  }

  repo_mapped_type *find(uint32_t id) {
    auto it = templates_map_.find(id);
    if (it != templates_map_.end())
      return &it->second;
    return nullptr;
  }

  template_instruction *get_template(uint32_t id) override {
    repo_mapped_type *entry = this->find(id);
    if (entry) {
      return converter_.to_instruction(*entry);
    }
    return nullptr;
  }

  repo_mapped_type *unique_entry() {
    if (templates_map_.size() == 1) {
      return &templates_map_.begin()->second;
    }
    return nullptr;
  }

  template <typename Message>
  void add_template(template_instruction *inst, Message *msg) {
    // assert(dynamic_cast<const typename
    // Message::instruction_type*>(templates_map_[Message::the_id]) );
    this->templates_map_.emplace(inst->id(),
                                 converter_.to_repo_entry(inst, msg));
  }

protected:
  templates_map_t templates_map_;
  EntryValueConverter converter_;
};

struct trivial_template_repo_entry_converter {
  typedef template_instruction *repo_mapped_type;
  template_instruction *to_instruction(template_instruction *inst) const {
    return inst;
  }
  template_instruction *to_repo_entry(template_instruction *inst,
                                      void *) const {
    return inst;
  }
};

typedef template_repo<trivial_template_repo_entry_converter>
    simple_template_repo_t;
}
