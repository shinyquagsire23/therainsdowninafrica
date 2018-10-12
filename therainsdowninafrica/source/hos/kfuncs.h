/*
 * Copyright (c) 2015-2018, SALT.
 * This file is part of therainsdowninafrica and is distributed under the 3-clause BSD license.
 * See LICENSE.md for terms of use.
 */

#ifndef KERNFUNCS_H
#define KERNFUNCS_H

#include "types.h"
#include "kobjects.h"

extern u64 (*kproc_add_handle)(KProcessHandleTable* table, u32* out, void* obj, u16 id);
extern u32 (*kproc_map_mem)(KMemoryManager* memmanage, u64 paddr, u64 size, u8 perms);
extern u32 (*kproc_query_mem)(struct MemoryInfo* meminfo, u32* pageinfo, u32 kproc_hand, u64 addr);

void kfuncs_init();
u64 find_vaddr_for_paddr(u64 paddr);
void* africa_kaddr_to_uaddr(void* addr);

#endif // KERNFUNCS_H
