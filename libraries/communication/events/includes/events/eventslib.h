#include <concepts>
#include <functional>
#include <memory>
#include <vector>
#include <coroutine>
#include <atomic>
#include <queue>
#include <poll.h>
#include <boost/asio.hpp>

namespace elaeo::comm::events {

// Concept for event sources
template <typename T>
concept EventSource = requires(T t) {
    { t.poll() } -> std::same_as<bool>;
};

// Concept for event callbacks
template <typename T, typename Event>
concept EventCallback = requires(T t, const Event& event) {
    { t.handleEvent(event) } -> std::same_as<void>;
};

// PollingEventSource using concepts
class PollingEventSource {
public:
    virtual bool poll() = 0;
    virtual ~PollingEventSource() = default;
};

// FDEventSource using concepts
class FDEventSource {
public:
    virtual int getFD() = 0;
    virtual bool read() = 0;
    virtual bool write() = 0;
    virtual ~FDEventSource() = default;
};

// EventCallback using concepts
template <typename Event>
class EventCallbackBase {
public:
    virtual void handleEvent(const Event& event) = 0;
    virtual ~EventCallbackBase() = default;
};

// LambdaCallback using concepts and modern C++
template <typename Event, std::invocable<const Event&> Callback>
class LambdaCallback : public EventCallbackBase<Event> {
public:
    explicit LambdaCallback(Callback&& callback) : callback_(std::move(callback)) {}
    void handleEvent(const Event& event) override { callback_(event); }

private:
    Callback callback_;
};

// EventPublisher using modern C++
template <typename Event>
class EventPublisher {
public:
    using CallbackPtr = std::shared_ptr<EventCallbackBase<Event>>;

    subscription_t registerCallback(CallbackPtr callback, priority_t priority = 0) {
        callbacks_.emplace_back(std::make_pair(priority, std::move(callback)));
        std::sort(callbacks_.begin(), callbacks_.end(), [](const auto& a, const auto& b) {
            return a.first > b.first;
        });
        return next_subscription_id_++;
    }

    void publishEvent(const Event& event) {
        for (const auto& [priority, callback] : callbacks_) {
            callback->handleEvent(event);
        }
    }

private:
    std::vector<std::pair<priority_t, CallbackPtr>> callbacks_;
    subscription_t next_subscription_id_{1};
};

// BaseEventLoop using coroutines and modern C++
template <EventSource FDEventSourcePoller>
class BaseEventLoop {
public:
    BaseEventLoop() = default;
    ~BaseEventLoop() { shutdown(); }

    void loop(bool stop_when_empty = true) {
        while (!stopped_) {
            bool processed = poll();
            processed = fd_event_source_poller_.poll([this] { fireDeferreds(); }) || processed;
            fireDeferreds();
            executeMainThreadTasks();

            if (!processed && stop_when_empty) {
                stopped_ = true;
            }
        }
        executeMainThreadTasks();
    }

    void shutdown() {
        if (background_thread_.joinable()) {
            background_thread_.join();
        }
    }

private:
    std::atomic<bool> stopped_{false};
    std::thread background_thread_;
    FDEventSourcePoller fd_event_source_poller_;
    std::queue<std::function<void()>> deferred_queue_;
    std::atomic<bool> deferred_queue_empty_{true};
};

} // namespace elaeo::comm::events
