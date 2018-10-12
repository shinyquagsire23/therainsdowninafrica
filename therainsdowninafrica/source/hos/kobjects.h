/*
 * Copyright (c) 2015-2018, SALT.
 * This file is part of therainsdowninafrica and is distributed under the 3-clause BSD license.
 * See LICENSE.md for terms of use.
 */

#ifndef KERN_H
#define KERN_H

#include "types.h"

#define H_KTHREAD  0xFFFF800
#define H_KPROCESS 0xFFFF8001

typedef void KResourceLimit; //TODO
struct KSession;
struct KProcess;
struct KPort;

enum KObjectTypeId : u8
{
    KProcessTypeId = 0x1501,
    KClientSessionTypeId = 0xd00,
};

struct KLinkedListNode
{
    KLinkedListNode *prev;
    KLinkedListNode *next;
};
static_assert(sizeof(KLinkedListNode) == 0x10, "Bad KLinkedListNode size");

struct KLinkedList
{
    u64 count;
    KLinkedListNode bounds;
};
static_assert(sizeof(KLinkedList) == 0x18, "Bad KLinkedList size");

struct KAutoObject
{
    void **vtable;
    u32 refcnt;
    u64 idk_0;
    void* idk_1;
    u64 idk_2;
    u64 idk_3;
};
static_assert(sizeof(KAutoObject) == 0x30, "Bad KAutoObject size");

struct KMutex
{
    u64 ownerTag;
};
static_assert(sizeof(KMutex) == 0x8, "Bad KMutex size");

struct KSynchronizationObject : KAutoObject
{
    KLinkedList threadSyncList;
};
static_assert(sizeof(KSynchronizationObject) == 0x48, "Bad KSynchronizationObject size");

struct KProcessTerminationMessage
{
    u64 a;
    u64 b;
};
static_assert(sizeof(KProcessTerminationMessage) == 0x10, "Bad KSynchronizationObject size");

struct KPageTable
{
    void *rawPageTable;
    bool unk;
    u32 addrSpaceSizeInGib;
};
static_assert(sizeof(KPageTable) == 0x10, "Bad KPageTable size");

struct KMemoryBlockManager
{
    //KMemoryBlock *list;
    void *list;
    u64 minAddr;
    u64 maxAddr;
};
static_assert(sizeof(KMemoryBlockManager) == 0x18, "Bad KMemoryBlockManager size");

struct KMemoryManager
{
    void *vtable;
    u64 execBaseAddr;
    u64 execEndAddr;

    u64 heapBaseAddr;
    u64 heapEndAddr;
    u64 heapCurAddr;

    u64 mapBaseAddr;
    u64 mapEndAddr;

    u64 newmapBaseAddr;
    u64 newmapEndAddr;

    u64 tlsIoBaseAddr;
    u64 tlsIoEndAddr;
    
    u64 heapMaxAlloc;
    
    u64 unkBaseAddr;
    u64 unkEndAddr;

    KMutex lock;
    KPageTable pageTable;
    KMemoryBlockManager memoryBlockManager;

    u32 unk;
    u32 addrSpaceWidth;
    
    KLinkedList maybeList;
    u64 unk2;
    u64 unk3;
    void* unk4;

    u64 ttbr0;
    u64 tcr;
    u32 asid;
};
static_assert(sizeof(KMemoryManager) == 0xF8, "Bad KMemoryManager size");

struct KServerPort : KSynchronizationObject
{
    KLinkedListNode incomingConnections;
    KLinkedListNode incomingLightConnections;
    KPort* parentPort;
};

struct KClientPort : KSynchronizationObject
{
    u32 numSessions;
    u32 maxSessions;
    u64 unk;
    KPort* parent;
};

struct KPort : KAutoObject
{
    KServerPort server;
    KClientPort client;
    u64 idk;
    u8 hasInitted;
    u8 isLight;
    u8 filler[2];
};

struct KClientSession : KAutoObject
{
    KSession* parentSession;
    u64 hasInited;
    KClientPort* parentPort;
    u64 unk;
    KProcess* parentProcess;
};

