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
#include <deque>
#include "decoder.h"
#include "decoder_field_operator.h"
#include "field_visitor.h"
#include <boost/container/map.hpp>
#include "presence_map.h"
#include "sequence_ref.h"
#include "dictionary_builder.h"
#include "malloc_allocator.h"
#include "exceptions.h"
#include "debug_stream.h"
#include "output.h"

namespace mfast {

typedef boost::container::map<uint32_t, message_base> message_map_t;

struct decoder_impl
{
  std::deque<presence_map> pmap_stack_;
  fast_istream* strm_;
  allocator* alloc_;
  dictionary_resetter resetter_;

  arena_allocator template_alloc_;  // template_alloc MUST be constructed before template_messages,
  message_map_t template_messages_; // Do not chnage the order of the two

  allocator* message_alloc_;
  message_base* active_message_;
  bool force_reset_;
  debug_stream debug_;

  decoder_impl();
  ~decoder_impl();
  void reset_messages();
  presence_map& current_pmap();

  template <typename SimpleMRef>
  void visit(const SimpleMRef &mref);
  bool pre_visit(const group_mref& mref);
  void post_visit(const group_mref& mref);
  bool pre_visit(const sequence_mref& mref);
  void post_visit(const sequence_mref&);
  bool pre_visit(std::size_t /* index */, const sequence_element_mref& mref);
  void post_visit(std::size_t /* index */, const sequence_element_mref& mref);
  bool pre_visit(const message_mref&);
  void post_visit(const message_mref&);
  bool pre_visit(dynamic_mref&);
  void post_visit(const dynamic_mref&);

