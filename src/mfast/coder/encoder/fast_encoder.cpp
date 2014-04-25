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

#include "../mfast_coder_export.h"
#include "mfast/field_visitor.h"
#include "mfast/sequence_ref.h"
#include "mfast/malloc_allocator.h"
#include "../fast_encoder.h"
#include "../common/dictionary_builder.h"
#include "../common/exceptions.h"
#include "mfast/output.h"
#include "encoder_presence_map.h"
#include "encoder_field_operator.h"
#include "fast_ostream.h"
#include "resizable_fast_ostreambuf.h"

namespace mfast
{


  struct fast_encoder_impl
  {

    enum {
      visit_absent = 1
    };

    fast_ostream strm_;
    dictionary_resetter resetter_;
    dictionary_value_destroyer value_destroyer_;

    arena_allocator template_alloc_;

    int64_t active_message_id_;
    encoder_presence_map* current_;
    template_id_map_t templates_map_;


    fast_encoder_impl(allocator* alloc);
    ~fast_encoder_impl();
    encoder_presence_map& current_pmap();


    struct pmap_state
    {
      encoder_presence_map pmap_;
      encoder_presence_map* prev_pmap_;

      pmap_state()
        : prev_pmap_(0)
      {
      }

    };


    void setup_pmap(pmap_state& state, std::size_t sz)
    {
      state.prev_pmap_ = this->current_;
      this->current_ = &state.pmap_;
      state.pmap_.init(&this->strm_, sz);
    }

    void commit_pmap(pmap_state& state)
    {
      if (state.prev_pmap_) {
        state.pmap_.commit();
        this->current_ = state.prev_pmap_;
      }
    }

    void visit(enum_cref &cref)
    {
      this->visit(reinterpret_cast<uint64_cref&>(cref));
    }

    template <typename SimpleCRef>
    void visit(SimpleCRef &cref);

    template <typename IntType>
    void visit(int_vector_cref<IntType> &cref);

    void visit(group_cref& cref, int);
    void visit(sequence_cref&, int);
    void visit(sequence_element_cref& cref, int);
    void visit(nested_message_cref&, int);

    template_instruction*  encode_segment_preemble(uint32_t template_id, bool force_reset);
    void encode_segment(const message_cref& cref, fast_ostreambuf& sb, bool force_reset);
  };

  inline
  fast_encoder_impl::fast_encoder_impl(allocator* alloc)
    : strm_(alloc)
    , value_destroyer_(alloc)
    , active_message_id_(-1)
  {
  }

  fast_encoder_impl::~fast_encoder_impl()
  {
  }

  inline encoder_presence_map&
  fast_encoder_impl::current_pmap()
  {
    return *current_;
  }

  template <typename SimpleCRef>
  inline void
  fast_encoder_impl::visit(SimpleCRef& cref)
  {

    const encoder_field_operator* field_operator
      = encoder_operators[cref.instruction()->field_operator()];
    field_operator->encode(cref,
                           strm_,
                           current_pmap());

  }

  template <typename IntType>
  void fast_encoder_impl::visit(int_vector_cref<IntType> &cref)
  {
    strm_.encode(static_cast<uint32_t>(cref.size()), cref.optional(), !cref.present());
    if (cref.present()) {
      for (std::size_t i = 0; i < cref.size(); ++i) {
        strm_.encode(cref[i], false, false);
      }
    }
  }

  inline void
  fast_encoder_impl::visit(group_cref& cref, int)
  {

    // If a group field is optional, it will occupy a single bit in the presence map.
    // The contents of the group may appear in the stream iff the bit is set.

    if (cref.optional())
    {
      current_pmap().set_next_bit(cref.present());

      if (cref.absent())
        return;
    }

    pmap_state state;

    if (cref.instruction()->segment_pmap_size() > 0) {
      setup_pmap(state, cref.instruction()->segment_pmap_size() );
    }

    cref.accept_accessor(*this);

    commit_pmap(state);
  }

  inline void
  fast_encoder_impl::visit(sequence_cref& cref, int)
  {

    value_storage storage;

    uint32_mref length_mref(0, &storage, cref.instruction()->length_instruction());

    if (cref.present())
      length_mref.as(cref.size());
    else
      length_mref.omit();

    this->visit(length_mref);

    if (length_mref.present() && length_mref.value() > 0)
      cref.accept_accessor(*this);

  }

