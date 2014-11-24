#include "fast_encoder_core.h"

namespace mfast
{
  namespace coder
  {

    void
    fast_encoder_core::encode_segment(const message_cref cref, bool force_reset)
    {
      uint32_t template_id = cref.id();

      message_info* info;

      message_info_map_t::iterator itr = message_infos_.find(template_id);

      if (itr != message_infos_.end()) {
        info = &itr->second;
      }
      else {
        BOOST_THROW_EXCEPTION(fast_dynamic_error("D9") << template_id_info(template_id));
      }

      template_instruction* instruction = info->get<0>();

      if ( force_reset ||  instruction->has_reset_attribute())
        resetter_.reset();

      bool need_encode_template_id = (active_message_info_ != info);

      encoder_presence_map pmap;
      this->current_ = &pmap;
      pmap.init(&this->strm_, instruction->segment_pmap_size());
      pmap.set_next_bit(need_encode_template_id);

      if (need_encode_template_id)
      {
        active_message_info_ = info;
        strm_.encode(template_id, false, false_type());
      }

      message_cref message(cref.field_storage(0), instruction);
      message_encode_function_t encode = info->get<1>();
      (this->*encode)(message);

      pmap.commit();
    }

  } /* coder */
} /* mfast */
