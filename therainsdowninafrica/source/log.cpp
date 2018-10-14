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

const char* LOG_FNAME = "/therainsdowninafrica.log";

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
        u64 pos;

        kproc_add_handle(&getCurrentContext()->pCurrentProcess->handleTable, (u32*)&fsp, fspsrv, KClientSessionTypeId);

        ret = fsp.openSdCardFileSystem(&sdcard);
        if (ret)
        {
            uart_debug_printf("log: openSdCardFileSystem got return %x, %x\r\n", ret, sdcard.h);
            goto sdcard_failed;
        }

        if (!log_fs_once)
        {
            sdcard.deleteFile(LOG_FNAME);
            sdcard.createFile(LOG_FNAME);
            log_fs_once = true;
        }

        ret = sdcard.openFile(LOG_FNAME, IFILE_ALL, &logfile);
        if (ret)
        {
            uart_debug_printf("log: openFile(%s) got return %x\r\n", LOG_FNAME, ret);
            goto file_failed;
        }

        ret = logfile.getSize(&pos);
        ret = logfile.write(pos, africa_kaddr_to_uaddr(log_buffer), strlen(log_buffer));

        logfile.close();

file_failed:
        sdcard.close();
sdcard_failed:
        fsp.close();
    }
    
    mutex_unlock(&log_lock);
}
