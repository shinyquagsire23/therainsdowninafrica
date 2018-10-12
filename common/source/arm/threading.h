/*
 * Copyright (c) 2015-2018, SALT.
 * This file is part of therainsdowninafrica and is distributed under the 3-clause BSD license.
 * See LICENSE.md for terms of use.
 */

#ifndef THREADING_H
#define THREADING_H

#ifdef __cplusplus
extern "C" {
#endif

extern void mutex_lock(u64* mutex);
extern void mutex_unlock(u64* mutex);
extern int get_core();
extern void* getSP_EL0();

#ifdef __cplusplus
}
#endif

#endif // THREADING_H
