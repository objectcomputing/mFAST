#include "view_iterator.h"
#include "sequence_ref.h"

namespace mfast
{
  bool view_iterator::access_field()
  {
    if (view_info_->prefix_diff() < non_leaves_.size())
      non_leaves_.resize(view_info_->prefix_diff());

    while (non_leaves_.back().absent()) {
      // if the field is nesting inside a sequence, an absent
      // object could be inserted into the stack in order to
      // keep nesting level correct.
      non_leaves_.pop_back();
    }

    unsigned nest_level = non_leaves_.size()-1;
    int index = view_info_->nest_indices[nest_level];
    bool finished = false;
    do
    {
      leaf_.refers_to( non_leaves_.back()[index] );

      index = view_info_->nest_indices[++nest_level];
      finished = (leaf_.absent() || index == -1 );
      if (!finished)
      {

        if (leaf_.field_type() == mfast::field_type_sequence)
        {
          sequence_cref seq(leaf_);
          if ((int)seq.size() > index){
            // I cannot store an sequence_cref into the stack because of the
            // incompatibility between sequence_cref and aggregate_cref.
            // Therefore, I just store an empty object to keep the
            // correct nesting level
            non_leaves_.push_back(aggregate_cref());
            non_leaves_.push_back( seq[index] );

            index = view_info_->nest_indices[++nest_level];
            if (index == -1) {
              leaf_.refers_to(non_leaves_.back()[0]);
            }
          }
          else {
            finished = true;
          }
        }
        else {
          non_leaves_.push_back( leaf_ );
        }
      }
    } while (!finished);
    return leaf_.present();
  }

  void view_iterator::increment()
  {
    ++view_info_;
    if (view_info_->prop == 0)
      return;

    while (!access_field() && view_info_->cont())
      ++view_info_;

    while (view_info_->cont())
      ++view_info_;
  }

} /* mfast */