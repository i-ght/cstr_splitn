#ifndef _STR_DIVIDE_INTERNAL_H_
#define _STR_DIVIDE_INTERNAL_H_

#include "splitstrn.h"

enum Result _splitstrn(
    const char input[],
    const char delim[],
    const size_t maxDivisions,
    const AllocFunc resrcAlloc,
    struct Strings* strings,
    struct SplitStrError* error
);
    

#endif