#pragma once

#include <atomic>
#include <cstddef>
#include <vector>

template <typename T>
class RingBuffer {
private:
    std::vector<T> m_data;
    const std::size_t m_capacity;
    std::atomic<std::size_t> m_read = 0;
    std::atomic<std::size_t> m_write = 0;
public:
    /// @brief Creates `RingBuffer` with given `capacity`, which must be a power of two and cannot change after construction.
    explicit RingBuffer(std::size_t capacity)
        : m_data(std::vector<T>(capacity)),
          m_capacity{capacity}
    {}

    /// @brief Will attempt to push the value to the buffer at the write pointer. If it is full, it returns false.
    /// @param t Item to be pushed.
    /// @return True if pushed, false if not. Only advances if returned true.
    /// Should only be called in the producer thread.
    bool push(T&& t) {
        const auto w = m_write.load(std::memory_order_relaxed);
        const auto r = m_read.load(std::memory_order_acquire);
        
        // If full
        if (w - r == m_capacity)
            return false;
        
        m_data[w & (m_capacity - 1)] = std::move(t);
        m_write.store(w + 1, std::memory_order_release);
        return true;
    }

    /// @brief Will attempt to pop the value at the read pointer. If it is empty, it returns false.
    /// @param t Popped value.
    /// @return True if popped, false if not. Only advances and modifies `t` if true.
    bool pop(T& t) {
        const auto r = m_read.load(std::memory_order_relaxed);
        const auto w = m_write.load(std::memory_order_acquire);

        // If empty
        if (r == w)
            return false;
        
        t = std::move(m_data[r & (m_capacity - 1)]);
        m_read.store(r + 1, std::memory_order_release);
        return true;
    }

    std::size_t num_ready_to_read() const {
        const auto r = m_read.load(std::memory_order_relaxed);
        const auto w = m_write.load(std::memory_order_acquire);

        return w - r;
    }
};
