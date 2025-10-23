/**
 * \file locking_vector.h
 * \brief Thread-safe vector container with locking mechanisms.
 */

#ifndef EVGET_ASYNC_CONTAINER_LOCKING_VECTOR_H
#define EVGET_ASYNC_CONTAINER_LOCKING_VECTOR_H

#include <cstddef>
#include <mutex>
#include <optional>
#include <utility>
#include <vector>

namespace evget {

/**
 * \brief A thread-safe lockable vector implementation.
 * \tparam T The inner vector type.
 */
template <class T>
class LockingVector {
public:
    /**
     * \brief Default constructor for an empty LockingVector.
     */
    LockingVector();

    /**
     * \brief Construct a LockingVector with an initial vector.
     * \param inner initial vector to wrap
     */
    explicit LockingVector(std::vector<T> inner);

    /**
     * \brief Move data into the inner vector without locking. Not thread-safe.
     * \param value value to move.
     */
    constexpr void UnsafePushBack(T&& value);

    /**
     * \brief Move data into the inner vector by locking. Thread-safe.
     * \param value value to move.
     */
    constexpr void PushBack(T&& value);

    /**
     * \brief Consume the inner without locking. Not thread-safe.
     * \return the consumed vector if there are elements in it.
     */
    constexpr std::optional<std::vector<T>> UnsafeIntoInner();

    /**
     * \brief Consume the inner vector when its size is greater than or equal to
     * the size parameter without locking. Thread-safe.
     * \return the consumed vector if there are elements in it.
     */
    constexpr std::optional<std::vector<T>> IntoInner();

    /**
     * \brief Consume the inner vector when its size is greater than or equal to
     * the size parameter without locking. Not thread-safe.
     * \param size size to consume at.
     * \return the optionally consumed vector.
     */
    constexpr std::optional<std::vector<T>> UnsafeIntoInnerAt(std::size_t size);

    /**
     * \brief Consume the inner vector when its size is greater than or equal to
     * the size parameter by locking. Thread-safe.
     * \param size size to consume at.
     * \return the optionally consumed vector.
     */
    constexpr std::optional<std::vector<T>> IntoInnerAt(std::size_t size);

private:
    std::vector<T> inner_{};
    std::mutex lock_;
};

template <class T>
constexpr void LockingVector<T>::UnsafePushBack(T&& value) {
    inner_.push_back(std::move(value));
}

template <class T>
constexpr void LockingVector<T>::PushBack(T&& value) {
    const std::scoped_lock guard{lock_};
    UnsafePushBack(std::move(value));
}

template <class T>
constexpr std::optional<std::vector<T>> LockingVector<T>::UnsafeIntoInner() {
    auto out{std::move(inner_)};
    inner_.clear();
    return out;
}

template <class T>
constexpr std::optional<std::vector<T>> LockingVector<T>::IntoInner() {
    const std::scoped_lock guard{lock_};
    return UnsafeIntoInner();
}

template <class T>
constexpr std::optional<std::vector<T>> LockingVector<T>::UnsafeIntoInnerAt(std::size_t size) {
    if (inner_.size() >= size) {
        return UnsafeIntoInner();
    }

    return {};
}

template <class T>
constexpr std::optional<std::vector<T>> LockingVector<T>::IntoInnerAt(std::size_t size) {
    const std::scoped_lock guard{lock_};
    return UnsafeIntoInnerAt(size);
}

template <class T>
LockingVector<T>::LockingVector() = default;

template <class T>
LockingVector<T>::LockingVector(std::vector<T> inner) : inner_{inner} {}

} // namespace evget

#endif
