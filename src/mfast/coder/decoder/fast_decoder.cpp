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
#include <boost/container/map.hpp>
#include "../mfast_coder_export.h"
#include "../fast_decoder.h"
#include "mfast/field_visitor.h"
#include "mfast/sequence_ref.h"
#include "mfast/malloc_allocator.h"
#include "mfast/output.h"
#include "mfast/composite_type.h"
#include "../common/exceptions.h"
#include "../common/debug_stream.h"
#include "../common/dictionary_builder.h"
#include "../common/codec_helper.h"
#include "decoder_presence_map.h"
#include "decoder_field_operator.h"
#include "fast_istream.h"
#include "mfast/vector_ref.h"

namespace mfast {

  typedef boost::container::map<uint32_t, message_type> message_map_t;

  struct fast_decoder_impl
  {

    enum {
      visit_absent = 1
    };

    fast_istream strm_;
    dictionary_resetter resetter_;

    arena_allocator template_alloc_; // template_alloc MUST be constructed before template_messages,
    message_map_t template_messages_; // Do not change the order of the two

    allocator* message_alloc_;
    message_type* active_message_;
    bool force_reset_;
    debug_stream debug_;
    decoder_presence_map* current_;
    std::ostream* warning_log_;

    fast_decoder_impl();
    ~fast_decoder_impl();
    void reset_messages();
    decoder_presence_map& current_pmap();


    struct pmap_state
    {
      decoder_presence_map pmap_;
      decoder_presence_map* prev_pmap_;

      pmap_state()
        : prev_pmap_(0)
      {
      }

    };

    void decode_pmap(pmap_state& state)
    {
      state.prev_pmap_ = this->current_;
      this->current_ = &state.pmap_;
      this->strm_.decode(state.pmap_);
    }

    void restore_pmap(pmap_state& state)
    {
      if (state.prev_pmap_)
        this->current_ = state.prev_pmap_;
    }

    void visit(enum_mref &mref)
    {
      this->visit(reinterpret_cast<uint64_mref&>(mref));
    }

    template <typename SimpleMRef>
    void visit(SimpleMRef &mref);

    template <typename IntType>
    void visit(int_vector_mref<IntType>& mref);

    void visit(group_mref& mref, int);
    void visit(sequence_mref& mref, int);
    void visit(nested_message_mref& mref, int);
    void visit(sequence_element_mref& mref, int);

    message_type*  decode_segment(fast_istreambuf& sb);
  };


  inline
  fast_decoder_impl::fast_decoder_impl()
    : strm_(0)
    , warning_log_(0)
  {
  }

  void fast_decoder_impl::reset_messages()
  {
    // if (message_alloc_->reset()) {
    //   message_map_t::iterator itr;
    //   for (itr = template_messages_.begin(); itr!= template_messages_.end(); ++itr) {
    //     itr->second.reset();
    //   }
    // }
  }

  fast_decoder_impl::~fast_decoder_impl()
  {
    reset_messages();
  }

  inline decoder_presence_map&
  fast_decoder_impl::current_pmap()
  {
    return *current_;
  }

  template <typename SimpleMRef>
  inline void
  fast_decoder_impl::visit(SimpleMRef& mref)
  {
    debug_ << "   decoding " << mref.name() << ": pmap -> " << current_pmap() << "\n"
           << "               stream -> " << strm_ << "\n";

    const decoder_field_operator* field_operator
      = decoder_operators[mref.instruction()->field_operator()];
    field_operator->decode(mref,
                           strm_,
                           current_pmap());

    if (mref.present())
      debug_ << "   decoded " << mref.name() << " = " << mref << "\n";
    else
      debug_ << "   decoded " << mref.name() << " is absent\n";
  }

  template <typename IntType>
  void fast_decoder_impl::visit(int_vector_mref<IntType> &mref)
  {
    debug_ << "decoding int vector " << mref.name();

    uint32_t length=0;
    if (!strm_.decode(length, mref.optional())) {
      mref.omit();
      return;
    }

    mref.resize(length);
    for (uint32_t i = 0; i < length; ++i) {
      strm_.decode(mref[i], false);
    }
  }

  inline void
  fast_decoder_impl::visit(group_mref& mref, int)
  {
    debug_ << "decoding group " << mref.name();

    // If a group field is optional, it will occupy a single bit in the presence map.
    // The contents of the group may appear in the stream iff the bit is set.
    if (mref.optional())
    {
      debug_ << " : current pmap = " << current_pmap() << "\n";
      if (!current_pmap().is_next_bit_set()) {
        debug_ << "        " << mref.name() << " is absent\n";
        mref.omit();
        return;
      }
    }
    else {
      debug_ << " : mandatory\n";
    }

    pmap_state state;

    if (mref.instruction()->segment_pmap_size() > 0) {
      decode_pmap(state);
      debug_ << "        " << mref.name() << " has group pmap -> " << current_pmap() << "\n";
    }

    mref.accept_mutator(*this);

    restore_pmap(state);
  }

  inline void
  fast_decoder_impl::visit(sequence_mref& mref, int)
  {
    debug_ << "decoding sequence " << mref.name()  << " ---\n";

    const uint32_field_instruction* length_instruction = mref.instruction()->length_instruction();
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
      mref.omit();
    }
    if (length_mref.present() && length_mref.value() > 0)
    {
      mref.accept_mutator(*this);
    }
  }

  inline void
  fast_decoder_impl::visit(sequence_element_mref& mref, int index)
  {
    debug_ << "decoding  element[" << index << "] : segment pmap size = " <<  mref.instruction()->segment_pmap_size() << "\n";

    pmap_state state;
    if (mref.instruction()->segment_pmap_size() > 0)
    {
      decode_pmap(state);
      debug_ << "    decoded pmap -> " <<  current_pmap() << "\n";
    }

    mref.accept_mutator(*this);

    restore_pmap(state);
  }

  inline void
  fast_decoder_impl::visit(nested_message_mref& mref, int)
  {
    pmap_state state;
    message_type* saved_active_message = active_message_;


    debug_ << "decoding dynamic templateRef ...\n";

    decode_pmap(state);

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
        using namespace coder;

        BOOST_THROW_EXCEPTION(fast_dynamic_error("D9") << template_id_info(template_id)
                                                       << referenced_by_info(active_message_->name()));
      }
      mref.set_target_instruction(active_message_->instruction(), false);
    }
    mref.accept_mutator(*this);

    restore_pmap(state);
    active_message_ = saved_active_message;

  }

  message_type*
  fast_decoder_impl::decode_segment(fast_istreambuf& sb)
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
        BOOST_THROW_EXCEPTION(fast_dynamic_error("D9") << coder::template_id_info(template_id));
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
    // message->ensure_valid();
    message->ref().accept_mutator(*this);
    return message;
  }

  fast_decoder::fast_decoder(allocator* alloc)
    : impl_(new fast_decoder_impl)
  {
    impl_->message_alloc_ = alloc;
  }

  fast_decoder::~fast_decoder()
  {
    delete impl_;
  }

  void
  fast_decoder::include(const templates_description** descriptions, std::size_t description_count)
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
  fast_decoder::decode(const char*& first, const char* last, bool force_reset)
  {
    assert(first < last);
    fast_istreambuf sb(first, last-first);
    impl_->force_reset_ = force_reset;
    message_cref result = impl_->decode_segment(sb)->cref();
    first = sb.gptr();
    return result;
  }

  void
  fast_decoder::debug_log(std::ostream* log)
  {
    impl_->debug_.set(log);
  }

  void
  fast_decoder::warning_log(std::ostream* os)
  {
    impl_->strm_.warning_log(os);
  }

}
