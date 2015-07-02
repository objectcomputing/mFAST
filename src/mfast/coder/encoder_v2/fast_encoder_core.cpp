#include "fast_encoder_core.h"

namespace mfast {
namespace coder {

void fast_encoder_core::encode_segment(const message_cref cref,
                                       bool force_reset) {
  uint32_t template_id = cref.id();

  info_entry *info = repo_.find(template_id);

  if (info == nullptr) {
    BOOST_THROW_EXCEPTION(fast_dynamic_error("D9")
                          << template_id_info(template_id));
  }

  template_instruction *instruction = std::get<0>(*info);

  if (force_reset || instruction->has_reset_attribute())
    repo_.reset_dictionary();

  bool need_encode_template_id = (active_message_info_ != info);

  encoder_presence_map pmap;
  this->current_ = &pmap;
  pmap.init(&this->strm_, std::max<std::size_t>(instruction->segment_pmap_size(), 1));
  pmap.set_next_bit(need_encode_template_id);

  if (need_encode_template_id) {
    active_message_info_ = info;
    strm_.encode(template_id, false, false_type());
  }

  message_cref message(cref.field_storage(0), instruction);
  message_encode_function_t encode = std::get<1>(*info);
  (this->*encode)(message);

  pmap.commit();
}

} /* coder */
} /* mfast */
