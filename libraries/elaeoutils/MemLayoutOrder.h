namespace elaeocarpus::elaeoutils{

/* funtion template to get relative memory ordering of a class' data-members. */
template <typename class_type, typename data_type1, typename data_type2>
//checking pointer location
char *access_order(data_type1 *class_type::mem1, data_type2 *class_type::mem2) {
  assert(mem1 != mem2);
  return mem1 < mem2 ? "member 1 occurs first" : "member 2 occurs first";
}

} // namespace elaeoutils
