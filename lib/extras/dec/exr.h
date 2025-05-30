// Copyright (c) the JPEG XL Project Authors. All rights reserved.
//
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#ifndef LIB_EXTRAS_DEC_EXR_H_
#define LIB_EXTRAS_DEC_EXR_H_

// Decodes OpenEXR images in memory.

#include <cstdint>

#include "lib/jxl/base/span.h"
#include "lib/jxl/base/status.h"

namespace jxl {

struct SizeConstraints;

namespace extras {

class ColorHints;
class PackedPixelFile;

bool CanDecodeEXR();

// Decodes `bytes` into `ppf`. color_hints are ignored.
Status DecodeImageEXR(Span<const uint8_t> bytes, const ColorHints& color_hints,
                      PackedPixelFile* ppf,
                      const SizeConstraints* constraints = nullptr);

}  // namespace extras
}  // namespace jxl

#endif  // LIB_EXTRAS_DEC_EXR_H_
