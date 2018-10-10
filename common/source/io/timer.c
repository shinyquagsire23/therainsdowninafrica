/*
 * Copyright (c) 2015-2018, SALT.
 * This file is part of therainsdowninafrica and is distributed under the 3-clause BSD license.
 * See LICENSE.md for terms of use.
 */

#include "timer.h"
#include "types.h"

u64 timer_get_tick(void)
{
    return TIMERUS_CNTR_1US_0;
}

void timer_wait(u64 us)
{
    u64 end = TIMERUS_CNTR_1US_0 + us;
    while(TIMERUS_CNTR_1US_0 < end);
}
