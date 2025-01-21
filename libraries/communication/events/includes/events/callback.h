/**
 * @file event-callback.h
 * @brief  defines base event callback class.
 */

#ifndef ELAEO_COMM_EVENTS_CALLBACK_H
#define ELAEO_COMM_EVENTS_CALLBACK_H
#include <concepts>
#include <type_traits>

namespace elaeo::comm::events{

/**
 * @brief : The concept is defined in the context of a template parameter Event. Constraint Expression: It uses std::is_trivially_copyable_v and std::is_move_assignable_v to ensure that any type used as Event is trivially copyable and move assignable.
 * @tparam  Event
 */
template <typename Event>
concept EventType = std::is_trivially_copyable<Event>() && std::is_move_assignable<Event>();

// typename <EventType Event>
// class EventCallback{
// public:
//   virtual void handleEvents(const Event& event) noexcept = 0;
//   virtual ~EventCallback()  noexcept = default;
// }

}
#endif //ELAEO_COMM_EVENTS_CALLBACK_H
