#ifndef FAST_ENCODER_CORE_H_7EA7EA84
#define FAST_ENCODER_CORE_H_7EA7EA84

#include "../mfast_coder_export.h"
#include <boost/container/map.hpp>
#include "mfast/sequence_ref.h"
#include "mfast/nested_message_ref.h"
#include "mfast/malloc_allocator.h"
#include "../common/dictionary_builder.h"
#include "../common/exceptions.h"
#include "../encoder/fast_ostream.h"
#include "../encoder/resizable_fast_ostreambuf.h"
#include "../encoder/encoder_presence_map.h"
#include "encoder_function.h"
#include <boost/mpl/for_each.hpp>
#include <boost/mpl/placeholders.hpp>
#include <boost/tuple/tuple.hpp>

namespace mfast
{
  namespace coder
  {


    class fast_encoder_visitor;

    template <typename T>
    class encoder_pmap_saver;

    struct MFAST_CODER_EXPORT fast_encoder_core
    {
      typedef void (fast_encoder_core::*message_encode_function_t) (const message_cref&);

      struct message_info
      {
        message_info(const boost::tuple<template_instruction*, message_encode_function_t>& inputs)
          : instruction_(inputs.get<0>())
          , encode_fun_(inputs.get<1>())
        {
        }

        template_instruction* instruction_;
        message_encode_function_t encode_fun_;
      };

      typedef boost::container::map<uint32_t, message_info> message_info_map_t;

      fast_ostream strm_;
      dictionary_resetter resetter_;
      dictionary_value_destroyer value_destroyer_;

      arena_allocator template_alloc_;   // template_alloc MUST be constructed before template_messages,
      message_info_map_t message_infos_;   // Do not change the order of the two

      message_info* active_message_info_;

      encoder_presence_map* current_;

      encoder_presence_map& current_pmap()
      {
        return *current_;
      }

      fast_ostream& strm()
      {
        return strm_;
      }

      fast_encoder_core(allocator* alloc)
        : strm_(alloc)
        , value_destroyer_(alloc)
        , active_message_info_(0)
        , current_(0)
      {
      }

      inline void
      encode_nested_message(const nested_message_cref& cref)
      {
        message_info* saved_message_info = active_message_info_;
        encoder_presence_map* prev_pmap = this->current_;
        encode_segment(cref.target(), false);
        this->current_ = prev_pmap;
        active_message_info_ = saved_message_info;
      }

      void encode_segment(const message_cref cref, bool force_reset);

      template <typename Message>
      void encode_message(const message_cref& cref);

      template <typename Message>
      void register_message();

      template <typename Templates>
      void build_dictionary(template_id_map_t& templates_map);

      template <typename Descriptions>
      void init();

      std::size_t
      encode_i(const message_cref& message,
               char*               buffer,
               std::size_t         buffer_size,
               bool                force_reset);

      void
      encode_i(const message_cref& message,
               std::vector<char>&  buffer,
               bool                force_reset);

      inline void allow_overlong_pmap_i(bool v);
    };

    template <typename T>
    class encoder_pmap_saver
    {
      encoder_presence_map pmap_;
      encoder_presence_map* prev_pmap_;
      fast_encoder_core* core_;

    public:
      encoder_pmap_saver(fast_encoder_core* core)
        : prev_pmap_(core->current_)
        , core_(core)
      {
        core_->current_ = &pmap_;
        this->pmap_.init(&core_->strm_, T::value);
      }

      ~encoder_pmap_saver()
      {
        this->pmap_.commit();
        core_->current_ = this->prev_pmap_;
      }

    };

    template <>
    class encoder_pmap_saver<pmap_segment_size_zero>
    {
    public:
      encoder_pmap_saver(fast_encoder_core*)
      {
      }

      ~encoder_pmap_saver()
      {
      }

    };

    class fast_encoder_visitor
      : private encoder_function
    {
      fast_encoder_core* core_;

    public:
      fast_encoder_visitor(fast_encoder_core* core)
        : core_(core)
      {
      }

      template <typename T>
      void visit(const T& ext_ref)
      {
        typedef typename T::type_category type_category;
        this->visit(ext_ref, type_category());
      }

      template <typename T, typename TypeCategory>
      void visit(const T &ext_ref, TypeCategory);

      template <typename T>
      void visit(const T &ext_ref, split_decimal_type_tag);

      template <typename T>
      void visit(const T &ext_ref, int_vector_type_tag);

      template <typename T>
      void visit(const T &ext_ref, group_type_tag);

      template <typename T>
      void visit(const T &ext_ref, sequence_type_tag);

      void visit(const nested_message_cref& cref)
      {
        core_->encode_nested_message(cref);
      }

    };

    template <typename T, typename TypeCategory>
    inline void
    fast_encoder_visitor::visit(const T& ext_ref, TypeCategory)
    {
      this->encode(ext_ref,
                   core_->strm(),
                   core_->current_pmap(),
                   typename T::operator_category(),
                   TypeCategory());

    }

