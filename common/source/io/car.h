/*
 * Copyright (c) 2015-2018, SALT.
 * This file is part of therainsdowninafrica and is distributed under the 3-clause BSD license.
 * See LICENSE.md for terms of use.
 */

#ifndef _CAR_H_
#define _CAR_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "types.h"

#define CAR_PADDR ((void*)0x60006000)
#define CAR_VADDR ((void*)0xFFFFFFFF60006000)

typedef struct {
    u32 rst_dev_offset;
    u32 clk_out_enb_offset;
    u32 clk_source_offset;
    u8 dev_bit;
    u8 clk_source;
    u8 clk_divisor;
} car_device_info;

int car_disable_device(const car_device_info* info);
int car_enable_device(const car_device_info* info);

#define o(name)                          \
    int car_enable_ ## name (void);      \
    int car_disable_ ## name (void);     \
    bool car_ ## name ## _enabled(void);

#include "car_devices.in"

#undef o

#ifdef __cplusplus
}
#endif

#endif // _CAR_H_
