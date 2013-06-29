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
#ifndef DICTIONARY_BUILDER_H_F26FXFII
#define DICTIONARY_BUILDER_H_F26FXFII

#include "field_instruction.h"
#include "arena_allocator.h"
#include <vector>
#include <map>
#include <string>
#include <stdexcept>

namespace mfast {

// field value is use to store the previous values inside dictionaries.

inline bool is_empty_string(const char* str)
{
  return str == 0 || str[0] == '\0';
}


class dictionary_resetter
{
public:

  void push_back(value_storage_t* entry)
  {
    impl_.push_back(entry);
  }

  void reset()
  {
    for (std::size_t i = 0; i < impl_.size(); ++i) {
      impl_[i]->defined(false);
    }
  }

  std::size_t size() const {
    return impl_.size();
  }

private:
  std::vector<value_storage_t*> impl_;
};

class dictionary_builder
  : private field_instruction_visitor
{
  public:

    dictionary_builder(dictionary_resetter& resetter, arena_allocator* allocator);

    void build(const templates_description* def);

    typedef std::map<uint32_t, template_instruction*> template_id_map_t;
    const template_id_map_t& templates_map() const {
      return template_id_map_;
    }
  private:

    virtual void visit(const int32_field_instruction*, void*);
    virtual void visit(const uint32_field_instruction*, void*);
    virtual void visit(const int64_field_instruction*, void*);
    virtual void visit(const uint64_field_instruction*, void*);
    virtual void visit(const decimal_field_instruction*, void*);
    virtual void visit(const ascii_field_instruction*, void*);
    virtual void visit(const unicode_field_instruction*, void*);
    virtual void visit(const byte_vector_field_instruction*, void*);
    virtual void visit(const group_field_instruction*, void*);
    virtual void visit(const sequence_field_instruction*, void*);
    virtual void visit(const template_instruction*, void*);
    virtual void visit(const templateref_instruction*, void*);
    void build_template(const template_instruction*, void*);

    void build_group(const field_instruction*    fi,
                     const group_content_helper* src,
                     group_content_helper*       dest);

    value_storage_t*
    get_dictionary_storage(const char*         key,
                           const char*         ns,
                           const op_context_t* op_context,
                           field_type_enum_t   field_type,
                           value_storage_t*    candidate_storage);

    template_instruction* find_template(uint32_t template_id);

    struct indexer_value_type
    {
      field_type_enum_t field_type_;
      value_storage_t*  storage_;
    };

    dictionary_resetter& resetter_;
    typedef std::map<std::string, indexer_value_type>  indexer_t;
    indexer_t indexer_;
    arena_allocator* alloc_;
    std::string current_template_;
    std::string current_type_;
    const char* current_ns_;
    const char* current_dictionary_;
    template_id_map_t template_id_map_;
    typedef std::map<std::string, template_instruction*> template_name_map_t;
    template_name_map_t template_name_map_;
};


}


#endif /* end of include guard: DICTIONARY_BUILDER_H_F26FXFII */
