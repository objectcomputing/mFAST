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

#include "mfast/encoder.h"
#include "mfast/field_visitor.h"
#include "mfast/sequence_ref.h"
#include "mfast/dictionary_builder.h"
#include "mfast/malloc_allocator.h"
#include "mfast/exceptions.h"
#include "mfast/debug_stream.h"
#include "mfast/output.h"
#include "mfast/encoder/encoder_presence_map.h"
#include "mfast/encoder/encoder_field_operator.h"
#include "mfast/encoder/fast_ostream.h"
#include "mfast/encoder/resizable_fast_ostreambuf.h"

namespace mfast
{

struct encoder_impl
  : field_accessor_base
{
  template <typename CREF>
  class cref_mixin
    : public CREF
  {
    public:


      cref_mixin(value_storage*                  storage,
                 typename CREF::instruction_cptr inst)
        : CREF(storage, inst)
      {
      }

      template <typename T>
      cref_mixin(T t)
        : CREF(t)
      {
      }

      cref_mixin()
      {
      }

      void setup_pmap(encoder_impl* coder)
      {
        this->prev_pmap_ = coder->current_;
        coder->current_ = &pmap_;
        pmap_.init(&coder->strm_, this->instruction()->pmap_size());
      }

      void commit_pmap(encoder_impl* coder)
      {
        if (this->prev_pmap_) {
          pmap_.commit();
          coder->current_ = this->prev_pmap_;
        }
      }

    private:
      encoder_presence_map pmap_;
      encoder_presence_map* prev_pmap_;
  };

  typedef cref_mixin<mfast::group_cref> group_ref_type;
  typedef cref_mixin<mfast::sequence_element_cref> sequence_element_ref_type;
  typedef cref_mixin<mfast::dynamic_cref> dynamic_ref_type;


  fast_ostream strm_;
  dictionary_resetter resetter_;
  dictionary_value_destroyer value_destroyer_;

  arena_allocator template_alloc_;

  int64_t active_message_id_;
  debug_stream debug_;
  encoder_presence_map* current_;
  template_id_map_t templates_map_;


  encoder_impl(allocator* alloc);
  ~encoder_impl();
  encoder_presence_map& current_pmap();

  template <typename SimpleCRef>
  void visit(SimpleCRef &cref);

  bool pre_visit(group_ref_type& cref);
  void post_visit(group_ref_type& cref);
  bool pre_visit(sequence_cref& cref);
  void post_visit(sequence_cref&);
  bool pre_visit(std::size_t /* index */,  sequence_element_ref_type& cref);
  void post_visit(std::size_t /* index */, sequence_element_ref_type& cref);
  bool pre_visit(const message_cref&);
  void post_visit(const message_cref&);
  bool pre_visit(dynamic_ref_type&);
  void post_visit(dynamic_ref_type&);

  template_instruction*  encode_segment_preemble(uint32_t template_id, bool force_reset);
  void encode_segment(const message_cref& cref, fast_ostreambuf& sb, bool force_reset);
};

inline
encoder_impl::encoder_impl(allocator* alloc)
  : strm_(alloc)
  , value_destroyer_(alloc)
  , active_message_id_(-1)  
{
}

encoder_impl::~encoder_impl()
{
}

inline encoder_presence_map&
encoder_impl::current_pmap()
{
  return *current_;
}

template <typename SimpleCRef>
inline void
encoder_impl::visit(SimpleCRef& cref)
{

  encoder_field_operator* field_operator
    = encoder_operators[cref.instruction()->field_operator()];
  field_operator->encode(cref,
                         strm_,
                         current_pmap());

}

inline bool
encoder_impl::pre_visit(group_ref_type& cref)
{

  // If a group field is optional, it will occupy a single bit in the presence map.
  // The contents of the group may appear in the stream iff the bit is set.

  if (cref.optional())
  {
    current_pmap().set_next_bit(cref.present());

    if (cref.absent())
      return false;
  }


  if (cref.instruction()->has_pmap_bit()) {
    cref.setup_pmap(this);
  }
  return true;
}

inline void
encoder_impl::post_visit(group_ref_type& cref)
{
  cref.commit_pmap(this);
}

inline bool
encoder_impl::pre_visit(sequence_cref& cref)
{

  uint32_field_instruction* length_instruction = cref.instruction()->sequence_length_instruction_;
  value_storage storage;

  uint32_mref length_mref(0, &storage, length_instruction);
  
  if (cref.present())
    length_mref.as(cref.size());
  else
    length_mref.as_absent();

  this->visit(length_mref);

  return length_mref.present() && length_mref.value() > 0;
}

inline void
encoder_impl::post_visit(sequence_cref&)
{
}

inline bool
encoder_impl::pre_visit(std::size_t /* index */, encoder_impl::sequence_element_ref_type& cref)
{
  if (cref.instruction()->has_pmap_bit())
  {
    cref.setup_pmap(this);
  }
  return true;
}

inline void
encoder_impl::post_visit(std::size_t /* index */, encoder_impl::sequence_element_ref_type& cref)
{
  cref.commit_pmap(this);
}

inline bool
encoder_impl::pre_visit(const message_cref&)
{
  return true;
}

inline void
encoder_impl::post_visit(const message_cref&)
{
}


inline bool
encoder_impl::pre_visit(encoder_impl::dynamic_ref_type& cref)
{
  cref.setup_pmap(this);
  cref.instruction_ = encode_segment_preemble(cref.instruction()->id(), false); 
  return true;
}

inline void
encoder_impl::post_visit(encoder_impl::dynamic_ref_type& cref)
{
  cref.commit_pmap(this);
}


template_instruction* 
encoder_impl::encode_segment_preemble(uint32_t template_id, bool force_reset)
{
  template_instruction* instruction;
  template_id_map_t::iterator itr = templates_map_.find(template_id);
  
  if (itr != templates_map_.end()) {
    instruction = itr->second;
  }
  else {
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
encoder_impl::encode_segment(const message_cref& cref, fast_ostreambuf& sb, bool force_reset)
{
  this->strm_.rdbuf(&sb);
  
  encoder_presence_map pmap;
  this->current_ = &pmap;
  pmap.init(&this->strm_, cref.instruction()->pmap_size());
  
  template_instruction* instruction = encode_segment_preemble(cref.id(), force_reset);
  
  message_cref message(cref.storage_, instruction); 
  message.accept_accessor(*this);
  
  pmap.commit();
}

encoder::encoder(allocator* alloc)
  : impl_(new encoder_impl(alloc))
{
}

encoder::~encoder()
{
  delete impl_;
}

void
encoder::include(const templates_description** descriptions, std::size_t description_count)
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
encoder::encode(const message_cref& message,
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
encoder::encode(const message_cref& message,
                std::vector<char>&  buffer,
                bool                force_reset)
{
  resizable_fast_ostreambuf sb(buffer);
  impl_->encode_segment(message, sb, force_reset);
  buffer.resize(sb.length());
}

const template_instruction* 
encoder::template_with_id(uint32_t id)
{
  template_instruction* instruction =0;
  template_id_map_t::iterator itr = impl_->templates_map_.find(id);
  
  if (itr != impl_->templates_map_.end()) {
    instruction = itr->second;
  }
  return instruction;
}

}
