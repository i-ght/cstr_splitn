#include "splitstrn_internal.h"

enum Result splitstrn(
    const char input[],
    const char delim[],
    const size_t n,
    const AllocFunc resrcAlloc,
    struct Strings* strings,
    struct SplitStrError* error)
{
    return _splitstrn(
        input,
        delim,
        n,
        resrcAlloc,
        strings,
        error
    );
}