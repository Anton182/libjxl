// Copyright (c) the JPEG XL Project Authors. All rights reserved.
//
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#ifndef LIB_JXL_ENC_ANS_PARAMS_H_
#define LIB_JXL_ENC_ANS_PARAMS_H_

// Encoder-only parameter needed for ANS entropy encoding methods.

#include <stdint.h>
#include <stdlib.h>

#include <vector>

#include "lib/jxl/ans_common.h"
#include "lib/jxl/common.h"

namespace jxl {

// Forward declaration to break include cycle.
struct CompressParams;

// RebalanceHistogram requires a signed type.
using ANSHistBin = int32_t;

struct HistogramParams {
  enum class ClusteringType {
    kFastest,  // Only 4 clusters.
    kFast,
    kBest,
  };

  enum class HybridUintMethod {
    kNone,        // just use kHybridUint420Config.
    k000,         // force the fastest option.
    kFast,        // just try a couple of options.
    kContextMap,  // fast choice for ctx map.
    kBest,
  };

  enum class LZ77Method {
    kNone,     // do not try lz77.
    kRLE,      // only try doing RLE.
    kLZ77,     // try lz77 with backward references.
    kOptimal,  // optimal-matching LZ77 parsing.
  };

  enum class ANSHistogramStrategy {
    kFast,         // Only try some methods, early exit.
    kApproximate,  // Only try some methods.
    kPrecise,      // Try all methods.
  };

  HistogramParams() = default;

  HistogramParams(SpeedTier tier, size_t num_ctx) {
    if (tier > SpeedTier::kFalcon) {
      clustering = ClusteringType::kFastest;
      lz77_method = LZ77Method::kNone;
    } else if (tier > SpeedTier::kTortoise) {
      clustering = ClusteringType::kFast;
    } else {
      clustering = ClusteringType::kBest;
    }
    if (tier > SpeedTier::kTortoise) {
      uint_method = HybridUintMethod::kNone;
    }
    if (tier >= SpeedTier::kSquirrel) {
      ans_histogram_strategy = ANSHistogramStrategy::kApproximate;
    }
  }

  static HistogramParams ForModular(
      const CompressParams& cparams,
      const std::vector<uint8_t>& extra_dc_precision, bool streaming_mode);

  ClusteringType clustering = ClusteringType::kBest;
  HybridUintMethod uint_method = HybridUintMethod::kBest;
  LZ77Method lz77_method = LZ77Method::kRLE;
  ANSHistogramStrategy ans_histogram_strategy = ANSHistogramStrategy::kPrecise;
  std::vector<size_t> image_widths;
  size_t max_histograms = ~0;
  bool force_huffman = false;
  bool initialize_global_state = true;
  bool streaming_mode = false;
  bool add_missing_symbols = false;
  bool add_fixed_histograms = false;
};

struct Histogram {
  // Create flat histogram
  static Histogram Flat(int length, int total_count) {
    Histogram flat;
    flat.counts_ = CreateFlatHistogram(length, total_count);
    flat.total_count_ = static_cast<size_t>(total_count);
    flat.entropy_ = 0.0;
    return flat;
  }
  void Clear() {
    counts_.clear();
    total_count_ = 0;
    entropy_ = 0.0;
  }
  void Add(size_t symbol) {
    if (counts_.size() <= symbol) {
      counts_.resize(DivCeil(symbol + 1, kRounding) * kRounding);
    }
    ++counts_[symbol];
    ++total_count_;
  }
  void AddHistogram(const Histogram& other) {
    if (other.counts_.size() > counts_.size()) {
      counts_.resize(other.counts_.size());
    }
    for (size_t i = 0; i < other.counts_.size(); ++i) {
      counts_[i] += other.counts_[i];
    }
    total_count_ += other.total_count_;
  }
  size_t alphabet_size() const {
    for (int i = counts_.size() - 1; i >= 0; --i) {
      if (counts_[i] > 0) {
        return i + 1;
      }
    }
    return 0;
  }

  // Returns an estimate of the number of bits required to encode the given
  // histogram (header bits plus data bits).
  StatusOr<float> ANSPopulationCost() const;

  float ShannonEntropy() const;

  std::vector<ANSHistBin> counts_;
  size_t total_count_ = 0;
  mutable float entropy_ = 0;  // WARNING: not kept up-to-date.
  static constexpr size_t kRounding = 8;
};

}  // namespace jxl

#endif  // LIB_JXL_ENC_ANS_PARAMS_H_
