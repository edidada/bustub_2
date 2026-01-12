//===----------------------------------------------------------------------===//
//
//                         BusTub
//
// hyperloglog.cpp
//
// Identification: src/primer/hyperloglog.cpp
//
// Copyright (c) 2015-2025, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//

#include "primer/hyperloglog.h"
#include <sys/types.h>
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <mutex>
#include <shared_mutex>

namespace bustub {

/** @brief Parameterized constructor. */
template <typename KeyType>
HyperLogLog<KeyType>::HyperLogLog(int16_t n_bits)
    : cardinality_(0), n_bits_(std::max(n_bits, static_cast<int16_t>(0))) {
  registers_.resize(1ULL << n_bits_, 0);
}

/**
 * @brief Function that computes binary.
 *
 * @param[in] hash
 * @returns binary of a given hash
 */
template <typename KeyType>
auto HyperLogLog<KeyType>::ComputeBinary(const hash_t &hash) const -> std::bitset<BITSET_CAPACITY> {
  return {hash};
}

/**
 * @brief Function that computes leading zeros.
 *
 * @param[in] bset - binary values of a given bitset
 * @returns leading zeros of given binary set
 */
template <typename KeyType>
auto HyperLogLog<KeyType>::PositionOfLeftmostOne(const std::bitset<BITSET_CAPACITY> &bset) const -> uint64_t {
  uint64_t start = BITSET_CAPACITY - n_bits_;
  // loop range: [start - 1, 0]
  for (int i = start - 1; i >= 0; --i) {
    if (bset.test(i)) {
      return start - i;
    }
  }
  return start + 1;
}

/**
 * @brief Adds a value into the HyperLogLog.
 *
 * @param[in] val - value that's added into hyperloglog
 */
template <typename KeyType>
auto HyperLogLog<KeyType>::AddElem(KeyType val) -> void {
  hash_t hash = CalculateHash(val);
  // compute bucket index
  uint64_t index = hash >> (BITSET_CAPACITY - n_bits_);
  std::bitset<BITSET_CAPACITY> bset = ComputeBinary(hash);
  auto position = static_cast<uint8_t>(PositionOfLeftmostOne(bset));
  if (n_bits_ == 0) {
    index = 0;
  }

  std::unique_lock lock(rwlock_);
  if (registers_[index] < position) {
    registers_[index] = position;
  }
}

/**
 * @brief Function that computes cardinality.
 */
template <typename KeyType>
auto HyperLogLog<KeyType>::ComputeCardinality() -> void {
  std::unique_lock lock(rwlock_);

  double sum = 0.0;
  for (const auto &reg : registers_) {
    sum += 1.0 / std::pow(2.0, reg);
  }
  auto m = static_cast<double>(1ULL << n_bits_);
  double estimate = CONSTANT * m * m / sum;
  cardinality_ = static_cast<size_t>(std::floor(estimate));
}

template class HyperLogLog<int64_t>;
template class HyperLogLog<std::string>;

}  // namespace bustub