    template <typename T>
    inline void
    fast_encoder_visitor::visit(const T& ext_ref, split_decimal_type_tag)
    {

      typename T::exponent_type exponent_ref = ext_ref.get_exponent();
      this->encode(exponent_ref, core_->strm(), core_->current_pmap());
      if (exponent_ref.present())
      {
        this->encode(ext_ref.get_mantissa(), core_->strm(), core_->current_pmap());
      }
    }

    template <typename T>
    inline void fast_encoder_visitor::visit(const T& ext_ref, int_vector_type_tag)
    {
      typename T::cref_type cref = ext_ref.get();

      core_->strm_.encode(static_cast<uint32_t>(cref.size()), !cref.present(), cref.optional());
      if (cref.present()) {
        for (std::size_t i = 0; i < cref.size(); ++i) {
          core_->strm_.encode(cref[i], false, false_type());
        }
      }
    }

    template <typename T>
    inline void
    fast_encoder_visitor::visit(const T& ext_ref, group_type_tag)
    {
      // If a group field is optional, it will occupy a single bit in the presence map.
      // The contents of the group may appear in the stream iff the bit is set.
      if (ext_ref.optional())
      {
        core_->current_pmap().set_next_bit(ext_ref.present());

        if (!ext_ref.present())
          return;
      }

      encoder_pmap_saver<typename T::pmap_segment_size_type> saver(core_);
      ext_ref.get().accept(*this);
    }

    template <typename T>
    inline void
    fast_encoder_visitor::visit(const T& ext_ref, sequence_type_tag)
    {
      value_storage storage;

      typename T::length_type length = ext_ref.get_length(storage);
      this->visit(length);
      std::size_t sz = length.get().value();
      for (std::size_t i = 0; i < sz; ++i)
      {
        this->visit(ext_ref[i]);
      }
    }

    template <typename Message>
    void fast_encoder_core::encode_message(const message_cref& cref)
    {
      fast_encoder_visitor visitor(this);
      typename Message::cref_type ref(cref);
      ref.accept(visitor);
    }

    struct construct_encoder_message_info
    {
      fast_encoder_core* core_;
      template_id_map_t& templates_map_;

      construct_encoder_message_info(fast_encoder_core* core,
                                     template_id_map_t& templates_map)
        : core_(core)
        , templates_map_(templates_map)
      {
      }

      template< typename Message>
      typename boost::disable_if< boost::is_base_of<mfast::templates_description, Message> >::type
      operator()(Message*)
      {
        // fast_encoder_core::message_decode_function_t fun = &fast_encoder_core::decode_message<Message>;

        uint32_t id = Message::the_id;
        // assert(dynamic_cast<const typename Message::instruction_type*>(templates_map_[Message::the_id]) );
        core_->message_infos_.emplace(id, boost::make_tuple(templates_map_[Message::the_id],
                                                            &fast_encoder_core::encode_message<Message>));
      }

      template< typename Description>
      typename boost::enable_if< boost::is_base_of<mfast::templates_description, Description> >::type
      operator()(Description*)
      {
        using boost::mpl::placeholders::_1;
        boost::mpl::for_each<typename Description::types, boost::add_pointer<_1> >(*this);
      }

    };

    template <typename Descriptions>
    void fast_encoder_core::init()
    {
      template_id_map_t templates_map;
      using boost::mpl::placeholders::_1;

      boost::mpl::for_each<Descriptions, boost::add_pointer<_1> >(
        dictionary_builder(this->resetter_, templates_map, &this->template_alloc_)
        );

      boost::mpl::for_each<Descriptions, boost::add_pointer<_1> >( construct_encoder_message_info(this, templates_map) );

      if (this->message_infos_.size() == 1)
      {
        active_message_info_ = &message_infos_.begin()->second;
      }
    }

    inline std::size_t
    fast_encoder_core::encode_i(const message_cref& message,
                                char*               buffer,
                                std::size_t         buffer_size,
                                bool                force_reset)
    {
      assert(buffer_size > 0);

      fast_ostreambuf sb(buffer, buffer_size);
      this->strm_.rdbuf(&sb);
      this->encode_segment(message, force_reset);
      return sb.length();
    }

    inline void
    fast_encoder_core::encode_i(const message_cref& message,
                                std::vector<char>&  buffer,
                                bool                force_reset)
    {
      resizable_fast_ostreambuf sb(buffer);
      this->strm_.rdbuf(&sb);
      this->encode_segment(message, force_reset);
      buffer.resize(sb.length());
    }

    inline void
    fast_encoder_core::allow_overlong_pmap_i(bool v)
    {
      this->strm_.allow_overlong_pmap(v);
    }

  } /* coder */
} /* mfast */


#endif /* end of include guard: FAST_ENCODER_CORE_H_7EA7EA84 */
