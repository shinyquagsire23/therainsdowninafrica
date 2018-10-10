/*
 * Copyright (c) 2015-2018, SALT.
 * This file is part of therainsdowninafrica and is distributed under the 3-clause BSD license.
 * See LICENSE.md for terms of use.
 */

#ifndef _UTILS_H_
#define _UTILS_H_
#include "types.h"

void *utils_memcpy(void *dst, const void *src, int len);
void *utils_memset (void *ptr, int value, int len);
int utils_memcmp(const void* ptr1, const void* ptr2, int len);

#define swap16(x) __builtin_bswap16(x)
#define swap32(x) __builtin_bswap32(x)
#define swap64(x) __builtin_bswap64(x)

#define getle16(x) *(u16*)x
#define getle32(x) *(u32*)x
#define getle64(x) *(u64*)x
#define getbe16(x) swap16(*(u16*)x)
#define getbe32(x) swap32(*(u32*)x)
#define getbe64(x) swap64(*(u64*)x)

#endif // _UTILS_H_
