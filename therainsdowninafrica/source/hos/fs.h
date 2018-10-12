#ifndef FS_H
#define FS_H

#include "hos/hipc.h"
#include "hos/kobjects.h"

struct Handle
{
    u32 h;
    
    Handle(u32 handle = 0) : h(handle) {}
    
    void close()
    {
        ksvcCloseHandle(h);
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

        delete p;
        
        return p->ret ? p->ret : p->ipcRet;
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

        delete p;
        
        return p->ret ? p->ret : p->ipcRet;
    }
};

struct IFileSystem : Handle
{
    IFileSystem(u32 handle = 0) : Handle(handle) {}

    u32 createFile(char* path, u64 size = 0)
    {
        HIPCPacket* packet = get_current_packet();
        HIPCCraftedPacket* p = new HIPCCraftedPacket();
        
        p->clear();

        char* path_temp = (char*)packet + 0x80;
        strcpy(path_temp, path);

        p->ipc_cmd(0)->push_arg<u64>(0)->push_arg<u64>(size)->push_arg<u32>(0)->push_static_buffer(path_temp, 0x301)->send_to(h);
        p->free_data();

        delete p;
        
        return p->ret ? p->ret : p->ipcRet;
    }

    u32 deleteFile(char* path)
    {
        HIPCPacket* packet = get_current_packet();
        HIPCCraftedPacket* p = new HIPCCraftedPacket();
        
        p->clear();

        char* path_temp = (char*)packet + 0x80;
        strcpy(path_temp, path);

        p->ipc_cmd(1)->push_static_buffer(path_temp, 0x301)->send_to(h);
        p->free_data();

        delete p;
        
        return p->ret ? p->ret : p->ipcRet;
    }
    
    u32 openFile(char* path, u32 flags, IFile* out)
    {
        HIPCPacket* packet = get_current_packet();
        HIPCCraftedPacket* p = new HIPCCraftedPacket();
        
        p->clear();

        char* path_temp = (char*)packet + 0x80;
        strcpy(path_temp, path);

        p->ipc_cmd(8)->push_arg<u32>(flags)->push_static_buffer(path_temp, 0x301)->send_to(h);
        p->free_data();
        
        *out = IFile(p->get_handle(0));

        delete p;
        
        return p->ret ? p->ret : p->ipcRet;
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
        
        return p->ret ? p->ret : p->ipcRet;
    }
};

#endif // FS_H
