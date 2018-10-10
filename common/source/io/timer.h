/*
 * Copyright (c) 2015-2018, SALT.
 * This file is part of therainsdowninafrica and is distributed under the 3-clause BSD license.
 * See LICENSE.md for terms of use.
 */

#ifndef _TIMER_H_
#define _TIMER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "types.h"
#include "utils.h"

#define TMR_PADDR                      ((void*)0x60005000)
#define TMR_VADDR                      ((void*)0xFFFFFFFF60005000)

#define TIMERUS_CNTR_1US_0            (*(vu32*)(TMR_VADDR + 0x010))

u64 timer_get_tick(void);
void timer_wait(u64 us);

#ifdef __cplusplus
}
#endif

#endif // _TIMER_H_
