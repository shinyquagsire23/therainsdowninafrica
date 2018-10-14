/*
 * Copyright (c) 2015-2018, SALT.
 * This file is part of therainsdowninafrica and is distributed under the 3-clause BSD license.
 * See LICENSE.md for terms of use.
 */

#include "types.h"
#include "utils.h"
#include "therainsdowninafrica_bin.h"

#include "io/car.h"
#include "io/timer.h"
#include "io/pinmux.h"
#include "io/uart.h"
#include "arm/mmu.h"
#include "arm/cache.h"

#define AFRICA_VADDR 0xffffffff00000000

void* (*kalloc)(u64 size) = 0x80060874; //TODO: search
extern void svc_intr_hook();
extern void except_hook();

bool once = false;
void *africa = NULL;

typedef struct africa_header
{
	u32 start;
	u32 except_start;
	u32 magic;
	u32 size;
	u32 text_end;
	u32 data_end;
	u32 africa_paddr;
	u32 africa_size;
	u32 except_shift;
	u32 except_shift_svca64
} africa_header;

void add_entry_to_ttb1(u64 *ttb1_lv1, u32 paddr, u64 vaddr, u64 lv12mask, u64 mask)
{
    u64 *ttb1_lv2, *ttb1_lv3;

    u16 lv1_idx, lv2_idx, lv3_idx;
    lv1_idx = (vaddr & TTB_LV1_MASK) >> TTB_LV1_SHIFT;
    lv2_idx = (vaddr & TTB_LV2_MASK) >> TTB_LV2_SHIFT;
    lv3_idx = (vaddr & TTB_LV3_MASK) >> TTB_LV3_SHIFT;
    
    u32 next_addr = ttb1_lv1[lv1_idx] & TTB_LV_ADDR_MASK;
    if (!next_addr)
    {
        next_addr = (u32)kalloc(0x1000);
        ttb1_lv1[lv1_idx] = next_addr | lv12mask;
    }
    
    ttb1_lv2 = (u64*)next_addr;
    next_addr = ttb1_lv2[lv2_idx] & TTB_LV_ADDR_MASK;
    if (!next_addr)
    {
        next_addr = (u32)kalloc(0x1000);
        ttb1_lv2[lv2_idx] = next_addr | lv12mask;
    }
    
    ttb1_lv3 = (u64*)next_addr;
    ttb1_lv3[lv3_idx] = paddr | mask;
}

void add_mem_range_to_ttb1(u64 *ttb1_lv1, u32 paddr, u64 vaddr, size_t size, int attr)
{
    for (size_t i = 0; i < size; i += 0x1000)
    {
        add_entry_to_ttb1(ttb1_lv1, paddr + i, vaddr + i, TTB_LV12_OR, TTB_MEM_LV3_OR | (attr << TTB_AP_SHIFT));
    }
}

void add_io_range_to_ttb1(u64 *ttb1_lv1, u32 paddr, u64 vaddr, size_t size, int attr)
{
    for (size_t i = 0; i < size; i += 0x1000)
    {
        add_entry_to_ttb1(ttb1_lv1, paddr + i, vaddr + i, TTB_IO_LV12_OR, TTB_IO_LV3_OR | (attr << TTB_AP_SHIFT));
    }
}

void patch_svc_a64()
{
    u32 svc64_blr = 0;
    for (svc64_blr = 0x80060000; svc64_blr; svc64_blr += 0x4)
    {
        if (*(u32*)svc64_blr == 0xD63F0160)
            break;
    }
    
    utils_memcpy((void*)svc64_blr-4, svc_intr_hook, 8);

    africa_header *header = (africa_header*)africa;
    header->except_shift_svca64 = svc64_blr-0x80060000+0x4;
}

void patch_exception()
{
    u32 exception_hook = 0;

    for (exception_hook = 0x80060000; exception_hook; exception_hook += 0x4)
    {
        if (*(u32*)exception_hook == 0xD5385201)
            break;
    }

    utils_memcpy((void*)exception_hook-4, except_hook, 12);

    africa_header *header = (africa_header*)africa;
    header->except_shift = exception_hook-0x80060000+0x8;
}

void main(u64* ttb1)
{
    africa_header *header = (africa_header*)therainsdowninafrica_bin;
    u32 africa_size_rounded = (header->size + 0x1000) & ~0xFFF;
    u32 text_size = header->text_end;
    u32 data_size = header->data_end - header->text_end;

    if (!once)
    {
        once = true;

        africa = kalloc(africa_size_rounded);
        header->africa_paddr = africa;
        header->africa_size = africa_size_rounded;

        utils_memcpy(africa, therainsdowninafrica_bin, therainsdowninafrica_bin_size);
        dcache_flush((void*)africa, therainsdowninafrica_bin_size);
    }
    add_mem_range_to_ttb1(ttb1, africa, AFRICA_VADDR, text_size, TTB_AP_UNO_KRO);
    add_mem_range_to_ttb1(ttb1, africa + text_size, AFRICA_VADDR + text_size, data_size, TTB_AP_UNO_KRW);
    
    add_io_range_to_ttb1(ttb1, CAR_PADDR, CAR_VADDR, 0x1000, TTB_AP_UNO_KRW);
    add_io_range_to_ttb1(ttb1, TMR_PADDR, TMR_VADDR, 0x1000, TTB_AP_UNO_KRW);
    add_io_range_to_ttb1(ttb1, PINMUX_PADDR, PINMUX_VADDR, 0x1000, TTB_AP_UNO_KRW);
    add_io_range_to_ttb1(ttb1, UART_PADDR, UART_VADDR, 0x1000, TTB_AP_UNO_KRW);
    patch_svc_a64();
    patch_exception();
}
