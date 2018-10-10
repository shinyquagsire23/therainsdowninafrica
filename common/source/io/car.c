/*
 * Copyright (c) 2015-2018, SALT.
 * This file is part of therainsdowninafrica and is distributed under the 3-clause BSD license.
 * See LICENSE.md for terms of use.
 */

#include "car.h"

#include "types.h"
#include "utils.h"
#include "timer.h"

static const car_device_info car_info_uart_a = {
    .rst_dev_offset = 0x004,
    .clk_out_enb_offset = 0x010,
    .clk_source_offset = 0x178,
    .dev_bit = 6,
    .clk_source = 0,
    .clk_divisor = 0,
};

static const car_device_info car_info_uart_b = {
    .rst_dev_offset = 0x004,
    .clk_out_enb_offset = 0x010,
    .clk_source_offset = 0x17C,
    .dev_bit = 7,
    .clk_source = 0,
    .clk_divisor = 0,
};

static const car_device_info car_info_uart_c = {
    .rst_dev_offset = 0x008,
    .clk_out_enb_offset = 0x014,
    .clk_source_offset = 0x1A0,
    .dev_bit = 23,
    .clk_source = 0,
    .clk_divisor = 0,
};

static const car_device_info car_info_uart_d = {
    .rst_dev_offset = 0x00C,
    .clk_out_enb_offset = 0x018,
    .clk_source_offset = 0x1C0,
    .dev_bit = 1,
    .clk_source = 0,
    .clk_divisor = 0,
};

int car_disable_device(const car_device_info* info)
{
    *(vu32*)(CAR_VADDR + info->rst_dev_offset) |= BIT(info->dev_bit);
    *(vu32*)(CAR_VADDR + info->clk_out_enb_offset) &= ~BIT(info->dev_bit);

    return 0;
}

int car_enable_device(const car_device_info* info)
{
    int ret = car_disable_device(info);
    if(ret) return ret;

    if(info->clk_source_offset)
        *(vu32*)(CAR_VADDR + info->clk_source_offset) = (info->clk_source << 29) | info->clk_divisor;

    *(vu32*)(CAR_VADDR + info->clk_out_enb_offset) |= BIT(info->dev_bit);
    *(vu32*)(CAR_VADDR + info->rst_dev_offset) &= ~BIT(info->dev_bit);

    return 0;
}

bool car_device_enabled(const car_device_info* info)
{
    if(*(vu32*)(CAR_VADDR + info->rst_dev_offset) & BIT(info->dev_bit)) return false;
    if(!(*(vu32*)(CAR_VADDR + info->clk_out_enb_offset) & BIT(info->dev_bit))) return false;
    return true;
}

#define o(name)                                         \
    int car_enable_ ## name (void)                      \
    { return car_enable_device(&car_info_ ## name); }   \
    int car_disable_ ## name (void)                     \
    { return car_disable_device(&car_info_ ## name); }  \
    bool car_ ## name ## _enabled(void)                 \
    { return car_device_enabled(&car_info_ ## name); }

#include "car_devices.in"

#undef o
