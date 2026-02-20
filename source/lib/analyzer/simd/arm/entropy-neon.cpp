/*****************************************************************************
 * Copyright (C) 2025 Werner Robitza
 * Copyright (C) 2026 Christian Doppler Laboratory ATHENA
 *
 * Authors: Werner Robitza <werner.robitza@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.
 *****************************************************************************/

// Note: Entropy calculation involves histogram building and log2 operations
// which don't benefit significantly from SIMD vectorization. The current
// implementation falls back to the scalar C implementation which is already
// efficient for this algorithm. Future optimizations could explore:
// - Parallel histogram building using NEON for counting
// - Vectorized population count operations
// - SIMD-accelerated log2 approximations

#if defined(VCA_ARCH_ARM) || defined(__aarch64__) || defined(_M_ARM64)

#include <arm_neon.h>
#include <vector>
#include <cmath>

namespace vca {
namespace neon {

// Placeholder for future NEON-optimized entropy calculation
// Currently, entropy calculation uses the C implementation which is
// already efficient for histogram-based algorithms.
//
// The entropy calculation involves:
// 1. Building a histogram of pixel values (hard to vectorize)
// 2. Computing probabilities (simple division)
// 3. Computing -p*log2(p) for each unique value (hard to vectorize)
//
// NEON could potentially help with:
// - Faster memory access patterns
// - Parallel absolute difference calculations for edge density

} // namespace neon
} // namespace vca

#endif // VCA_ARCH_ARM
