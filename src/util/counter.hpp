#pragma once
#include <atomic>

namespace xyl::util {

class Counter {
 public:
  void hit() { n_.fetch_add(1, std::memory_order_relaxed); }
  unsigned long long value() const { return n_.load(std::memory_order_relaxed); }

 private:
  std::atomic<unsigned long long> n_{0};
};

} // namespace xyl::util
