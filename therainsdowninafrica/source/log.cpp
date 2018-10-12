/*
 * Copyright (c) 2015-2018, SALT.
 * This file is part of therainsdowninafrica and is distributed under the 3-clause BSD license.
 * See LICENSE.md for terms of use.
 */

#include "log.h"

#include <cstdarg>

#include "io/uart.h"
#include "modules/sm.h"
#include "hos/kobjects.h"
#include "hos/kfuncs.h"
#include "hos/svc.h"
#include "hos/fs.h"

u64 log_lock;
char log_buffer[0x200];
bool log_fs_once = false;

//#define LOG_EXTRA_DEBUG

void log_printf(char* fmt, ...)
{
    va_list args;
    mutex_lock(&log_lock);
    
    va_start(args, fmt);
    vsnprintf(log_buffer, 0x200, fmt, args);
    
    va_end(args);
    
    uartDebugPrint(log_buffer);
    
    //TODO: don't flush from fs
    if (sm_is_sdcard_usable())
    {
        HIPCPacket* packet = get_current_packet();
        KClientSession* fspsrv = sm_get_fspsrv();
        FspSrv fsp;
        IFileSystem sdcard;
        IFile logfile;
        u32 ret, file_hand, file_ret;

        kproc_add_handle(&getCurrentContext()->pCurrentProcess->handleTable, (u32*)&fsp, fspsrv, KClientSessionTypeId);
        
        ret = fsp.openSdCardFileSystem(&sdcard);
#ifdef LOG_EXTRA_DEBUG
        uart_debug_printf("sdcard got return %x handle %x\r\n", ret, sdcard.h);
#endif
        
        if (!log_fs_once)
        {
            //delete
            ret = sdcard.deleteFile("/therainsdowninafrica.log");
#ifdef LOG_EXTRA_DEBUG
        uart_debug_printf("delete ret %x\r\n", ret);
#endif
            ret = sdcard.createFile("/therainsdowninafrica.log");
#ifdef LOG_EXTRA_DEBUG
        uart_debug_printf("create ret %x\r\n", ret);
#endif
            log_fs_once = true;
        }
        
        ret = sdcard.openFile("/therainsdowninafrica.log", 7, &logfile);
#ifdef LOG_EXTRA_DEBUG
        uart_debug_printf("open ret %x\r\n", ret);
#endif

        u64 pos;
        ret = logfile.getSize(&pos);
#ifdef LOG_EXTRA_DEBUG
        uart_debug_printf("size ret %x size %016llx\r\n", ret, pos);
#endif

        ret = logfile.write(pos, africa_kaddr_to_uaddr(log_buffer), strlen(log_buffer));
#ifdef LOG_EXTRA_DEBUG
        uart_debug_printf("write ret %x\r\n", ret);
#endif

        logfile.close();
        
        
file_failed:
        sdcard.close();
        fsp.close();
    }
    
    mutex_unlock(&log_lock);
}
