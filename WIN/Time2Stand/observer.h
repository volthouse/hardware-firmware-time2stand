#ifndef OBSERVER_H
#define OBSERVER_H

#include <functional>
#include <future>
#include <unordered_map>
#include <string>
#include <system_error>
#include <thread>
#include <mutex>

/**
 * A class that allows to subscribe to events and notify all subscribers.
 * @tparam Args The types of the arguments that are passed to the subscribed
 * callbacks.
 */
template <typename... Args>
class Observer
{
public:
    using Callback = std::function<void(Args...)>;

    /**
     * Adds a callback to the list of subscribed callbacks.
     * @param id The ID of the callback to add.
     * @param callback The callback function to add.
     * @return An error code indicating the success or failure of the operation.
     *         Returns std::errc::already_exists if a callback with the given ID
     * already exists.
     */
    std::error_code Subscribe(const std::string& id, Callback callback) noexcept
    {
        std::lock_guard<std::recursive_mutex> lock(m_observerMutex);
        auto [it, inserted] = m_callbacks.emplace(id, callback);
        if (!inserted) {
            return std::make_error_code(std::errc::file_exists);
        }
        return std::error_code();
    }

    /**
     * Checks whether a callback with the given ID is currently subscribed.
     * @param id The ID of the callback to check.
     * @return true if a callback with the given ID is currently subscribed, false
     * otherwise.
     */
    bool IsSubscribed(const std::string& id) const noexcept
    {
        std::lock_guard<std::recursive_mutex> lock(m_observerMutex);
        return m_callbacks.count(id) > 0;
    }

    /**
     * Removes the callback associated with the given ID from the list of
     * subscribed callbacks.
     * @param id The ID of the callback to remove.
     * @return An error code indicating the success or failure of the operation.
     *         Returns std::errc::no_such_file_or_directory if the ID is not
     * found.
     */
    std::error_code Unsubscribe(const std::string& id) noexcept
    {
        std::lock_guard<std::recursive_mutex> lock(m_observerMutex);
        auto it = m_callbacks.find(id);
        if (it == m_callbacks.end()) {
            return std::make_error_code(std::errc::no_such_file_or_directory);
        }
        m_callbacks.erase(it);
        return std::error_code();
    }

    /**
     * Notifies all subscribed callbacks with the given arguments.
     * @param args The arguments to pass to the subscribed callbacks.
     * @return A future that becomes ready when all callbacks have been
     * notified.
     */
    std::future<void> NotifyAsync(Args... args) noexcept
    {
        auto promise = std::make_shared<std::promise<void>>();
        std::thread([this, promise, args...] {
            Notify(args...);
            promise->set_value();
        }).detach();
        return promise->get_future();
    }

    /**
     * Notifies all subscribed callbacks with the given arguments.
     * @param args The arguments to pass to the subscribed callbacks.
     */
    void Notify(Args... args) noexcept
    {
        static_assert((std::is_same_v<Args, decltype(args)> && ...),
                      "Type mismatch in Notify arguments.");
        std::lock_guard<std::recursive_mutex> lock(m_observerMutex);
        auto callbacksCopy = m_callbacks;
        for (const auto& [id, callback] : callbacksCopy) {
            /* Check if the callback is still registered */
            if (m_callbacks.count(id) > 0) {
                try {
                    callback(args...);
                } catch (...) {
                    /* Ignore all exceptions as it is not possible to handle them
                        * here. */
                }
            }
        }
    }

private:
    mutable std::recursive_mutex m_observerMutex;
    std::unordered_map<std::string, Callback> m_callbacks;
};

#endif // OBSERVER_H
