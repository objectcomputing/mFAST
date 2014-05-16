#ifndef FAST_DECODER_CORE_H_51998563
#define FAST_DECODER_CORE_H_51998563

#include "../mfast_coder_export.h"

#include <boost/container/map.hpp>
#include "mfast/sequence_ref.h"
#include "mfast/nested_message_ref.h"
#include "mfast/malloc_allocator.h"
#include "mfast/output.h"
#include "mfast/composite_type.h"
#include "../common/exceptions.h"
#include "../common/debug_stream.h"
#include "../common/dictionary_builder.h"
#include "../common/codec_helper.h"
#include "../decoder/decoder_presence_map.h"
#include "decoder_function.h"
#include <boost/mpl/for_each.hpp>
#include <boost/tuple/tuple.hpp>
namespace mfast
{
  namespace coder
  {
    class fast_decoder_visitor;

    template <typename HasPMap>
    class decoder_pmap_saver;



    struct MFAST_CODER_EXPORT fast_decoder_core
    {
      typedef void (fast_decoder_core::*message_decode_function_t) (const message_mref&);

      struct message_info
      {
        message_info(const boost::tuple<allocator*, const template_instruction*, message_decode_function_t>& inputs)
          : message_(inputs.get<0>(), inputs.get<1>())
          , decode_fun_(inputs.get<2>())
        {
        }

        message_type message_;
        message_decode_function_t decode_fun_;
      };

      typedef boost::container::map<uint32_t, message_info> message_info_map_t;

      fast_istream strm_;
      dictionary_resetter resetter_;

      arena_allocator template_alloc_; // template_alloc MUST be constructed before template_messages,
      message_info_map_t message_infos_; // Do not change the order of the two

      allocator* message_alloc_;
      message_info* active_message_info_;
      bool force_reset_;
      decoder_presence_map* current_;

      decoder_presence_map&

      current_pmap()
      {
        return *current_;
      }

      fast_istream& strm()
      {
        return strm_;
      }

      fast_decoder_core(allocator* alloc)
        : strm_(0)
        , message_alloc_(alloc)
        , active_message_info_(0)
        , force_reset_(false)
        , current_(0)
      {
      }

      void decode_nested_message(const nested_message_mref& mref);
      message_type* decode_segment(fast_istreambuf& sb);
      message_cref decode_stream(const char*& first, const char* last, bool force_reset);


      template <typename Message>
      void decode_message(const message_mref& mref);

      template <typename Message>
      void register_message();


      template <typename Templates>
      void build_dictionary(template_id_map_t& templates_map);

      template <typename Templates>
      void construct_messages(template_id_map_t& templates_map);

      template <typename Descriptions>
      void init();
    };

    template <typename T>
    class decoder_pmap_saver
    {
      decoder_presence_map pmap_;
      decoder_presence_map* prev_pmap_;
      fast_decoder_core* core_;

    public:
      decoder_pmap_saver(fast_decoder_core* core)
        : prev_pmap_(core->current_)
        , core_(core)
      {
        core_->current_ = &pmap_;
        core_->strm().decode(pmap_);
      }

      ~decoder_pmap_saver()
      {
        core_->current_ = this->prev_pmap_;
      }

    };

    template <>
    class decoder_pmap_saver<pmap_segment_size_zero>
    {
    public:
      decoder_pmap_saver(fast_decoder_core*)
      {
      }

      ~decoder_pmap_saver()
      {
      }

    };

    class fast_decoder_visitor
      : private decoder_function
    {
      fast_decoder_core* core_;

    public:
      fast_decoder_visitor(fast_decoder_core* core)
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
      void visit(const T& ext_ref, TypeCategory);

      template <typename T>
      void visit(const T& ext_ref, split_decimal_type_tag);

      template <typename T>
      void visit(const T& ext_ref, int_vector_type_tag);

      template <typename T>
      void visit(const T& ext_ref, group_type_tag);

      template <typename T>
      void visit(const T& ext_ref, sequence_type_tag);

      void visit(const nested_message_mref& mref)
      {
        core_->decode_nested_message(mref);
      }

    };