struct KServerSession : KSynchronizationObject
{
    KLinkedListNode incomingConnections;
    KSession* parent;
    KLinkedListNode requestList;
    void* activeRequest;
    u64 unk;
};

struct KSession : KAutoObject
{
    KServerSession server;
    KClientSession client;
    u64 hasInitted;
};

struct KProcessCapabilities
{
    u8 svcAccessMask[16];
    u8 irqAccessMask[128];
    u64 allowedCpuIds;
    u64 allowedThreadPrios;
    u32 debugFlags;
    u32 handleTableSize;
    u32 kernelReleaseVersion;
    u32 applicationType;
};
static_assert(sizeof(KProcessCapabilities) == 0xB0, "Bad KProcessCapabilities size");

struct KSpinLock
{
    u8 idk[0x40];
};

struct KHandleEntry
{
    u16 id;
    u16 typeId;
    u32 unk2;
    void* obj;
};

struct KProcessHandleTable
{
    KHandleEntry* first;
    KHandleEntry* next;
    KHandleEntry entries[1024];
    u16 size;
    u16 maxUsageAtOnce;
    u16 handleCounter;
    u16 handlesActive;
    KSpinLock spinlock;
};

struct KProcess : KSynchronizationObject
{
    KProcessTerminationMessage terminateMessage;
    KMemoryManager memorymanager;
    u64 unk;
    void* unk2;
    u64 defaultCpuCore;
    u64 unk3;
    KResourceLimit *resLimit;
    u64 unk4;
    u64 unk5;
    u64 unk6;
    u64 unk7;
    u64 unk8;
    u64 unk9;
    u64 unk10;
    u64 unk11;
    u64 randomEntropy[4];
    u8 hasStateChanged;
    u8 hasInitialized;
    u8 isSystem;
    char name[12+1];
    u16 numCreatedThreads;
    u16 threadingUnk;
    u32 flags;
    KProcessCapabilities caps;
    u64 unk12; // maybe part of caps
    u64 unk13; // ditto
    u64 tid;
    u64 pid;
    u64 createdTickstamp;
    u64 entrypoint;
    u64 codeMemUsage;
    u64 dynMemUsage;
    u64 maxMemUsage;
    u32 processCategory;
    u64 unk14;
    u64 unk15;
    KProcessHandleTable handleTable;
    u64 unk16[13];
    void* usermodeExceptionTlsArea;
    KLinkedListNode exceptionThreadList;
    void* exceptionThread;
    KLinkedListNode threadList;
    KLinkedListNode mappedSharedMemoriesList;
    u64 unk17[7];
    void* unk18;
    u16 unk19[16];
    u64 idk;

    KHandleEntry* getHandleEntry(u32 handle)
    {
        return &this->handleTable.entries[handle & 0x7FFF];
    }

    template <class T>
    T* getKObjectFromHandle(u32 handle)
    {
        KHandleEntry* entry = getHandleEntry(handle);
        if (entry->id == handle >> 15)
            return (T*)entry->obj;

        return nullptr;
    }
    
    template <class T>
    void setKObjectForHandle(u32 handle, T* kobj)
    {
        KHandleEntry* entry = getHandleEntry(handle);
        entry->id = handle >> 15;
        entry->obj = (void*)kobj;
    }
};

struct KThread : KSynchronizationObject
{
    u8 idk[0x308];
    u64 entrypoint; // 308+48
    u64 condvar_mutexuseraddr; //310+48
    KProcess* parent; // 318+48
    void* kernel_thread_stack; // 320+48
    u64 unk0; // 328+48
    void* tls_userland; // 330+48
    void* tls_kernel; // 338+48
};

struct KCurrentContext
{
    KThread *pCurrentThread;
    KProcess *pCurrentProcess;
    void *pScheduler;
    void *pInterruptTaskManager;
};
//static_assert(sizeof(KCurrentContext) == 0x20, "Bad KCurrentContext size");

#ifdef __cplusplus
extern "C" {
#endif

KCurrentContext* getCurrentContext();

static inline void* getTls(void) 
{
    return getCurrentContext()->pCurrentThread->tls_userland;
}

#ifdef __cplusplus
}
#endif

#endif // KERN_H
