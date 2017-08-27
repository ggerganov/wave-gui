/*! \file cg_ring_buffer.h
 *  \brief Enter description here.
 *  \author Georgi Gerganov
 */

#pragma once

#include <array>
#include <mutex>
#include <condition_variable>

namespace CG {

template <class TData, std::size_t BufferSize>
class RingBuffer {
public:
    RingBuffer() : head_(0), size_(0) {}

    TData pop() {
        std::unique_lock<std::mutex> lock(mutex_);
        cv_.wait(lock, [this] {return size_ != 0;});
        auto position = head_;
        if (++head_ >= BufferSize) head_ -= BufferSize;
        --size_;
        return std::move(buffer_[position]);
    }

    void pop(TData& item) {
        std::unique_lock<std::mutex> lock(mutex_);
        cv_.wait(lock, [this] {return size_ != 0;});
        item = std::move(buffer_[head_]);
        if (++head_ >= BufferSize) head_ -= BufferSize;
        --size_;
    }


    bool push(const TData& item) {
        std::unique_lock<std::mutex> lock(mutex_);
        if (size_ == BufferSize) {
            return false;
        }
        std::size_t idx = (head_ + size_);
        if (idx >= BufferSize) idx -= BufferSize;
        buffer_[idx] = item;
        ++size_;
        lock.unlock();
        cv_.notify_one();
        return true;
    }

    bool push(TData && item) {
        std::unique_lock<std::mutex> lock(mutex_);
        if (size_ == BufferSize) {
            return false;
        }
        std::size_t idx = (head_ + size_);
        if (idx >= BufferSize) idx -= BufferSize;
        buffer_[idx] = std::move(item);
        ++size_;
        lock.unlock();
        cv_.notify_one();
        return true;
    }

    std::size_t size() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return size_;
    }

    void clear() {
        std::lock_guard<std::mutex> lock(mutex_);
        head_ = 0;
        size_ = 0;
    }

private:
    mutable std::mutex mutex_;

    std::size_t head_;
    std::size_t size_;
    std::condition_variable cv_;

    std::array<TData, BufferSize> buffer_;
};

}
