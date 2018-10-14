/*
 * Copyright (c) 2015-2018, SALT.
 * This file is part of therainsdowninafrica and is distributed under the 3-clause BSD license.
 * See LICENSE.md for terms of use.
 */

#include "kfuncs.h"

#include "kobjects.h"
#include "svc.h"

#include "main.h"

u64 (*kproc_add_handle)(KProcessHandleTable* table, u32* out, void* obj, u16 id);
u32 (*kproc_map_mem)(KMemoryManager* memmanage, u64 paddr, u64 size, u8 perms);
u32 (*kproc_query_mem)(struct MemoryInfo* meminfo, u32* pageinfo, u32 kproc_hand, u64 addr);
void (*kprintf)(char* fmt, ...);

void kfuncs_init()
{
    kproc_add_handle = (u64 (*)(KProcessHandleTable* table, u32* out, void* obj, u16 id))(g_aslrBase + 0xABB0);
    kproc_map_mem = (u32 (*)(KMemoryManager* memmanage, u64 paddr, u64 size, u8 perms))(g_aslrBase + 0x19668);
    kproc_query_mem = (u32 (*)(struct MemoryInfo* meminfo, u32* pageinfo, u32 kproc_hand, u64 addr))(g_aslrBase + 0x3A0CC);
    kprintf = (void (*)(char* fmt, ...))(g_aslrBase + 0x1F44);
}


u64 find_vaddr_for_paddr(u64 paddr)
{
    u64 io_addr = 0;
    if (!ksvcQueryIoMapping(&io_addr, paddr, 0x1000))
        return io_addr;

    u64 addr = 0;
    while (1)
    {
        struct MemoryInfo meminfo;
        u32 pageinfo;
        u32 ret = kproc_query_mem(&meminfo, &pageinfo, H_KPROCESS, addr);

        u64 search_paddr, kaddr, size;
        ret = ksvcQueryPhysicalAddress(&search_paddr, &kaddr, &size, meminfo.addr);
        if (!ret && search_paddr == paddr)
            return addr;

        addr = meminfo.addr + meminfo.size;

        if (addr == 0) break;
    }

    return 0;
}

void* africa_kaddr_to_uaddr(void* addr)
{
    u64 vaddr = find_vaddr_for_paddr(africaPAddr);
    if (!vaddr)
    {
        kproc_map_mem(&getCurrentContext()->pCurrentProcess->memorymanager, africaPAddr, africaSize, 0x1b);
        vaddr = find_vaddr_for_paddr(africaPAddr);
    }

    if (!vaddr) return nullptr;

    return (void*)(((u64)addr) - AFRICA_KADDR + vaddr);
}
