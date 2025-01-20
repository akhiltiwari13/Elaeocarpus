#include <iostream>
#include <events/callback.h>
#include <events/types.h>

static constexpr int a  = 5;

/**
 * @brief  function to test neogen
 *
 * @return  returns constexpr bool.
 */
constexpr bool func(){
  return 5 == a;
}
