/*
 * Copyright (c) 2015-2018, SALT.
 * This file is part of therainsdowninafrica and is distributed under the 3-clause BSD license.
 * See LICENSE.md for terms of use.
 */

#ifndef MMU_H
#define MMU_H

#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif

#define TTB_LV1_MASK 0x7FC0000000
#define TTB_LV1_SHIFT 30
#define TTB_LV1_ADD 0x40000000
#define TTB_LV2_MASK 0x3FE00000
#define TTB_LV2_SHIFT 21
#define TTB_LV2_ADD 0x200000
#define TTB_LV3_MASK 0x1FF000
#define TTB_LV3_SHIFT 12
#define TTB_LV3_ADD 0x1000

#define TTB_LV12_OR 0x3000000000000003 
#define TTB_IO_LV12_OR 0x3800000000000003 
#define TTB_MEM_LV3_OR 0x40000000000003 | (0x102 << 2) /*id 2, AF*/
#define TTB_IO_LV3_OR  0x60000000000604 | 3
#define TTB_LV_ADDR_MASK 0xFFFFF000

#define TTB_AP_SHIFT 6
#define TTB_AP_UNO_KRW 0
#define TTB_AP_URW_KRW 1
#define TTB_AP_UNO_KRO 2
#define TTB_AP_URO_KRO 3

extern u64* getTTB1();

#ifdef __cplusplus
}
#endif

#endif // MMU_H
