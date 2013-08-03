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
#include <boost/container/map.hpp>
#include "mfast/decoder.h"
#include "mfast/field_visitor.h"
#include "mfast/sequence_ref.h"
#include "mfast/dictionary_builder.h"
#include "mfast/malloc_allocator.h"
#include "mfast/exceptions.h"
#include "mfast/debug_stream.h"
#include "mfast/output.h"
#include "mfast/decoder/decoder_presence_map.h"
#include "mfast/decoder/decoder_field_operator.h"
#include "mfast/decoder/fast_istream.h"
#include "mfast/composite_field.h"
namespace mfast {

typedef boost::container::map<uint32_t, message_type> message_map_t;

struct decoder_impl
  : field_mutator_base
{
  template <typename MREF>
  class mref_mixin
    : public MREF
  {
    public:
      mref_mixin(allocator*                      alloc,
                 value_storage*                  storage,
                 typename MREF::instruction_cptr inst)
        : MREF(alloc, storage, inst)
      {
      }

      template <typename T>
      mref_mixin(T t)
        : MREF(t)
      {
      }

      mref_mixin()
      {
      }

      void decode_pmap(decoder_impl* coder)
      {
        this->prev_pmap_ = coder->current_;
        coder->current_ = &pmap_;
        coder->strm_.decode(pmap_);
      }

      void restore_pmap(decoder_impl* coder)
      {
        if (this->prev_pmap_)
          coder->current_ = this->prev_pmap_;
      }

    private:
      decoder_presence_map pmap_;
      decoder_presence_map* prev_pmap_;
  };

  typedef mref_mixin<mfast::group_mref> group_ref_type;
  typedef mref_mixin<mfast::dynamic_message_mref> dynamic_message_ref_type;
  typedef index_mixin<mref_mixin<mfast::sequence_element_mref> > sequence_element_ref_type;

  fast_istream strm_;
  dictionary_resetter resetter_;

  arena_allocator template_alloc_;  // template_alloc MUST be constructed before template_messages,
  message_map_t template_messages_; // Do not change the order of the two

  allocator* message_alloc_;
  message_type* active_message_;
  bool force_reset_;
  debug_stream debug_;
  decoder_presence_map* current_;
  std::ostream* warning_log_;

  decoder_impl();
  ~decoder_impl();
  void reset_messages();
  decoder_presence_map& current_pmap();

  template <typename SimpleMRef>
  void visit(SimpleMRef &mref);

  bool pre_visit(group_ref_type& mref);
  void post_visit(group_ref_type& mref);
  bool pre_visit(sequence_mref& mref);
  void post_visit(sequence_mref&);

  bool pre_visit(sequence_element_ref_type& mref);
  void post_visit(sequence_element_ref_type& mref);
  bool pre_visit(const message_mref&);
  void post_visit(const message_mref&);
  bool pre_visit(dynamic_message_ref_type&);
  void post_visit(dynamic_message_ref_type&);

  message_type*  decode_segment(fast_istreambuf& sb);
};


inline
decoder_impl::decoder_impl()
  : strm_(0)
  , warning_log_(0)
{
}

void decoder_impl::reset_messages()
{
  if (message_alloc_->reset()) {
    message_map_t::iterator itr;
    for (itr = template_messages_.begin(); itr!= template_messages_.end(); ++itr) {
      itr->second.ref().reset();
    }
  }
}

decoder_impl::~decoder_impl()
{
  reset_messages();
}

inline decoder_presence_map&
decoder_impl::current_pmap()
{
  return *current_;
}

template <typename SimpleMRef>
inline void
decoder_impl::visit(SimpleMRef& mref)
{
  debug_ << "   decoding " << mref.name() << ": pmap -> " << current_pmap() << "\n"
         << "               stream -> " << strm_ << "\n";

  decoder_field_operator* field_operator
    = decoder_operators[mref.instruction()->field_operator()];
  field_operator->decode(mref,
                         strm_,
                         current_pmap());

  if (mref.present())
    debug_ << "   decoded " << mref.name() << " = " << mref << "\n";
  else
    debug_ << "   decoded " << mref.name() << " is absent\n";
}

inline bool
decoder_impl::pre_visit(group_ref_type& mref)
{
  debug_ << "decoding group " << mref.name();

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
    mref.decode_pmap(this);
    debug_ << "        " << mref.name() << " has group pmap -> " << current_pmap() << "\n";
  }
  mref.ensure_valid();
  return true;
}

