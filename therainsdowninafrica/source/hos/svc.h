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

extern u64* a64_svc_tbl;
extern void svcRunFunc(u64 *regs_in, u64 *regs_out, void *handler_ptr);

void svcs_init();

u64 ksvcSleepThread(u64 ns);
u32 ksvcCloseHandle(u64 handle);
u32 ksvcSendSyncRequest(u64 handle);
u64 ksvcGetSystemTick(void);

#ifdef __cplusplus
}
#endif

#endif // SVC_H
