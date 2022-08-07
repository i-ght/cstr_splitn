#include <assert.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>

#include "splitstrn_internal.h"


enum {ONE_GIGABYTE=1073741824};

static struct SplitStrError splitstrErr(
    const enum SplitStrErrorKind errType)
{
    static const struct SplitStrError noDelimErr = {
        .kind=SPLIT_STR_ERROR_INPUT_DOES_NOT_CONTAIN_DELIMTER,
        .message="input did not contain delimiter."
    };
    static const struct SplitStrError strDupStrTooLarge = {
        .kind=SPLIT_STR_ERROR_MEM_SPACE_ALLOC_LARGER_THAN_1GB,
        .message="string too large (> 1GB) to allocate."
    };
    static const struct SplitStrError missingNo = {
        .kind=-1,
        .message="!?"
    };
    switch (errType) {
        case SPLIT_STR_ERROR_INPUT_DOES_NOT_CONTAIN_DELIMTER:
            return noDelimErr;
        case SPLIT_STR_ERROR_MEM_SPACE_ALLOC_LARGER_THAN_1GB:
            return strDupStrTooLarge;
        default:
            return missingNo;
    }
}

static enum Result setsplitstrErr(
    const enum SplitStrErrorKind type,
    struct SplitStrError* error)
{
    if (error != NULL) {
        *error = splitstrErr(type);
    }
    return ERROR;
}

static size_t strCount(
    const char input[],
    const char value[],
    const size_t valLen)
{
    size_t count = 0;
    const char* tmp = input;
    while ((tmp = strstr(tmp, value)) != NULL) {
        count++;
        tmp += valLen;
    }

    return count;
}


static bool amLastIteration(
    const size_t index,
    const size_t count)
{
    return index == count - 1;
}

static const char* substrTerminator(
    const size_t index,
    const size_t count,
    const char delim[],
    bool* amTheEnd)
{
    *amTheEnd = false;
    if (amLastIteration(index, count)) {
        *amTheEnd = true;
        return ""; /*last iteration will not have delimiter string at the end of it. */    
    } else {
        return delim;
    }
}

static void advancePosition(
    const bool amTheEnd,
    const char** current,
    const char** previous,
    const char delim[],
    const size_t dLen)
{
    if (amTheEnd) {
        *current = *previous;
    }
    else {
        *current = strstr(
            *current,
            delim
        );
    }

    *previous = *current;
    
    /* advance beyond the delimiter*/
    *current += dLen;
}

/* length of string up to next defined terminator */
static size_t strLen(
    const char input[],
    const char terminator[])
{
    enum {STR_LEN_INVALID_INPUT=SIZE_MAX};
    static const char Emptiness[] = "";

    /* if terminator is an empty string then strlen */
    if (memcmp(terminator, Emptiness, 1) == 0) {
        return strlen(input);
    }
    
    /* find the next occurence of the terminator */
    const char* index =
        strstr(
            input,
            terminator
        );
    
    assert(index && "terminator not found in string.");

    if (index == NULL) {
        return STR_LEN_INVALID_INPUT;
    }
    const size_t ret = index - input;
    return ret;
}

static size_t calcSpaceRequired(
    const size_t count,
    const size_t lengths[])
{
    enum {NULL_TERM=1};
    
    size_t spaceRequired = 0;
    
    /*short left side, memory space container of array indices */
    const size_t n = sizeof(char*) * count; /* number of values in the array */
    spaceRequired += n;

    /* calc total characters required to divide the string into (a) new string(s) */
    for (size_t i = 0; i < count; i++) {
        const size_t length = lengths[i];
        spaceRequired += length + NULL_TERM;
    }
    return spaceRequired;
}

static enum Result splitstrOK(
    struct Strings* strings,
    const size_t divisons,
    const char** array)
{
    strings->count = divisons;
    strings->array = array;

    return OK;
}

/* iterates thru the input string to find each substring before
   or after a delimiter occurence. places occurence memory address and string lengths into
   indices and lengths arrays arguments. */
static enum Result storeSubStrMemoryIndices(
    const char input[],
    const char delim[],
    const size_t delimLen,
    const size_t divisionCount,
    const size_t stringCount,
    size_t lengths[],
    const char* indices[],
    struct SplitStrError* error)
{
    /* when will now be then?
       soon */
    /* current memory index position of the input string */
    const char* now = input;
    /* current position including previous delimiter (if any)*/
    const char* then = input;
    
