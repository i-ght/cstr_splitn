#ifndef _STR_DIVIDE_PUBLIC_H_
#define _STR_DIVIDE_PUBLIC_H_

#include "mem_func_ptrs.h"

enum Result
{
    OK,
    ERROR
};

struct Strings
{
    size_t count;
    const char** array;
};

enum SplitStrErrorKind
{
    SPLIT_STR_ERROR_INPUT_DOES_NOT_CONTAIN_DELIMTER,
    /* while splitting the string up, expected to find a delim but did not. unexpected end of input */
    SPLIT_STR_ERROR_SUBSTR_DELIM_NOT_FOUND,
    SPLIT_STR_ERROR_MEM_SPACE_ALLOC_LARGER_THAN_1GB
};

struct SplitStrError
{
    enum SplitStrErrorKind kind;
    const char* message;
};

enum Result splitstrn(
    const char input[],
    const char delim[],
    const size_t maxDivisions,
    const AllocFunc resrcAlloc,
    struct Strings* strings,
    struct SplitStrError* error
);

#endif