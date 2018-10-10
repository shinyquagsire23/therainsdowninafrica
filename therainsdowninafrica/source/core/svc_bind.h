/*
 * Copyright (c) 2015-2018, SALT.
 * This file is part of therainsdowninafrica and is distributed under the 3-clause BSD license.
 * See LICENSE.md for terms of use.
 */

#ifndef _SVC_BIND_H
#define _SVC_BIND_H

#ifdef __cplusplus
extern "C" {
#endif

#include "types.h"

#define SVC_MAX_CALLBACKS (0x20)

typedef enum {
    idAll = 0x00,

    idSetHeapSize = 0x1,
    idSetMemoryPermission = 0x2,
    idSetMemoryAttribute = 0x3,
    idMapMemory = 0x4,
    idUnmapMemory = 0x5,
    idQueryMemory = 0x6,
    idExitProcess = 0x7,
    idCreateThread = 0x8,
    idStartThread = 0x9,
    idExitThread = 0xA,
    idSleepThread = 0xB,
    idGetThreadPriority = 0xC,
    idSetThreadPriority = 0xD,
    idGetThreadCoreMask = 0xE,
    idSetThreadCoreMask = 0xF,
    idGetCurrentProcessorNumber = 0x10,
    idSignalEvent = 0x11,
    idClearEvent = 0x12,
    idMapSharedMemory = 0x13,
    idUnmapSharedMemory = 0x14,
    idCreateTransferMemory = 0x15,
    idCloseHandle = 0x16,
    idResetSignal = 0x17,
    idWaitSynchronization = 0x18,
    idCancelSynchronization = 0x19,
    idArbitrateLock = 0x1A,
    idArbitrateUnlock = 0x1B,
    idWaitProcessWideKeyAtomic = 0x1C,
    idSignalProcessWideKey = 0x1D,
    idGetSystemTick = 0x1E,
    idConnectToNamedPort = 0x1F,
    idSendSyncRequestLight = 0x20,
    idSendSyncRequest = 0x21,
    idSendSyncRequestWithUserBuffer = 0x22,
    idSendAsyncRequestWithUserBuffer = 0x23,
    idGetProcessId = 0x24,
    idGetThreadId = 0x25,
    idBreak = 0x26,
    idOutputDebugString = 0x27,
    idReturnFromException = 0x28,
    idGetInfo = 0x29,
    idFlushEntireDataCache = 0x2A,
    idFlushDataCache = 0x2B,
    idMapPhysicalMemory = 0x2C,
    idUnmapPhysicalMemory = 0x2D,
    idGetFutureThreadInfo = 0x2E,
    idGetLastThreadInfo = 0x2F,
    idGetResourceLimitLimitValue = 0x30,
    idGetResourceLimitCurrentValue = 0x31,
    idSetThreadActivity = 0x32,
    idGetThreadContext3 = 0x33,
    idWaitForAddress = 0x34,
    idSignalToAddress = 0x35,
    idDumpInfo = 0x3C,
    idDumpInfoNew = 0x3D,
    idCreateSession = 0x40,
    idAcceptSession = 0x41,
    idReplyAndReceiveLight = 0x42,
    idReplyAndReceive = 0x43,
    idReplyAndReceiveWithUserBuffer = 0x44,
    idCreateEvent = 0x45,
    idMapPhysicalMemoryUnsafe = 0x48,
    idUnmapPhysicalMemoryUnsafe = 0x49,
    idSetUnsafeLimit = 0x4A,
    idCreateCodeMemory = 0x4B,
    idControlCodeMemory = 0x4C,
    idSleepSystem = 0x4D,
    idReadWriteRegister = 0x4E,
    idSetProcessActivity = 0x4F,
    idCreateSharedMemory = 0x50,
    idMapTransferMemory = 0x51,
    idUnmapTransferMemory = 0x52,
    idCreateInterruptEvent = 0x53,
    idQueryPhysicalAddress = 0x54,
    idQueryIoMapping = 0x55,
    idCreateDeviceAddressSpace = 0x56,
    idAttachDeviceAddressSpace = 0x57,
    idDetachDeviceAddressSpace = 0x58,
    idMapDeviceAddressSpaceByForce = 0x59,
    idMapDeviceAddressSpaceAligned = 0x5A,
    idMapDeviceAddressSpace = 0x5B,
    idUnmapDeviceAddressSpace = 0x5C,
    idInvalidateProcessDataCache = 0x5D,
    idStoreProcessDataCache = 0x5E,
    idFlushProcessDataCache = 0x5F,
    idDebugActiveProcess = 0x60,
    idBreakDebugProcess = 0x61,
    idTerminateDebugProcess = 0x62,
    idGetDebugEvent = 0x63,
    idContinueDebugEvent = 0x64,
    idGetProcessList = 0x65,
    idGetThreadList = 0x66,
    idGetDebugThreadContext = 0x67,
    idSetDebugThreadContext = 0x68,
    idQueryDebugProcessMemory = 0x69,
    idReadDebugProcessMemory = 0x6A,
    idWriteDebugProcessMemory = 0x6B,
    idSetHardwareBreakPoint = 0x6C,
    idGetDebugThreadParam = 0x6D,
    idGetSystemInfo = 0x6F,
    idCreatePort = 0x70,
    idManageNamedPort = 0x71,
    idConnectToPort = 0x72,
    idSetProcessMemoryPermission = 0x73,
    idMapProcessMemory = 0x74,
    idUnmapProcessMemory = 0x75,
    idQueryProcessMemory = 0x76,
    idMapProcessCodeMemory = 0x77,
    idUnmapProcessCodeMemory = 0x78,
    idCreateProcess = 0x79,
    idStartProcess = 0x7A,
    idTerminateProcess = 0x7B,
    idGetProcessInfo = 0x7C,
    idCreateResourceLimit = 0x7D,
    idSetResourceLimitLimitValue = 0x7E,
    idCallSecureMonitor = 0x7F,
} svcNumber;

int svc_pre(svcNumber number, u64* regs_in, u64* regs_out, void* default_handler);
int svc_post(svcNumber number, u64* regs_in, u64* regs_out, void* default_handler);

int svc_pre_bind(svcNumber number, void* callback);
int svc_pre_unbind(void* callback);

int svc_post_bind(svcNumber number, void* callback);
int svc_post_unbind(void* callback);

#ifdef __cplusplus
}
#endif

#endif
