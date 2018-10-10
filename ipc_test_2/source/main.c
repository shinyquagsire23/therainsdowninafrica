// Copyright 2017 plutoo
#include <switch.h>
//#include "kernelhax.h"

#include <string.h>
#include <stdio.h>
#include <switch/kernel/ipc.h>

u32 __nx_applet_type = AppletType_None;

static char g_heap[0x20000];

void __libnx_initheap(void)
{
    extern char* fake_heap_start;
    extern char* fake_heap_end;

    fake_heap_start = &g_heap[0];
    fake_heap_end   = &g_heap[sizeof g_heap];
}

void __appInit(void)
{
    
}

#define write_log(...) \
    {char log_buf[0x200]; snprintf(log_buf, 0x200, __VA_ARGS__); \
    svcOutputDebugString(log_buf, strlen(log_buf));}

void proddingThread(int core)
{
    while (1)
    {
        write_log("hello thread from core %u\n", core);
        svcSleepThread(0);
    }
}

void send_message(Handle port, char* msg)
{
    Result ret;

    // Send a command
    IpcCommand c;
    ipcInitialize(&c);
    ipcAddSendBuffer(&c, msg, strlen(msg)+1, BufferType_Normal);
    ipcSendPid(&c);

    struct {
        u64 magic;
        u64 cmd_id;
        u64 reserved[2];
    } *raw;

    raw = ipcPrepareHeader(&c, sizeof(*raw));

    raw->magic = SFCI_MAGIC;
    raw->cmd_id = 0xf00ff00f;

    //write_log("sending IPC request\n");
    ret = ipcDispatch(port);

    if (R_SUCCEEDED(ret)) {
        IpcParsedCommand r;
        ipcParse(&r);

        struct {
            u64 magic;
            u64 result;
            char msg[0x10];
        } *resp = r.Raw;

        ret = resp->result;
        write_log("Got reply\n%s\n", resp->msg);
    }
    else
    {
        write_log("IPC dispatch failed, %x\n", ret);
    }
}

int main(int argc, char *argv[])
{
    Result ret;

    Handle port;
    ret = -1;
    do
    {
        ret = svcConnectToNamedPort(&port, "ipc_test");
        write_log("svcConnectToNamedPort returned %x, handle %x\n", ret, port);
        
        if (ret)
            svcCloseHandle(port);
    }
    while (ret);
    
    send_message(port, "Hey STUD!\nLOL j/k");
    send_message(port, "What's going on??\n;)");

    svcCloseHandle(port);

    return 0;
}

