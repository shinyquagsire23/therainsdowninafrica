/*
 * Copyright (c) 2015-2018, SALT.
 * This file is part of therainsdowninafrica and is distributed under the 3-clause BSD license.
 * See LICENSE.md for terms of use.
 */

#ifndef KERN_500_H
#define KERN_500_H

#include "kern.h"

struct KAutoObject_500
{
    void *vtable;
    u32 refcnt;
    KLinkedListNode instances;
};
static_assert(sizeof(KAutoObject_500) == 0x20, "Bad KAutoObject size");

struct KSynchronizationObject_500 : KAutoObject_500
{
    KLinkedList threadSyncList;
};
static_assert(sizeof(KSynchronizationObject_500) == 0x38, "Bad KSynchronizationObject size");

#endif // KERN_500_H
