#pragma once

#include <mfast.h>
#include <iostream>
#include <boost/io/ios_state.hpp>

using namespace mfast;

class indenter
{
  std::size_t level_;

  public:
    indenter()
      : level_(0)
    {
    }

    indenter& operator ++()
    {
      ++level_;
      return *this;
    }

    indenter& operator --()
    {
      --level_;
      return *this;
    }

    std::size_t blanks() const
    {
      return level_*2;
    }

};

std::ostream&
operator << (std::ostream& os, const indenter& indent)
{
  os << std::setw(indent.blanks()) << ' ';
  return os;
}

class message_printer
{
  std::ostream& os_;
  indenter indent_;

  public:

    message_printer(std::ostream& os)
      : os_(os)
    {
    }

    template <typename IntType>
    void visit(const int_cref<IntType>& ref)
    {
      // matches int32_cref, uint32_cref, int64_cref, uint64_cref
      os_ << ref.value();
    }

    void visit(const enum_cref& ref)
    {
      os_ << ref.value_name();
    }

    void visit(const decimal_cref& ref)
    {
      os_ << ref.mantissa() << "*10^" << (int)ref.exponent();
    }

    template <typename CharType>
    void visit(const string_cref<CharType>& ref)
    {
      // matches ascii_string_cref and unicode_string_cref
      os_ << ref.c_str();
    }

    void visit(const byte_vector_cref& ref)
    {
      boost::io::ios_flags_saver  ifs( os_ );
      os_ << std::hex << std::setfill('0');

      for (auto elem : ref){
        os_ << std::setw(2) <<  static_cast<unsigned>(elem);
      }
      os_ << std::setfill(' ');

    }

    template <typename IntType>
    void visit(const int_vector_cref<IntType>& ref)
    {
      // matches int32_vector_cref, uint32_vector_cref, int64_vector_cref, uint64_vector_cref

      char sep[2]= { '\x0', '\x0'};
      os_ << "{";
      for (auto elem : ref){
        os_ << sep << elem;
        sep[0] = ',';
      }
      os_ << "}";
    }

    void visit(const aggregate_cref& ref, int)
    {
      ++indent_;
      for (auto&& field : ref) {
        if (field.present()) {
          os_ << indent_ << field.name() << ": ";
          apply_accessor(*this, field);
          os_ << std::endl;
        }
      }
      --indent_;
    }

    void visit(const sequence_cref&  ref, int)
    {
      size_t index = 0;
      ++indent_;
      for (auto element : ref) {
        os_ << indent_ << "[" << index++ << "]" << ":\n";
        // we cannot use apply_accessor(*this, field) here, because that would only visit the
        // sub-fields of the elment instead of calling visit(const aggregate_cref& ref, int)
        this->visit(element, 0);
        os_ << "\n";
      }
      --indent_;
    }
};

