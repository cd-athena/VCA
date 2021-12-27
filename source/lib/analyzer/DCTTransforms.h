#pragma once

#include <cstdint>

namespace vca {

typedef void (*dct_t)(const int16_t *src, int16_t *dst, intptr_t srcStride);

void dct8_c(const int16_t *src, int16_t *dst, intptr_t srcStride);
void dct16_c(const int16_t *src, int16_t *dst, intptr_t srcStride);
void dct32_c(const int16_t *src, int16_t *dst, intptr_t srcStride);

} // namespace vca
