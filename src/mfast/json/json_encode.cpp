// Copyright (c) 2016, Huang-Ming Huang,  Object Computing, Inc.
// All rights reserved.
//
// This file is part of mFAST.
// See the file license.txt for licensing information.
#include "json.h"
#include <boost/io/ios_state.hpp>
#include <cstdio>
#ifdef _MSC_VER // someday someone at microsoft will read the C++11 standard.
#define snprintf _snprintf
#else // _MSCVER
#define snprintf std::snprintf
#endif // _MSCVER

namespace mfast {
namespace json {
namespace encode_detail {

struct quoted_string {
  quoted_string(const char *str) : str_(str) {}

  const char *str_;
};

std::ostream &operator<<(std::ostream &os, quoted_string str) {
  os.put('"');
  const char *ptr = str.str_;
  char c;
  while ((c = *ptr++) != '\0') {
    // According to ECMA-404, the control charanter (U+0000 to U+001F) must be
    // escaped
    if ((c & 0xE0) == 0) {
      // if c is '\x08', '\x09','\0x0A','\0x0C' and '\0x0D', encoded as "\b",
      // "\t", "\n", "\f", "\r" repsectively;
      // otherwise encoded as "\uxxxx" where xxxx is 4 digit hexidecimal
      // characters.
      const char control_table[33] = "uuuuuuuubtnufruuuuuuuuuuuuuuuuuu";
      char buf[7] = "\\";
      buf[1] = control_table[static_cast<int>(c)];
      if (buf[1] == 'u') {
        snprintf(buf + 2, 5, "%04x", static_cast<int>(c));
      }
      os << buf;
      continue;
    }
    if (c == '\\' || c == '"')
      os.put('\\');
    os.put(c);
  }
  os.put('"');
  return os;
}

class json_visitor {
protected:
  std::ostream &strm_;
  char separator_[2];
  unsigned json_object_tag_mask_;

public:
  json_visitor(std::ostream &strm, unsigned json_object_tag_mask)
      : strm_(strm), json_object_tag_mask_(json_object_tag_mask) {
    separator_[0] = 0;
    separator_[1] = 0;
  }

  template <typename NumericTypeRef> void visit(const NumericTypeRef &ref) {
    strm_ << separator_ << ref.value();
  }

  void visit(const decimal_cref &ref) {
    const decimal_value_storage &storage =
        *reinterpret_cast<const decimal_value_storage *>(
            field_cref_core_access::storage_of(ref));
    strm_ << separator_ << storage;
  }

  void visit(const enum_cref &ref) {
    if (ref.is_boolean())
      strm_ << separator_ << ref.value_name();
    else
      strm_ << separator_ << ref.value();
  }

  template <typename Char> void visit(const mfast::string_cref<Char> &ref) {
    strm_ << separator_ << quoted_string(ref.c_str());
  }

  void visit(const mfast::byte_vector_cref &ref) {
    if (ref.instruction()->tag().to_uint64() & json_object_tag_mask_) {
      // if the json_object_tag_mask is on, that means the field contains
      // json encoded object already, just write it as it is without any
      // processing.

      strm_.rdbuf()->sputn(reinterpret_cast<const char *>(ref.data()),
                           ref.size());
    } else {
      // json doesn't have byte vector, treat it as hex string now
      strm_ << separator_ << "\"";
      boost::io::ios_flags_saver ifs(strm_);
      strm_ << std::hex << std::setfill('0') << std::setw(2);

      for (std::size_t i = 0; i < ref.size(); ++i) {
        // if the size is 16, we treat it as a UUID
        if (ref.size() == 16 && (i == 4 || i == 6 || i == 8 || i == 10))
          strm_ << '-';
        strm_ << (0xFF & (int)ref[i]);
      }
      strm_ << "\"" << std::setfill(' ');
    }
  }

  template <typename IntType>
  void visit(const mfast::int_vector_cref<IntType> &ref) {
    strm_ << separator_ << "[";
    separator_[0] = '\0';

    for (auto &&field : ref) {
      strm_ << separator_ << field;
      separator_[0] = ',';
    }
    strm_ << "]";
  }

  virtual void visit(const mfast::aggregate_cref &ref, int) {
    if (ref.num_fields() == 1) {
      field_cref f0 = ref[0];
      if (f0.instruction()->field_type() == mfast::field_type_templateref) {
        if (f0.present())
          this->visit(mfast::nested_message_cref(f0), 0);
        return;
      }
    }

    strm_ << separator_ << "{";
    separator_[0] = '\0';

    for (auto &&field : ref) {
      if (field.present()) {
        strm_ << separator_ << quoted_string(field.name()) << ":";
        separator_[0] = '\0';
        apply_accessor(*this, field);
        ;
        separator_[0] = ',';
      }
    }

    strm_ << "}";
  }

  void visit(const mfast::sequence_cref &ref, int) {
    strm_ << separator_ << "[";
    if (ref.size()) {
      separator_[0] = '\0';
      for (auto &&elem : ref)
        this->visit(elem, 0);
    }
    strm_ << "]";
  }

  void visit(const mfast::sequence_element_cref &ref, int) {
    if (ref.element_unnamed()) {
      apply_accessor(*this, ref[0]);
      ;
    } else {
      this->visit(mfast::aggregate_cref(ref), 0);
    }
    separator_[0] = ',';
  }

  void visit(const mfast::nested_message_cref &ref, int) {
    this->visit(mfast::aggregate_cref(ref), 0);
  }
};

class json_visitor_with_ignore_tag : public json_visitor {
private:
  unsigned ignore_tag_mask_;

public:
  using json_visitor::visit;

  json_visitor_with_ignore_tag(std::ostream &strm,
                               unsigned json_object_tag_mask,
                               unsigned ignore_tag_mask)
      : json_visitor(strm, json_object_tag_mask),
        ignore_tag_mask_(ignore_tag_mask) {}

  virtual void visit(const mfast::aggregate_cref &ref, int) override {
    if (ref.num_fields() == 1) {
      field_cref f0 = ref[0];
      if (f0.instruction()->field_type() == mfast::field_type_templateref) {
        if (f0.present())
          this->visit(mfast::nested_message_cref(f0), 0);
        return;
      }
    }

    strm_ << separator_ << "{";
    separator_[0] = '\0';

    for (auto &&field : ref) {
      if (field.present() &&
          0 == (field.instruction()->tag().to_uint64() & ignore_tag_mask_)) {
        strm_ << separator_ << quoted_string(field.name()) << ":";
        separator_[0] = '\0';
        apply_accessor(*this, field);
        ;
        separator_[0] = ',';
      }
    }

    strm_ << "}";
  }
};

} // namspace encode_detail

bool encode(std::ostream &os, const mfast::aggregate_cref &msg,
            unsigned json_object_tag_mask) {
  encode_detail::json_visitor visitor(os, json_object_tag_mask);
  visitor.visit(msg, 0);
  return os.good();
}

bool encode(std::ostream &os, const mfast::sequence_cref &seq,
            unsigned json_object_tag_mask) {
  encode_detail::json_visitor visitor(os, json_object_tag_mask);
  visitor.visit(seq, 0);
  return os.good();
}

bool encode(std::ostream &os, const ::mfast::aggregate_cref &msg,
            unsigned json_object_tag_mask, unsigned ignore_tag_mask) {
  encode_detail::json_visitor_with_ignore_tag visitor(os, json_object_tag_mask,
                                                      ignore_tag_mask);
  visitor.visit(msg, 0);
  return os.good();
}

} // namespace json
} // namespace mfast
