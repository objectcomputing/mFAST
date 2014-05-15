#ifndef TYPE_CATEGORY_H_6F1EC943
#define TYPE_CATEGORY_H_6F1EC943

namespace mfast
{
  struct number_type_tag {};
  struct integer_type_tag : number_type_tag {};
  struct vector_type_tag {};
  struct string_type_tag : vector_type_tag {};
  struct int_vector_type_tag : vector_type_tag {};
  struct group_type_tag {};
  struct sequence_type_tag {};
  struct decimal_type_tag : number_type_tag {};
  struct split_decimal_type_tag : decimal_type_tag {};
} /* mfast */


#endif /* end of include guard: TYPE_CATEGORY_H_6F1EC943 */