  message_base*  decode_segment(fast_istream* strm);
};


inline
decoder_impl::decoder_impl()
  : strm_(0)
{
}

void decoder_impl::reset_messages()
{
  if (message_alloc_->reset()) {
    message_map_t::iterator itr;
    for (itr = template_messages_.begin(); itr!= template_messages_.end(); ++itr) {
      itr->second.reset();
    }
  }
}

decoder_impl::~decoder_impl()
{
  reset_messages();
}

inline presence_map&
decoder_impl::current_pmap()
{
  return pmap_stack_.back();
}

template <typename SimpleMRef>
inline void
decoder_impl::visit(const SimpleMRef& mref)
{
  debug_ << "   decoding " << mref.name() << ": pmap -> " << current_pmap() << "\n"
         << "               stream -> " << *strm_ << "\n";

  decoder_field_operator* field_operator
    = decoder_operators[mref.instruction()->field_operator()];
  field_operator->decode(mref,
                         *strm_,
                         current_pmap());

  if (mref.present())
    debug_ << "   decoded " << mref.name() << " = " << mref << "\n";
  else
    debug_ << "   decoded " << mref.name() << " is absent\n";
}

inline bool
decoder_impl::pre_visit(const group_mref& mref)
{
  debug_ << "decoding group " << mref.name() ;

  // If a group field is optional, it will occupy a single bit in the presence map.
  // The contents of the group may appear in the stream iff the bit is set.

  if (mref.optional())
  {
    debug_ << " : current pmap = " << current_pmap() << "\n";
    if (!current_pmap().is_next_bit_set()) {
      debug_ << "        " << mref.name() << " is absent\n";
      mref.as_absent();
      return false;
    }
  }
  else {
    debug_ << " : mandatory\n";
  }

  if (mref.instruction()->has_pmap_bit()) {
    pmap_stack_.resize(pmap_stack_.size()+1);
    strm_->decode(current_pmap());
    debug_ << "        " << mref.name() << " has group pmap -> " << current_pmap() << "\n";
  }
  mref.ensure_valid();
  return true;
}

inline void
decoder_impl::post_visit(const group_mref& mref)
{
  if (mref.instruction()->has_pmap_bit())
    pmap_stack_.pop_back();
}

inline bool
decoder_impl::pre_visit(const sequence_mref& mref)
{
  debug_ << "decoding sequence " << mref.name()  << " ---\n";

  uint32_field_instruction* length_instruction = mref.instruction()->sequence_length_instruction_;
  value_storage_t storage;

  debug_ << "  decoding sequence length " << mref.name()  << " : stream -> " << *strm_ << "\n";
  uint32_mref length_mref(0, &storage, length_instruction);
  this->visit(length_mref);


  if (length_mref.present()) {
    debug_ << "  decoded sequence length " << length_mref.value() << "\n";
    mref.resize(length_mref.value());
  }
  else {
    debug_ << "  " << mref.name() << "is absent\n";
    mref.as_absent();
  }
  return length_mref.present() && length_mref.value() > 0;
}

inline void
decoder_impl::post_visit(const sequence_mref&)
{
}

inline bool
decoder_impl::pre_visit(std::size_t index, const sequence_element_mref& mref)
{
  debug_ << "decoding  element[" << index << "] : has pmap bit = " <<  mref.instruction()->has_pmap_bit() << "\n";

  if (mref.instruction()->has_pmap_bit())
  {
    pmap_stack_.resize(pmap_stack_.size()+1);
    strm_->decode(current_pmap());
    debug_ << "    decoded pmap -> " <<  current_pmap() << "\n";
  }
  return true;
}

inline void
decoder_impl::post_visit(std::size_t /* index */, const sequence_element_mref& mref)
{
  if (mref.instruction()->has_pmap_bit()) {
    pmap_stack_.pop_back();
  }
}

inline bool
decoder_impl::pre_visit(const message_mref& mref)
{
  mref.ensure_valid();
  debug_ << "decoding template " << mref.name()  << " ...\n";

  return true;
}

inline void
decoder_impl::post_visit(const message_mref&)
{
}

inline bool
decoder_impl::pre_visit(dynamic_mref& mref)
{
  debug_ << "decoding dynamic templateRef ...\n";

  strm_->decode(current_pmap());
  debug_ << "   decoded pmap -> " << current_pmap() << "\n";

  presence_map& pmap = current_pmap();

  if (pmap.is_next_bit_set()) {
    uint32_t template_id;

    strm_->decode(template_id, false);
    debug_ << "   decoded template id -> " << template_id << "\n";

    // find the message with corresponding template id
    message_map_t::iterator itr = template_messages_.find(template_id);
    if (itr != template_messages_.end())
    {
      active_message_ = &itr->second;
    }
    else {
      BOOST_THROW_EXCEPTION(fast_dynamic_error("D9") << template_id_info(template_id)
                                                     << referenced_by_info(active_message_->name()));
    }
  }
  mref.set_instruction(active_message_->instruction(), false);
  return true;
}

inline void
decoder_impl::post_visit(const dynamic_mref& mref)
{
  pmap_stack_.pop_back();
}

message_base*
decoder_impl::decode_segment(fast_istream* strm)
{
  strm_ = strm;
  pmap_stack_.resize(1);
  strm_->decode(current_pmap());

  presence_map& pmap = current_pmap();

  debug_ << "decoding segment : pmap -> " << pmap << "\n"
         << "                   entity -> " << *strm  << "\n";


  if (pmap.is_next_bit_set()) {
    uint32_t template_id;

    strm_->decode(template_id, false);

    debug_ << "decoded template id = " << template_id << "\n";

    // find the message with corresponding template id
    message_map_t::iterator itr = template_messages_.find(template_id);
    if (itr != template_messages_.end())
    {
      active_message_ = &itr->second;
    }
    else {
      BOOST_THROW_EXCEPTION(fast_dynamic_error("D9") << template_id_info(template_id));
    }
  }

  if (force_reset_ || active_message_->instruction()->has_reset_attribute()){
    resetter_.reset();
    reset_messages();
  }

  // we have to keep the active_message_ in a new variable
  // because after the accept_mutator(), the active_message_
  // may change because of the decoding of dynamic template reference
  message_base* message = active_message_;
  message->ensure_valid();
  message->ref().accept_mutator(*this);
  return message;
}

decoder::decoder()
  : impl_(new decoder_impl)
{
  impl_->message_alloc_ = malloc_allocator::instance();
}

decoder::decoder(allocator& alloc)
  : impl_(new decoder_impl)
{
  impl_->message_alloc_ = &alloc;
}

decoder::~decoder()
{
  delete impl_;
}

void
decoder::include(const templates_description** descriptions, std::size_t description_count)
{
  dictionary_builder builder(impl_->resetter_, &impl_->template_alloc_);

  for (std::size_t i = 0; i < description_count; ++i)
    builder.build(descriptions[i]);

  typedef dictionary_builder::template_id_map_t templates_map_t;
  const templates_map_t& templates_map = builder.templates_map();

  // Given the template definitions, we need to create another map for
  // mapping each template id to a fully constructed message.
  templates_map_t::const_iterator it = templates_map.begin();
  for (; it != templates_map.end(); ++it) {
    impl_->template_messages_.emplace(it->first, std::make_pair(impl_->message_alloc_, it->second));
  }

  if (impl_->template_messages_.size()==1) {
    impl_->active_message_ = &(impl_->template_messages_.begin()->second);
  }
  else {
    impl_->active_message_ = 0;
  }
}

message_cref
decoder::decode(fast_istream& strm, bool force_reset)
{

  assert(strm.in_avail() != 0);

  impl_->force_reset_ = force_reset;
  return impl_->decode_segment(&strm)->cref();
}

#ifndef NDEBUG
void
decoder::debug_log(std::ostream& log)
{
  impl_->debug_.set(log);
}
#endif

}
