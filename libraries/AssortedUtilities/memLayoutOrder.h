namespace com {
namespace elaeocarpus {
namespace assortedUtils {

/* @todo- does not work.. need to check! */
/* funtion template to get relative memory ordering of a class' data-members. */
template <class class_type, class data_type1, class data_type2>
char *access_order(data_type1 class_type::*mem1, data_type2 class_type::*mem2) {
  assert(mem1 != mem2);
  return mem1 < mem2 ? "member 1 occurs first" : "member 2 occurs first";
}
} // namespace assortedUtils
} // namespace elaeocarpus
} // namespace com
