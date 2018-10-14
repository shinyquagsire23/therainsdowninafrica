/*
 * Copyright (c) 2015-2018, SALT.
 * This file is part of therainsdowninafrica and is distributed under the 3-clause BSD license.
 * See LICENSE.md for terms of use.
 */

#ifndef FS_H
#define FS_H

#include "hos/hipc.h"
#include "hos/kobjects.h"

#define IFILE_READABLE   1
#define IFILE_WRITABLE   2
#define IFILE_APPENDABLE 4
#define IFILE_ALL (IFILE_READABLE | IFILE_WRITABLE | IFILE_APPENDABLE)

struct Handle
{
    u32 h;

    Handle(u32 handle = 0) : h(handle) {}

    u32 close()
    {
        return ksvcCloseHandle(h);
    }

    u32 toDomainId()
    {
        u32 domain_id = 0;

        HIPCCraftedPacket* p = new HIPCCraftedPacket();
        p->clear();

        p->type(HIPCPacketType_Control)->ipc_cmd(0)->send_to(h);

        domain_id = *p->get_data<u32>();
        p->free_data();

        delete p;

        return domain_id;
    }
};

struct DomainPair : Handle
{
    u32 d;
    DomainPair(u32 handle = 0, u32 domain = 0) : Handle(handle), d(domain) {}

    u32 closeDomain()
    {
        // Clean up straggling handles if we're returning errors
        HIPCCraftedPacket* p = new HIPCCraftedPacket();
        p->clear();
        p->domain_cmd(HIPCDomainCommand_CloseVirtualHandle)->send_to_domain(h, d);
        p->free_data();

        u32 retval = p->ret;

        delete p;

        return retval;
    }
};

struct IFile : Handle
{
    IFile(u32 handle = 0) : Handle(handle) {}
    
    u32 getSize(u64 *out)
    {
        HIPCPacket* packet = get_current_packet();
        HIPCCraftedPacket* p = new HIPCCraftedPacket();
        
        p->clear();

        p->ipc_cmd(4)->send_to(h);
        
        if (!p->ret && !p->ipcRet)
            *out = *p->get_data<u64>();
        p->free_data();

        u32 retval = p->ret ? p->ret : p->ipcRet;

        delete p;

        return retval;
    }

    u32 write(u64 offset, void *buf, size_t len)
    {
        HIPCPacket* packet = get_current_packet();
        HIPCCraftedPacket* p = new HIPCCraftedPacket();
        
        p->clear();

        //char* send_temp = (char*)packet + 0x80;
        //memcpy(send_temp, buf, len);

        p->ipc_cmd(1)->push_arg<u64>(1)->push_arg<u64>(offset)->push_arg<u64>(len)->push_send_buffer(buf, len, 1)->send_to(h);
        p->free_data();

        u32 retval = p->ret ? p->ret : p->ipcRet;

        delete p;

        return retval;
    }
};

struct IFileSystem : Handle
{
    IFileSystem(u32 handle = 0) : Handle(handle) {}

    u32 createFile(const char* path, u64 size = 0)
    {
        HIPCPacket* packet = get_current_packet();
        HIPCCraftedPacket* p = new HIPCCraftedPacket();
        
        p->clear();

        char* path_temp = (char*)packet + 0x80;
        strcpy(path_temp, path);

        p->ipc_cmd(0)->push_arg<u64>(0)->push_arg<u64>(size)->push_arg<u32>(0)->push_static_buffer(path_temp, 0x301)->send_to(h);
        p->free_data();

        u32 retval = p->ret ? p->ret : p->ipcRet;

        delete p;

        return retval;
    }

    u32 deleteFile(const char* path)
    {
        HIPCPacket* packet = get_current_packet();
        HIPCCraftedPacket* p = new HIPCCraftedPacket();
        
        p->clear();

        char* path_temp = (char*)packet + 0x80;
        strcpy(path_temp, path);

        p->ipc_cmd(1)->push_static_buffer(path_temp, 0x301)->send_to(h);
        p->free_data();

        u32 retval = p->ret ? p->ret : p->ipcRet;

        delete p;

        return retval;
    }
    
    u32 openFile(const char* path, u32 flags, IFile* out)
    {
        HIPCPacket* packet = get_current_packet();
        HIPCCraftedPacket* p = new HIPCCraftedPacket();
        
        p->clear();

        char* path_temp = (char*)packet + 0x80;
        strcpy(path_temp, path);

        p->ipc_cmd(8)->push_arg<u32>(flags)->push_static_buffer(path_temp, 0x301)->send_to(h);
        p->free_data();
        
        *out = IFile(p->get_handle(0));

        u32 retval = p->ret ? p->ret : p->ipcRet;

        delete p;

        return retval;
    }
};

struct FspSrv : Handle
{
    FspSrv(u32 handle = 0) : Handle(handle) {}
    
    u32 openSdCardFileSystem(IFileSystem *out)
    {
        HIPCCraftedPacket* p = new HIPCCraftedPacket();

        p->clear();
        p->ipc_cmd(18)->push_arg<u64>(0)->send_to(h);

        *out = IFileSystem(p->get_handle(0));
        p->free_data();

        u32 retval = p->ret ? p->ret : p->ipcRet;

        delete p;

        return retval;
    }
};

#endif // FS_H