inline void
decoder_impl::post_visit(group_ref_type& mref)
{
  mref.restore_pmap(this);
}

inline bool
decoder_impl::pre_visit(sequence_mref& mref)
{
  debug_ << "decoding sequence " << mref.name()  << " ---\n";

  uint32_field_instruction* length_instruction = mref.instruction()->sequence_length_instruction_;
  value_storage storage;

  debug_ << "  decoding sequence length " << mref.name()  << " : stream -> " << strm_ << "\n";
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
decoder_impl::post_visit(sequence_mref&)
{
}

inline bool
decoder_impl::pre_visit(decoder_impl::sequence_element_ref_type& mref)
{
  debug_ << "decoding  element[" << mref.index << "] : has pmap bit = " <<  mref.instruction()->has_pmap_bit() << "\n";

  if (mref.instruction()->has_pmap_bit())
  {
    mref.decode_pmap(this);
    debug_ << "    decoded pmap -> " <<  current_pmap() << "\n";
  }
  return true;
}

inline void
decoder_impl::post_visit( //std::size_t /* index */,
  decoder_impl::sequence_element_ref_type& mref)
{
  mref.restore_pmap(this);
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
decoder_impl::pre_visit(decoder_impl::dynamic_message_ref_type& mref)
{
  debug_ << "decoding dynamic templateRef ...\n";

  mref.decode_pmap(this);

  debug_ << "   decoded pmap -> " << current_pmap() << "\n";

  decoder_presence_map& pmap = current_pmap();

  if (pmap.is_next_bit_set()) {
    uint32_t template_id;

    strm_.decode(template_id, false);
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
decoder_impl::post_visit(decoder_impl::dynamic_message_ref_type& mref)
{
  mref.restore_pmap(this);
}

message_type*
decoder_impl::decode_segment(fast_istreambuf& sb)
{

  strm_.reset(&sb);

  decoder_presence_map pmap;
  this->current_ = &pmap;
  strm_.decode(pmap);

  debug_ << "decoding segment : pmap -> " << pmap << "\n"
         << "                   entity -> " << strm_  << "\n";


  if (pmap.is_next_bit_set()) {
    uint32_t template_id;

    strm_.decode(template_id, false);

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

  if (force_reset_ || active_message_->instruction()->has_reset_attribute()) {
    resetter_.reset();
    reset_messages();
  }

  // we have to keep the active_message_ in a new variable
  // because after the accept_mutator(), the active_message_
  // may change because of the decoding of dynamic template reference
  message_type* message = active_message_;
  message->ref().ensure_valid();
  message->ref().accept_mutator(*this);
  return message;
}

decoder::decoder(allocator* alloc)
  : impl_(new decoder_impl)
{
  impl_->message_alloc_ = alloc;
}

decoder::~decoder()
{
  delete impl_;
}

void
decoder::include(const templates_description** descriptions, std::size_t description_count)
{
  template_id_map_t templates_map;
  dictionary_builder builder(impl_->resetter_,templates_map,  &impl_->template_alloc_);

  for (std::size_t i = 0; i < description_count; ++i)
    builder.build(descriptions[i]);

  // Given the template definitions, we need to create another map for
  // mapping each template id to a fully constructed message.
  template_id_map_t::const_iterator it = templates_map.begin();
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
decoder::decode(const char*& first, const char* last, bool force_reset)
{
  assert(first < last);
  fast_istreambuf sb(first, last-first);
  impl_->force_reset_ = force_reset;
  message_cref result = impl_->decode_segment(sb)->cref();
  first = sb.gptr();
  return result;
}

void
decoder::debug_log(std::ostream* log)
{
  impl_->debug_.set(log);
}

void
decoder::warning_log(std::ostream* os)
{
  impl_->strm_.warning_log(os);
}

}
