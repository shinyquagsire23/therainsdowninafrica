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

void svcs_init()
{
    a64_svcSleepThread = a64_svc_tbl[0x0B];
    a64_svcCloseHandle = a64_svc_tbl[0x16];
    a64_svcGetSystemTick = a64_svc_tbl[0x1E];
    a64_svcSendSyncRequest = a64_svc_tbl[0x21];
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
