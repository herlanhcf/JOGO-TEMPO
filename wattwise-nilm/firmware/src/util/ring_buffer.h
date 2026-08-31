#pragma once
#include <atomic>
#include <cstddef>
#include <cstdint>

// Ring buffer SPSC lock-free: um produtor (sampler ISR/task), um consumidor (analytics).
// T deve ser trivialmente copiável. Capacidade fixa; sobrescrita NÃO permitida
// (o produtor larga a amostra se cheio e incrementa 'dropped' — melhor que travar a ISR).
template <typename T>
class RingBuffer {
 public:
  explicit RingBuffer(size_t capacity)
      : cap_(capacity + 1), buf_(new T[capacity + 1]) {}
  ~RingBuffer() { delete[] buf_; }

  bool push(const T& v) {
    const size_t head = head_.load(std::memory_order_relaxed);
    const size_t next = (head + 1) % cap_;
    if (next == tail_.load(std::memory_order_acquire)) {
      dropped_.fetch_add(1, std::memory_order_relaxed);
      return false;  // cheio
    }
    buf_[head] = v;
    head_.store(next, std::memory_order_release);
    return true;
  }

  bool pop(T& out) {
    const size_t tail = tail_.load(std::memory_order_relaxed);
    if (tail == head_.load(std::memory_order_acquire)) return false;  // vazio
    out = buf_[tail];
    tail_.store((tail + 1) % cap_, std::memory_order_release);
    return true;
  }

  size_t size() const {
    const size_t h = head_.load(std::memory_order_acquire);
    const size_t t = tail_.load(std::memory_order_acquire);
    return (h + cap_ - t) % cap_;
  }
  uint32_t dropped() const { return dropped_.load(std::memory_order_relaxed); }

 private:
  const size_t cap_;
  T* buf_;
  std::atomic<size_t> head_{0};
  std::atomic<size_t> tail_{0};
  std::atomic<uint32_t> dropped_{0};
};
