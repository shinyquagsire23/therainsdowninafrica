/*
 * Copyright (c) 2015-2018, SALT.
 * This file is part of therainsdowninafrica and is distributed under the 3-clause BSD license.
 * See LICENSE.md for terms of use.
 */

#ifndef SMC_H
#define SMC_H

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

extern u64 smcDebugPrint(char *print);
#define smcDebugPrintf(...) \
    {char log_buf[0x200]; snprintf(log_buf, 0x200, __VA_ARGS__); \
    smcDebugPrint(log_buf);}

#ifdef __cplusplus
}
#endif

#endif // SMC_H