  inline void
  fast_encoder_impl::visit(sequence_element_cref& cref, int)
  {
    pmap_state state;
    if (cref.instruction()->segment_pmap_size() > 0)
    {
      setup_pmap(state, cref.instruction()->segment_pmap_size());
    }
    cref.accept_accessor(*this);
    commit_pmap(state);
  }

  inline void
  fast_encoder_impl::visit(nested_message_cref& cref, int)
  {
    pmap_state state;
    int64_t saved_message_id = active_message_id_;

    state.prev_pmap_ = this->current_;
    this->current_ = &state.pmap_;

    // we have to replace the target instruction in cref so that the previous values of
    // the inner fields can be accessed.
    const template_instruction*& target_inst = const_cast<value_storage*>(field_cref_core_access::storage_of(cref))->of_templateref.of_instruction.instruction_;
    target_inst = encode_segment_preemble(target_inst->id(), false);

    cref.accept_accessor(*this);

    commit_pmap(state);
    active_message_id_ = saved_message_id;
  }

  template_instruction*
  fast_encoder_impl::encode_segment_preemble(uint32_t template_id, bool force_reset)
  {
    template_instruction* instruction;
    template_id_map_t::iterator itr = templates_map_.find(template_id);

    if (itr != templates_map_.end()) {
      instruction = itr->second;
      current_pmap().init(&this->strm_, instruction->segment_pmap_size());
    }
    else {
      using namespace coder;
      BOOST_THROW_EXCEPTION(fast_dynamic_error("D9") << template_id_info(template_id));
    }

    if ( force_reset ||  instruction->has_reset_attribute())
      resetter_.reset();


    bool need_encode_template_id = (active_message_id_ != template_id);
    current_pmap().set_next_bit(need_encode_template_id);

    if (need_encode_template_id)
    {
      active_message_id_ = template_id;
      strm_.encode(active_message_id_, false, false);
    }
    return instruction;
  }

  void
  fast_encoder_impl::encode_segment(const message_cref& cref, fast_ostreambuf& sb, bool force_reset)
  {
    this->strm_.rdbuf(&sb);

    encoder_presence_map pmap;
    this->current_ = &pmap;

    template_instruction* instruction = encode_segment_preemble(cref.id(), force_reset);

    aggregate_cref message(cref.field_storage(0), instruction);
    message.accept_accessor(*this);

    pmap.commit();
  }

  fast_encoder::fast_encoder(allocator* alloc)
    : impl_(new fast_encoder_impl(alloc))
  {
  }

  fast_encoder::~fast_encoder()
  {
    delete impl_;
  }

  void
  fast_encoder::include(const templates_description** descriptions, std::size_t description_count)
  {
    dictionary_builder builder(impl_->resetter_,
                               impl_->templates_map_,
                               &impl_->template_alloc_,
                               &impl_->value_destroyer_);

    for (std::size_t i = 0; i < description_count; ++i)
      builder.build(descriptions[i]);

    if (impl_->templates_map_.size() ==1 ) {
      impl_->active_message_id_ = impl_->templates_map_.begin()->first;
    }
  }

  std::size_t
  fast_encoder::encode(const message_cref& message,
                       char*               buffer,
                       std::size_t         buffer_size,
                       bool                force_reset)
  {
    assert(buffer_size > 0);

    fast_ostreambuf sb(buffer, buffer_size);
    impl_->encode_segment(message, sb, force_reset);
    return sb.length();
  }

  void
  fast_encoder::encode(const message_cref& message,
                       std::vector<char>&  buffer,
                       bool                force_reset)
  {
    resizable_fast_ostreambuf sb(buffer);
    impl_->encode_segment(message, sb, force_reset);
    buffer.resize(sb.length());
  }

  const template_instruction*
  fast_encoder::template_with_id(uint32_t id)
  {
    template_instruction* instruction =0;
    template_id_map_t::iterator itr = impl_->templates_map_.find(id);

    if (itr != impl_->templates_map_.end()) {
      instruction = itr->second;
    }
    return instruction;
  }

  void
  fast_encoder::allow_overlong_pmap(bool v)
  {
    impl_->strm_.allow_overlong_pmap(v);
  }

}
