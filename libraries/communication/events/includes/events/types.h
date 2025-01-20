/**
 * @file
 * @brief 
 */

#ifndef ELAEO_COMM_EVENTS_TYPES_H
#define ELAEO_COMM_EVENTS_TYPES_H

#include <cstdint>

namespace elaeo::comm::events{

  using subscription_t = uint32_t;
  using priority_t = uint16_t;

  // Predefined priorities for convenience
  constexpr priority_t PRIORITY_LOWEST = 0;
  constexpr priority_t PRIORITY_LOW = 10;
  constexpr priority_t PRIORITY_NORMAL = 100;
  constexpr priority_t PRIORITY_HIGH = 1000;
  constexpr priority_t PRIORITY_HIGHEST = 10000;
}

#endif // ELAEO_COMM_EVENTS_TYPES_H
