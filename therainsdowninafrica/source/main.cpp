/*
 * Copyright (c) 2015-2018, SALT.
 * This file is part of therainsdowninafrica and is distributed under the 3-clause BSD license.
 * See LICENSE.md for terms of use.
 */

#include "types.h"
#include "utils.h"

#include <cstring>

#include "core/svc_bind.h"
#include "hos/kobjects.h"
#include "hos/kfuncs.h"
#include "hos/svc.h"
#include "hos/smc.h"
#include "arm/cache.h"
#include "arm/tls.h"
#include "arm/threading.h"
#include "io/uart.h"

#include "log.h"
#include "modules.h"

int svc_print_info(svcNumber svcno, u64 *regs_in, u64 *regs_out, void* handler_ptr);

bool has_initted = false;
static char g_heap[0x10000];
u64 g_aslrBase = 0;

void test_dump(u64* mem, size_t size)
{
    for (int i = 0; i < size / 8; i++)
    {
       uart_debug_printf("%x (%016llx) %016llx\n", i * 8, &mem[i], mem[i]);
    }
}

int bus_patch_hook(u64 *regs_in, u64 *regs_out, void* handler_ptr)
{
    KProcess* kproc = getCurrentContext()->pCurrentProcess;
    KProcess *new_proc = kproc->getKObjectFromHandle<KProcess>(regs_in[1]);
    
    u64 src, dst, size;
    src = regs_in[2];
    dst = regs_in[0];
    size = regs_in[3];

    log_printf("map %s from %s %016llx %016llx %016llx\r\n", new_proc->name, kproc->name, src, dst, size);
    
    if (!strcmp(new_proc->name, "qlaunch"))
        log_printf("ticks %016llx\r\n", ksvcGetSystemTick());
    // dst has NSO contents which can be searched through and patched.
    
    return 0;
}

int heap_hook(u64 *regs_in, u64 *regs_out, void* handler_ptr)
{
    KProcess* kproc = getCurrentContext()->pCurrentProcess;

    //TODO: a bit of a hack but at the same time it could just be my running a 6.0 kernel
    // on 5.0.1
    if (!strcmp(kproc->name, "hbloader"))
        regs_in[1] = 0x2000000*12;

    svcRunFunc(regs_in, regs_out, handler_ptr);

    return 1;
}

void init()
{
    extern char* fake_heap_start;
    extern char* fake_heap_end;
    fake_heap_start = &g_heap[0];
    fake_heap_end   = &g_heap[sizeof g_heap];
    
    if (has_initted) return;

    uart_init(UART_A, 115200);
    
    g_aslrBase = a64_svc_tbl[1] - 0x496DC; //TODO

    kfuncs_init();
    svcs_init();
    modules_init();
    
    // Bind SVCs
    //svc_pre_bind(idAll, (void*)svc_print_info);
    svc_pre_bind(idUnmapProcessMemory, (void*)bus_patch_hook);
    svc_pre_bind(idSetHeapSize, (void*)heap_hook);

    has_initted = true;
}

void main(u64 svcno, u64 *regs_in, u64 *regs_out, void* handler_ptr)
{
    init();

    if (!svc_pre((svcNumber)svcno, regs_in, regs_out, handler_ptr))
    {
        svcRunFunc(regs_in, regs_out, handler_ptr);
    }
    svc_post((svcNumber)svcno, regs_in, regs_out, handler_ptr);

    // artificial lag test
    //ksvcSleepThread(1000 * 500);
}



int svc_print_info(svcNumber svcno, u64 *regs_in, u64 *regs_out, void* handler_ptr)
{
    KProcess* kproc = getCurrentContext()->pCurrentProcess;
    
    //test_dump((u64*)&getCurrentContext()->pCurrentThread->idk, 0x6C0);

    log_printf("svc %x from proc %s (tid %016llx), %p\r\n", svcno, kproc->name, kproc->tid, getTls());
    return 0;
}
