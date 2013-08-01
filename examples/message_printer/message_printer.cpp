
#include "MDRefreshSample.h"
#include "mfast/output.h"
#include <iostream>
#include <iomanip>
#include <map>
#include <vector>
using std::ostream;
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
  : public field_accessor_base
{
  std::ostream& os_;
  indenter indent_;
  


  public:
    
    typedef index_mixin<sequence_element_cref> sequence_element_ref_type;

    message_printer(std::ostream& os)
      : os_(os)
    {
    }

    template <typename PrimitiveTypeRef>
    void visit(const PrimitiveTypeRef& ref)
    {
      os_ << indent_ << ref.name() << ": " << ref
          << "\n";
    }

    template <typename CompositeTypeRef>
    bool pre_visit(const CompositeTypeRef& ref)
    {
      os_ << indent_ << ref.name() << ":\n";
      ++indent_;
      return true;
    }

    template <typename CompositeTypeRef>
    void post_visit(const CompositeTypeRef& /* ref */)
    {
      --indent_;
    }

    bool pre_visit(const sequence_element_ref_type&  ref)
    {
      os_ << indent_ <<  "[" << ref.index << "]:\n";
      ++indent_;
      return true;
    }

};

int main()
{
  using namespace MDRefreshSample;

  MarketDataIncrementalRefresh message;
  MarketDataIncrementalRefresh_mref ref = message.ref();
  ref.set_MDEntries().resize(1);
  MarketDataIncrementalRefresh_mref::MDEntries_element_mref entry(ref.set_MDEntries()[0]);
  entry.set_MDUpdateAction().as(1);
  const char* str = "abcd";
  entry.set_MDEntryType().as(str);

  entry.set_Symbol().as("AAPL");
  entry.set_SecurityType().as("Stock");
  entry.set_MDEntryPx().as(1, 2);
  entry.set_MDEntrySize().as(3,4);
  entry.set_NumberOfOrders().as(100);

  message_printer printer(std::cout);
  ref.accept_accessor(printer);
  return 0;
}
