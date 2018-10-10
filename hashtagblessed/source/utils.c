/*
 * Copyright (c) 2015-2018, SALT.
 * This file is part of therainsdowninafrica and is distributed under the 3-clause BSD license.
 * See LICENSE.md for terms of use.
 */

#include "utils.h"

// simple fast-enough memcpy implementation
void *utils_memcpy(void *dst, const void *src, int len)
{
    u8 *dst8 = (u8*)dst;
    u8 *src8 = (u8*)src;
    while(len >= 8 && !((u64)src8 & 0x7) && !((u64)dst8 & 0x7)) {
        *(u64*)dst8 = *(u64*)src8;
        dst8 += 8;
        src8 += 8;
        len -= 8;
    }
    while(len >= 4 && !((u64)src8 & 0x3) && !((u64)dst8 & 0x3)) {
        *(u32*)dst8 = *(u32*)src8;
        dst8 += 4;
        src8 += 4;
        len -= 4;
    }
    while(len) {
        *dst8++ = *src8++;
        len--;
    }
    
    return dst;
}

void *utils_memset(void *ptr, int value, int len)
{
    u8 *ptr8 = (u8*)ptr;
    while(len) {
        *ptr8++ = (u8)value;
        len--;
    }
    return ptr;
}

int utils_memcmp(const void* ptr1, const void* ptr2, int len)
{
    const u8* ptr1_8 = (u8*)ptr1;
    const u8* ptr2_8 = (u8*)ptr2;
    for(int i = 0; i < len; i++)
    {
        if(ptr1_8[i] != ptr2_8[i])
            return ptr1_8[i] - ptr2_8[i];
    }
    
    return 0;
}
