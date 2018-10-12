/*
 * Copyright (c) 2015-2018, SALT.
 * This file is part of therainsdowninafrica and is distributed under the 3-clause BSD license.
 * See LICENSE.md for terms of use.
 */

#ifndef SVC_H
#define SVC_H

#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif

struct MemoryInfo
{
    u64 addr;
    u64 size;
    u32 type;
    u32 attr;
    u32 perm;
    u32 device_refcnt;
    u32 ipc_refcnt;
    u32 pad;
};

extern u64* a64_svc_tbl;
extern u32 africaPAddr;
extern u32 africaSize;
extern void svcRunFunc(u64 *regs_in, u64 *regs_out, void *handler_ptr);

void svcs_init();

u64 ksvcSleepThread(u64 ns);
u32 ksvcCloseHandle(u64 handle);
u32 ksvcSendSyncRequest(u64 handle);
u64 ksvcGetSystemTick(void);
u32 ksvcSetHeapSize(u64* out, u32 size);
u32 ksvcMapPhysicalMemory(u64 address, u64 size);
u32 ksvcUnmapPhysicalMemory(u64 address, u64 size);
u32 ksvcQueryMemory(struct MemoryInfo *meminfo, u32 *pageinfo, u64 addr);
u32 ksvcQueryPhysicalAddress(u64 *paddr, u64 *kaddr, u64 *size, u64 vaddr);
u32 ksvcQueryIoMapping(u64 *vaddr, u64 paddr, u64 size);
u32 ksvcSetProcessMemoryPermission(u32 kproc, u64 addr, u64 size, u32 perm);
u32 ksvcMapProcessMemory(u64 dst, u32 proc, u64 src, u64 size);
u32 ksvcUnmapProcessMemory(u64 dst, u32 proc, u64 src, u64 size);


#ifdef __cplusplus
}
#endif

#endif // SVC_H
