#include "converter.h"
#include "converter_core.h"
#include "aggregate_inserter.h"
namespace mfast {
namespace SQLite3 {

converter::converter(const char *dbfile, const field_masks &masks)
    : core_(new converter_core(dbfile, masks)) {}

converter::~converter() { delete core_; }

void converter::add_instruction(const mfast::template_instruction *inst,
                                bool to_create_tables) {
  core_->add(inst, to_create_tables);
}

void converter::insert(const aggregate_cref &ref) {
  aggregate_inserter inserter(*core_);
  inserter.insert(ref, field_cref());
}

} /* SQLite3 */

} /* mfast */
