#ifndef REGMAP_H
#define REGMAP_H

#include "arm/threading.h"
#include "arm/cache.h"

#include <cstring>

#define ENTRIES_PER_BUCKET 0x20

template <class A, class B>
struct RegSlot
{
    bool taken;
    A a;
    B b;
};

template <class A, class B, size_t C>
struct RegMap
{
    u64 access_mutex;
    RegSlot<A, B> vals[C];
    
    uint32_t hash_idx(A a)
    {
        u8* raw = (u8*)&a;
        
        // FNV
        uint32_t ret = 2166136261;
        for (int i = 0; i < sizeof(a); i++)
        {
            ret *= 16777619;
            ret ^= raw[i];
        }
        
        return ret % (C / ENTRIES_PER_BUCKET); // 32 entries per bucket
    }
    
    B get(A a)
    {
        mutex_lock(&access_mutex);
        
        size_t start = hash_idx(a);
        dcache_invalidate(&vals[start], sizeof(vals[start]) * ENTRIES_PER_BUCKET);

        for (size_t i = start * ENTRIES_PER_BUCKET; i < (start + 1) * ENTRIES_PER_BUCKET; i++)
        {
            if (!memcmp(&vals[i].a, &a, sizeof(a)) && vals[i].taken)
            {
                B ret = vals[i].b;
                mutex_unlock(&access_mutex);

                return ret;
            }
        }
        
        mutex_unlock(&access_mutex);
        return NULL;
    }
    
    void set(A a, B b)
    {
        mutex_lock(&access_mutex);

        size_t start = hash_idx(a);

        dcache_invalidate(&vals[start], sizeof(vals[start]) * ENTRIES_PER_BUCKET);
        
        for (size_t i = start * ENTRIES_PER_BUCKET; i < (start + 1) * ENTRIES_PER_BUCKET; i++)
        {
            if (!vals[i].taken)
            {
                vals[i].a = a;
                vals[i].b = b;
                vals[i].taken = true;
                dcache_flush(&vals[i], sizeof(vals[i]));

                break;
            }
        }

        mutex_unlock(&access_mutex);
    }
    
    void clear(A a)
    {
        mutex_lock(&access_mutex);
        
        size_t start = hash_idx(a);
        dcache_invalidate(&vals[start], sizeof(vals[start]) * ENTRIES_PER_BUCKET);
        
        for (size_t i = start * ENTRIES_PER_BUCKET; i < (start + 1) * ENTRIES_PER_BUCKET; i++)
        {
            if (!memcmp(&vals[i].a, &a, sizeof(a)) && vals[i].taken)
            {
                vals[i].taken = false;
                dcache_flush(&vals[i], sizeof(vals[i]));
                
                break;
            }
        }

        mutex_unlock(&access_mutex);
    }
};

#endif // REGMAP_H
