//===----------------------------------------------------------------------===//
//
//                         BusTub
//
// hyperloglog_presto.cpp
//
// Identification: src/primer/hyperloglog_presto.cpp
//
// Copyright (c) 2015-2025, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//

#include "primer/hyperloglog_presto.h"
#include <bitset>
#include <cstdint>
#include "common/util/hash_util.h"

namespace bustub {

/** @brief Parameterized constructor. */
template <typename KeyType>
HyperLogLogPresto<KeyType>::HyperLogLogPresto(int16_t n_leading_bits) : cardinality_(0), n_bits_(n_leading_bits) {
  if (n_bits_ < 0) {
    n_bits_ = 0;
  }
  dense_bucket_.resize(1ULL << n_bits_, 0);
}

/** @brief Element is added for HLL calculation. */
template <typename KeyType>
auto HyperLogLogPresto<KeyType>::AddElem(KeyType val) -> void {
  hash_t hash = CalculateHash(val);
  uint64_t index = 0;
  if (n_bits_ > 0) {
    index = hash >> (64 - n_bits_);
  }
  auto position_of_highest_one = [&hash, this]() -> uint16_t {
    auto bset = std::bitset<BITSET_CAPACITY>(hash);
    uint16_t limit = BITSET_CAPACITY - n_bits_;
    for (uint16_t i = 0; i < limit; i++) {
      if (bset.test(i)) {
        return i;
      }
    }
    return limit;
  };

  auto old_rank = static_cast<uint16_t>(dense_bucket_[index].to_ulong());
  if (overflow_bucket_.count(index) != 0U) {
    old_rank += static_cast<uint16_t>(overflow_bucket_[index].to_ulong() << 4);
  }

  uint16_t rank = position_of_highest_one();
  if (old_rank < rank) {
    dense_bucket_[index] = std::bitset<DENSE_BUCKET_SIZE>(rank);
    if ((rank & 0x70) != 0U) {
      overflow_bucket_[index] = std::bitset<OVERFLOW_BUCKET_SIZE>(rank >> 4);
    }
  }
}

/** @brief Function to compute cardinality. */
template <typename T>
auto HyperLogLogPresto<T>::ComputeCardinality() -> void {
  double sum = 0.0;
  // for (const auto &reg : registers_) {
  // sum += 1.0 / std::pow(2.0, reg);
  //}
  for (size_t i = 0; i < dense_bucket_.size(); i++) {
    auto rank = static_cast<uint16_t>(dense_bucket_[i].to_ulong());
    if (overflow_bucket_.count(i) != 0U) {
      auto ofval = static_cast<uint16_t>(overflow_bucket_[i].to_ulong());
      rank = (ofval << 4) + rank;
    }
    sum += 1.0 / std::pow(2.0, rank);
  }
  auto m = static_cast<double>(1ULL << n_bits_);
  double estimate = CONSTANT * m * m / sum;
  cardinality_ = static_cast<size_t>(std::floor(estimate));
}

template class HyperLogLogPresto<int64_t>;
template class HyperLogLogPresto<std::string>;
}  // namespace bustub
