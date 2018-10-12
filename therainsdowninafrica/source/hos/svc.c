/*
 * Copyright (c) 2015-2018, SALT.
 * This file is part of therainsdowninafrica and is distributed under the 3-clause BSD license.
 * See LICENSE.md for terms of use.
 */

#include "svc.h"

u64 (*a64_svcSleepThread)(u64 ns);
u32 (*a64_svcSendSyncRequest)(u64 handle);
u32 (*a64_svcCloseHandle)(u64 handle);
u64 (*a64_svcGetSystemTick)(void);
u32 (*a64_svcMapPhysicalMemory)(u64 address, u64 size);
u32 (*a64_svcUnmapPhysicalMemory)(u64 address, u64 size);
u32 (*a64_svcSetProcessMemoryPermission)(u32 kproc, u64 addr, u64 size, u32 perm);
u32 (*a64_svcMapProcessMemory)(u64 dst, u32 proc, u64 src, u64 size);
u32 (*a64_svcUnmapProcessMemory)(u64 dst, u32 proc, u64 src, u64 size);

void svcs_init()
{
    a64_svcSleepThread = a64_svc_tbl[0x0B];
    a64_svcCloseHandle = a64_svc_tbl[0x16];
    a64_svcGetSystemTick = a64_svc_tbl[0x1E];
    a64_svcSendSyncRequest = a64_svc_tbl[0x21];
    a64_svcMapPhysicalMemory = a64_svc_tbl[0x2C];
    a64_svcUnmapPhysicalMemory = a64_svc_tbl[0x2D];

    a64_svcSetProcessMemoryPermission = a64_svc_tbl[0x73];
    a64_svcMapProcessMemory = a64_svc_tbl[0x74];
    a64_svcUnmapProcessMemory = a64_svc_tbl[0x75];
}

u64 ksvcSleepThread(u64 ns)
{
    return a64_svcSleepThread(ns);
}

u32 ksvcCloseHandle(u64 handle)
{
    return a64_svcCloseHandle(handle);
}

u32 ksvcSendSyncRequest(u64 handle)
{
    return a64_svcSendSyncRequest(handle);
}

u64 ksvcGetSystemTick(void)
{
    return a64_svcGetSystemTick();
}

u32 ksvcMapPhysicalMemory(u64 addr, u64 size)
{
    return a64_svcMapPhysicalMemory(addr, size);
}

u32 ksvcUnmapPhysicalMemory(u64 addr, u64 size)
{
    return a64_svcUnmapPhysicalMemory(addr, size);
}

u32 ksvcSetHeapSize(u64* out, u32 size)
{
    u64 regs_in[8], regs_out[8];
    regs_in[1] = size;
    svcRunFunc(regs_in, regs_out, a64_svc_tbl[0x01]);
    
    *out = regs_out[1];
    return regs_out[0];
}

u32 ksvcQueryMemory(struct MemoryInfo *meminfo, u32 *pageinfo, u64 addr)
{
    u64 regs_in[8], regs_out[8];
    memset(regs_in, 0, sizeof(regs_in));
    memset(regs_out, 0, sizeof(regs_out));

    regs_in[0] = (u64)meminfo;
    regs_in[2] = addr;
    svcRunFunc(regs_in, regs_out, a64_svc_tbl[0x6]);
    
    *pageinfo = regs_out[1];
    return regs_out[0];
}

u32 ksvcQueryPhysicalAddress(u64 *paddr, u64 *kaddr, u64 *size, u64 vaddr)
{
    u64 regs_in[8], regs_out[8];
    memset(regs_in, 0, sizeof(regs_in));
    memset(regs_out, 0, sizeof(regs_out));

    regs_in[1] = vaddr;
    svcRunFunc(regs_in, regs_out, a64_svc_tbl[0x54]);

    *paddr = regs_out[1];
    *kaddr = regs_out[2];
    *size = regs_out[3];
    return regs_out[0];
}

u32 ksvcQueryIoMapping(u64 *vaddr, u64 paddr, u64 size)
{
    u64 regs_in[8], regs_out[8];
    memset(regs_in, 0, sizeof(regs_in));
    memset(regs_out, 0, sizeof(regs_out));

    regs_in[1] = paddr;
    regs_in[2] = size;
    svcRunFunc(regs_in, regs_out, a64_svc_tbl[0x55]);

    *vaddr = regs_out[1];
    return regs_out[0];
}

u32 ksvcSetProcessMemoryPermission(u32 kproc, u64 addr, u64 size, u32 perm)
{
    return a64_svcSetProcessMemoryPermission(kproc, addr, size, perm);
}

u32 ksvcMapProcessMemory(u64 dst, u32 proc, u64 src, u64 size)
{
    return a64_svcMapProcessMemory(dst, proc, src, size);
}

u32 ksvcUnmapProcessMemory(u64 dst, u32 proc, u64 src, u64 size)
{
    return a64_svcUnmapProcessMemory(dst, proc, src, size);
}