    for (size_t i = 0; i < divisionCount; i++) {
        const char* substr = now;
        /* 
            last substring will not contain a delimiter at the end so the 
            length needs to be calculated using null term instead of delim. 
        */
        bool amTheEnd = false;
        const char* terminator =
            substrTerminator(
                i,
                divisionCount,
                delim,
                &amTheEnd
            ); /* "" or delim */

        const size_t substrLen =
            strLen(
                substr,
                terminator
            );
        if (substrLen == SIZE_MAX)  {
            return setsplitstrErr(
                SPLIT_STR_ERROR_SUBSTR_DELIM_NOT_FOUND,
                error
            );
        }

        lengths[i] = substrLen;
        indices[i] = substr;

        /*advance now beyond delimiter,
          advance then to start of previous delimiter */
        advancePosition(
            amTheEnd,
            &now,
            &then,
            delim,
            delimLen
        );

        assert(now != NULL && "now");
    }

    return OK;
}

/* create duplicates of each substring found in-between the delimiter occurrences. */
static enum Result dupSubStrs(
    /* amount of new strings to be allocated */
    const size_t divisions,
    /* amount of strings before or after delimiter occurences contained in input */
    const size_t stringCount,
    /* delimiter string that serves as a terminator */
    const char delim[],
    /* array to store pointers to the starts of the contents of the divided strings. */
    const char* indices[],
    /* lengths of strings to be divided*/
    const size_t lengths[],
    /* allocated space that will contain all the array indices and strings contents */
    char** array,
    /* result */
    struct Strings* strings,
    /* optional error information */
    struct SplitStrError* error)
{

    enum {NULL_TERM=1};

    char** dataStart = &array[divisions];
    char* memoryIndex = (char*)dataStart;

    for (size_t i = 0; i < divisions; i++) {
        const char* src = indices[i];
        const size_t len = lengths[i];
        const void* _ =
            memmove(
                memoryIndex,
                src,
                len
            );
    
        array[i] = memoryIndex;
        memoryIndex += (len + NULL_TERM);
    }

    return splitstrOK(
        strings,
        divisions,
        (const char**)/*read only now by decree*/array
    );
}

/*🐉🐍🌈*/
enum Result _splitstrn(
    const char input[],
    const char delim[],
    const size_t n,
    const AllocFunc resrcAlloc,
    struct Strings* strings,
    struct SplitStrError* error)
{
    const size_t delimiterLen = strlen(delim);
    assert(delimiterLen > 0);

    /*the amount of occurences of the delimiter string */
    const size_t delimCnt =
        strCount(
            input,
            delim,
            delimiterLen
        );

    if (delimCnt == 0) {
        return setsplitstrErr(
            SPLIT_STR_ERROR_INPUT_DOES_NOT_CONTAIN_DELIMTER,
            error
        );
    }

    #define MAX_DIVISIONS 255

    /* array that will contain pointers to the strings before or after delimiter occurences.*/
    static const char* indices[MAX_DIVISIONS] = {0};
    /* length of aformentioned strings up to next delimiter or null term (if last string). */
    static size_t lengths[MAX_DIVISIONS] = {0};

    /* the amount of strings before or after delimiter occurences. */
    const size_t stringCount = delimCnt + 1;

    /* value that represents the amount of new individiual strings to be created */
    const size_t divisions =
        stringCount > n
            ? n
            : stringCount;

    /* get pointers to the beginning of each string before or after delimter occurences.  */
    if (storeSubStrMemoryIndices(
        input,
        delim,
        delimiterLen,
        divisions,
        stringCount,
        lengths,
        indices,
        error
    ) == ERROR) {
        return ERROR;
    };

    /* space required for the containing array and each new null terminated string. */
    const size_t spaceRequired =
        calcSpaceRequired(
            divisions,
            lengths
        );

    if (spaceRequired > ONE_GIGABYTE) {
        return setsplitstrErr(
            SPLIT_STR_ERROR_MEM_SPACE_ALLOC_LARGER_THAN_1GB,
            error
        );
    }

    /* 
       short left side is pointers up to stringCount of char*,
       long right side is pointer to start of divided now null terminated strings
    */
    enum {BYTE=0};
    char** array = resrcAlloc(spaceRequired);

    const void* _ =
        memset(
            array,
            BYTE,
            spaceRequired
        );

    /* create duplicates of strings before or after delimiter occurences */
    return dupSubStrs(
        divisions,
        stringCount,
        delim,
        indices,
        lengths,
        array,
        strings,
        error
    );
}