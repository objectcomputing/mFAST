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
#ifndef DICTIONARY_BUILDER_H_F26FXFII
#define DICTIONARY_BUILDER_H_F26FXFII

#include "mfast/field_instructions.h"
#include "mfast/arena_allocator.h"
#include "mfast/coder/mfast_coder_export.h"
#include <vector>
#include <map>
#include <string>
#include <stdexcept>

#ifdef _MSC_VER
#pragma warning(disable: 4251) // non dll-interface class used as a member for dll-interface class
#endif //_MSC_VER


namespace mfast {

// field value is use to store the previous values inside dictionaries.


  class dictionary_resetter
  {
  public:

    void push_back(value_storage* entry)
    {
      impl_.push_back(entry);
    }

    void reset()
    {
      for (std::size_t i = 0; i < impl_.size(); ++i) {
        impl_[i]->defined(false);
      }
    }

    std::size_t size() const
    {
      return impl_.size();
    }

  private:
    std::vector<value_storage*> impl_;
  };


  class dictionary_value_destroyer
  {
  public:
    dictionary_value_destroyer(allocator* alloc)
      : alloc_(alloc)
    {
    }

    ~dictionary_value_destroyer()
    {
      for (std::size_t i = 0; i < impl_.size(); ++i) {
        if (impl_[i]->of_array.capacity_in_bytes_)
          alloc_->deallocate(impl_[i]->of_array.content_,
                             impl_[i]->of_array.capacity_in_bytes_);
      }
    }

    void push_back(value_storage* entry)
    {
      impl_.push_back(entry);
    }

    std::size_t size() const
    {
      return impl_.size();
    }

  private:
    allocator* alloc_;
    std::vector<value_storage*> impl_;
  };


  typedef std::map<uint32_t, template_instruction*> template_id_map_t;


  class MFAST_CODER_EXPORT dictionary_builder
    : private field_instruction_visitor
  {
  public:

    dictionary_builder(dictionary_resetter&        resetter,
                       template_id_map_t&          templates_map,
                       arena_allocator*            allocator,
                       dictionary_value_destroyer* value_destroyer=0);

    void build(const templates_description* def);

  private:

    virtual void visit(const int32_field_instruction*, void*);
    virtual void visit(const uint32_field_instruction*, void*);
    virtual void visit(const int64_field_instruction*, void*);
    virtual void visit(const uint64_field_instruction*, void*);
    virtual void visit(const decimal_field_instruction*, void*);
    virtual void visit(const ascii_field_instruction*, void*);
    virtual void visit(const unicode_field_instruction*, void*);
    virtual void visit(const byte_vector_field_instruction*, void*);
    virtual void visit(const int32_vector_field_instruction*, void*);
    virtual void visit(const uint32_vector_field_instruction*, void*);
    virtual void visit(const int64_vector_field_instruction*, void*);
    virtual void visit(const uint64_vector_field_instruction*, void*);
    virtual void visit(const group_field_instruction*, void*);
    virtual void visit(const sequence_field_instruction*, void*);
    virtual void visit(const template_instruction*, void*);
    virtual void visit(const templateref_instruction*, void*);

    virtual void visit(const enum_field_instruction*, void*);

    void build_template(const template_instruction*, void*);

    void build_group(const field_instruction*       fi,
                     const group_field_instruction* src,
                     group_field_instruction*       dest);

    value_storage*
    get_dictionary_storage(const char*         key,
                           const char*         ns,
                           const op_context_t* op_context,
                           field_type_enum_t   field_type,
                           value_storage*      candidate_storage);

    template_instruction* find_template(uint32_t template_id);

    struct indexer_value_type
    {
      field_type_enum_t field_type_;
      value_storage*  storage_;
    };

    dictionary_resetter& resetter_;
    typedef std::map<std::string, indexer_value_type>  indexer_t;
    indexer_t indexer_;
    arena_allocator* alloc_;
    std::string current_template_;
    std::string current_type_;
    const char* current_ns_;
    const char* current_dictionary_;
    template_id_map_t& template_id_map_;
    typedef std::map<std::string, template_instruction*> template_name_map_t;
    template_name_map_t template_name_map_;
    dictionary_value_destroyer* value_destroyer_;
  };


}


#endif /* end of include guard: DICTIONARY_BUILDER_H_F26FXFII */