    template <typename T, typename TypeCategory>
    inline void
    fast_decoder_visitor::visit(const T& ext_ref, TypeCategory)
    {
      this->decode(ext_ref,
                   core_->strm(),
                   core_->current_pmap(),
                   typename T::operator_category(),
                   TypeCategory());

    }

    template <typename T>
    inline void
    fast_decoder_visitor::visit(const T& ext_ref, split_decimal_type_tag)
    {

      typename T::exponent_type exponent_ref = ext_ref.set_exponent();
      this->decode(exponent_ref, core_->strm(), core_->current_pmap());
      if (exponent_ref.present())
      {
        this->decode(ext_ref.set_mantissa(), core_->strm(), core_->current_pmap());
      }
    }



    template <typename T>
    inline void fast_decoder_visitor::visit(const T& ext_ref, int_vector_type_tag)
    {
      typename T::mref_type mref = ext_ref.set();

      uint32_t length=0;
      if (!core_->strm().decode(length, ext_ref.optional())) {
        ext_ref.omit();
        return;
      }

      mref.resize(length);
      for (uint32_t i = 0; i < length; ++i) {
        core_->strm().decode(mref[i], false_type());
      }
    }

    template <typename T>
    inline void
    fast_decoder_visitor::visit(const T& ext_ref, group_type_tag)
    {
      // If a group field is optional, it will occupy a single bit in the presence map.
      // The contents of the group may appear in the stream iff the bit is set.
      if (ext_ref.optional())
      {
        if (!core_->current_pmap().is_next_bit_set()) {
          ext_ref.omit();
          return;
        }
      }

      decoder_pmap_saver<typename T::pmap_segment_size_type> saver(core_);
      ext_ref.set().accept(*this);
    }


    template <typename T>
    inline void
    fast_decoder_visitor::visit(const T& ext_ref, sequence_type_tag)
    {
      value_storage storage;

      typename T::length_type length = ext_ref.set_length(storage);
      this->visit(length);

      if (length.present()) {
        std::size_t len = length.get().value();
        ext_ref.set().resize(len);

        for (std::size_t i = 0; i < len; ++i)
        {
          this->visit(ext_ref[i]);
        }
      }
      else {
        ext_ref.omit();
      }
    }

    template <typename Message>
    void fast_decoder_core::decode_message(const message_mref& mref)
    {
      fast_decoder_visitor visitor(this);
      typename Message::mref_type ref(mref);
      ref.accept(visitor);
    }

    struct construct_decoder_message_info
    {
      fast_decoder_core* core_;
      template_id_map_t& templates_map_;

      construct_decoder_message_info(fast_decoder_core* core,
                           template_id_map_t& templates_map)
        : core_(core)
        , templates_map_(templates_map)
      {
      }

      template< typename Message>
      typename boost::disable_if< boost::is_base_of<mfast::templates_description, Message> >::type
      operator()(Message*)
      {
        // fast_decoder_core::message_decode_function_t fun = &fast_decoder_core::decode_message<Message>;

        uint32_t id = Message::the_id;
        // assert(dynamic_cast<const typename Message::instruction_type*>(templates_map_[Message::the_id]) );
        core_->message_infos_.emplace(id, boost::make_tuple(core_->message_alloc_,
                                                            templates_map_[Message::the_id],
                                                            &fast_decoder_core::decode_message<Message>));
      }

      template< typename Description>
      typename boost::enable_if< boost::is_base_of<mfast::templates_description, Description> >::type
      operator()(Description*)
      {
        boost::mpl::for_each<typename Description::types>(*this);
      }

    };

    template <typename Descriptions>
    void fast_decoder_core::init()
    {
      template_id_map_t templates_map;

      boost::mpl::for_each<Descriptions>(
        dictionary_builder(this->resetter_, templates_map, &this->template_alloc_)
        );

      boost::mpl::for_each<Descriptions>( construct_decoder_message_info(this, templates_map) );

      if (this->message_infos_.size() == 1)
      {
        active_message_info_ = &message_infos_.begin()->second;
      }
    }

  } /* coder */

} /* mfast */



#endif /* end of include guard: FAST_DECODER_CORE_H_51998563 */
