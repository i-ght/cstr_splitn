#ifndef _MEM_FUNC_PTRS_H_
#define _MEM_FUNC_PTRS_H_

#include <stdlib.h>

typedef void* (*AllocFunc)(
    const size_t a
);

typedef void* (*ReallocFunc)(
    void* memory,
    const size_t a
);

typedef void (*FreeFunc)(
    void* memory
);

#endif