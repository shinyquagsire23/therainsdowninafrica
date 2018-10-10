#ifndef ARM_CACHE_H
#define ARM_CACHE_H

#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif

void dcache_flush_invalidate(void* addr, size_t size);
void dcache_invalidate(void* addr, size_t size);
void dcache_flush(void* addr, size_t size);
void dcache_zero(void* addr, size_t size);

#ifdef __cplusplus
}
#endif

#endif // ARM_CACHE_H
