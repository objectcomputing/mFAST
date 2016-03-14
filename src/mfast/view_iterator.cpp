// Copyright (c) 2016, Huang-Ming Huang,  Object Computing, Inc.
// All rights reserved.
//
// This file is part of mFAST.
// See the file license.txt for licensing information.
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4244)
#endif

#include "view_iterator.h"
#include "sequence_ref.h"

namespace mfast {
bool view_iterator_helper::access_field(view_iterator *itr) {
  if (itr->view_info_->prefix_diff() < itr->non_leaves_.size())
    itr->non_leaves_.resize(itr->view_info_->prefix_diff());

  while (itr->non_leaves_.back() == aggregate_cref::iterator()) {
    // if the field is nesting inside a sequence, a null
    // object could be inserted into the stack in order to
    // keep nesting level correct.
    itr->non_leaves_.pop_back();
  }

  std::size_t nest_level = itr->non_leaves_.size() - 1;
  field_view_info::nest_index_t index =
      itr->view_info_->nest_indices[nest_level];
  bool finished = false;
  do {
    itr->leaf_.refers_to(*(itr->non_leaves_.back() + index));

    index = itr->view_info_->nest_indices[++nest_level];
    finished = (itr->leaf_.absent() || index == -1);
    if (!finished) {

      if (itr->leaf_.field_type() == mfast::field_type_sequence) {
        sequence_cref seq(itr->leaf_);
        if ((int)seq.size() > index) {
          // I cannot store an sequence_cref into the stack because of the
          // incompatibility between sequence_cref and aggregate_cref.
          // Therefore, I just store an empty object to keep the
          // correct nesting level
          itr->non_leaves_.push_back(aggregate_cref::iterator());
          itr->non_leaves_.push_back(seq[index].begin());

          index = itr->view_info_->nest_indices[++nest_level];
          if (index == -1) {
            itr->leaf_.refers_to(*itr->non_leaves_.back());
          }
        } else {
          finished = true;
        }
      } else {
        itr->non_leaves_.push_back(aggregate_cref(itr->leaf_).begin());
      }
    }
  } while (!finished);
  return itr->leaf_.present();
}

void view_iterator_helper::increment(view_iterator *itr) {
  ++itr->view_info_;
  if (itr->view_info_->prop == 0)
    return;

  while (!access_field(itr) && itr->view_info_->cont())
    ++itr->view_info_;

  while (itr->view_info_->cont())
    ++itr->view_info_;
}

} /* mfast */
#ifdef _MSC_VER
#pragma warning(pop)
#endif
