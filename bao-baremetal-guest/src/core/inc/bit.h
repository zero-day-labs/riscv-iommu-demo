/**
 * Bao, a Lightweight Static Partitioning Hypervisor
 *
 * Copyright (c) Bao Project (www.bao-project.org), 2019-
 *
 * Authors:
 *      Jose Martins <jose.martins@bao-project.org>
 *
 * Bao is free software; you can redistribute it and/or modify it under the
 * terms of the GNU General Public License version 2 as published by the Free
 * Software Foundation, with a special exception exempting guest code from such
 * license. See the COPYING file in the top-level directory for details.
 *
 */

#ifndef __BIT_H__
#define __BIT_H__

#include <stdint.h>

#define BIT_MASK(OFF, LEN) (((1UL << (OFF + LEN)) - 1) & ~((1UL << (OFF)) - 1))

#ifndef __ASSEMBLER__

static inline uint64_t bit_get(uint64_t word, uint64_t off)
{
    return word & (1UL << off);
}

static inline uint64_t bit_set(uint64_t word, uint64_t off)
{
    return word |= (1UL << off);
}

static inline uint64_t bit_clear(uint64_t word, uint64_t off)
{
    return word &= ~(1UL << off);
}

static inline uint64_t bit_extract(uint64_t word, uint64_t off, uint64_t len)
{
    return (word >> off) & BIT_MASK(0, len);
}

static inline uint64_t bit_insert(uint64_t word, uint64_t val, uint64_t off,
                                  uint64_t len)
{
    return (~BIT_MASK(off, len) & word) | ((BIT_MASK(0, len) & val) << off);
}

#endif /* |__ASSEMBLER__ */

#endif /* __BIT_H__ */
