#include <stdbool.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "splitstr.h"

#define ARRAY_SIZE(a) (sizeof(a) / sizeof(*a))

enum TryResult
{
    TRY_OK,
    TRY_ERROR_RETURN_VALUE,
    TRY_ERROR_SPLITN_ERROR,
    TRY_ERROR_ENUM_VAL_OUT_OF_RANGE
};

struct TestResult
{
    enum TryResult outcome;
    struct Strings strings;
    struct SplitStrError error;
};

typedef struct TestResult (*TestFunc)(void);

struct TestResults
{
    size_t count;
    struct TestResult* array;
};

/* acquire space */
static void* acquireSpace(
    const size_t a)
{
    void* memoria = calloc(1, a);
    if (memoria ==  NULL) {
        abort();
    }
    return memoria;
}

static void printStrings(
    const struct Strings* strings)
{
    for (size_t i = 0; i < strings->count; i++) {
        const char* string = strings->array[i];
        int _ = 
            printf(
                "%s\n",
                string
            );
    }
}

static bool streq(
    const char a[],
    const char b[]
) {
    int c = strcmp(a, b);
    return c == 0;
}

static bool validateRetVal(
    const char* knownReturn[],
    const size_t knownReturnCnt,
    struct Strings* ret)
{
    for (size_t i = 0; i < knownReturnCnt; i++) {
        const char* a = knownReturn[i];
        const char* b = ret->array[i];
        if (!streq(a,b)) {
            return false;
        }
    }

    return true;
}

static struct TestResult tryy(
    const char identifier[],
    const char input[],
    const char delim[],
    const size_t n,
    const char* knownReturn[],
    const size_t knownReturnSize)
{
    struct Strings strings = {0};
    struct SplitStrError error = {0};

    int _ =
        printf(
            "%s ... ",
            identifier
        );
    _ = fflush(stdout);

    const enum Result result =
        splitstrn(
            input,
            delim,
            n,
            acquireSpace,
            &strings,
            &error
        );
    enum TryResult tryResult = {0};
    switch (result) {
        case OK:
            if (!validateRetVal(
                knownReturn,
                knownReturnSize,
                &strings
            )) {
                _ = printf(
                    "❌ ERROR (%s)\n",
                    "unknown return value."
                );
                tryResult = TRY_ERROR_RETURN_VALUE;
            } else {
                _ = printf("☑️  OK\n");
                tryResult = TRY_OK;
            }
            break;
        case ERROR:
            _ = printf(
                "❌ ERROR (%d, %s)\n",
                error.kind,
                error.message
            );
            tryResult = TRY_ERROR_SPLITN_ERROR;
            break;
        default:
            assert(false && "Unhandled enum constant.");
            tryResult = TRY_ERROR_ENUM_VAL_OUT_OF_RANGE;
            break;
    }

    return (struct TestResult){
        .outcome=tryResult,
        .strings=strings,
        .error=error
    };
}

static struct TestResult try_splitn_1unicodechardelim()
{
    static const char Identifier[] = "try_splitn_1chardelim0";
    static const char Data[] = "\nMäry häd ä little lämb\nLittle lämb\n";
    static const char Delimiter[] = "ä";
    static const size_t N = 4;
    static const char* KnownReturn[] = {
        "\nM",
        "ry h",
        "d ",
        " little lämb\nLittle lämb\n"
    };

    return tryy(
        Identifier,
        Data,
        Delimiter,
        N,
        KnownReturn,
        N
    );
}



static struct TestResult try_splitn_http_request()
{
    static const char Identifier[] = "try_splitn_http_request";
    static const char Data[] = "POST / HTTP/1/1\r\nHost: 127.0.0.1\r\nContent-Type: application/json\r\n\r\n{\"a\":\"abc\"}";
    static const char Delimiter[] = "\r\n\r\n";
    static const size_t N = 2;
    static const char* KnownReturn[] = {
        "POST / HTTP/1/1\r\nHost: 127.0.0.1\r\nContent-Type: application/json",
        "{\"a\":\"abc\"}"
    };

    return tryy(
        Identifier,
        Data,
        Delimiter,
        N,
        KnownReturn,
        N
    );
}


/* test splitting a string with a single whitespace delimter */
static struct TestResult try_splitn_1whitespacechardelim()
{
    static const char Identifier[] = "try_splitn_delim1whitespace0";
    static const char Data[] = "\nMäry häd ä little lämb\nLittle lämb\n";
    static const char Delimiter[] = " ";
    static const size_t N = 4;
    static const char* KnownReturn[] = {
        "\nMäry",
        "häd",
        "ä",
        "little lämb\nLittle lämb\n"
    };

    return tryy(
        Identifier,
        Data,
        Delimiter,
        N,
        KnownReturn,
        N
    );
}

static struct TestResults try_splitn()
{
    static TestFunc SplitN1LenTests[] = {
        try_splitn_1whitespacechardelim,
        try_splitn_1unicodechardelim,
        try_splitn_http_request
    };
    
    #define TEST_COUNT (ARRAY_SIZE(SplitN1LenTests))

    static struct TestResult Results[
        TEST_COUNT
    ] = {0};

    for (size_t i = 0; i < TEST_COUNT; i++) {
        const TestFunc test = SplitN1LenTests[i];
        const struct TestResult result = test();
        Results[i] = result;
    }

    return (struct TestResults) {
        .count=TEST_COUNT,
        .array=Results
    };
}

int main(void)
{
    int _ =
        printf("executing tests\n");

    struct TestResults results =
        try_splitn();

    for (size_t i = 0; i < results.count; i++) {
        struct TestResult* result =
            &results.array[i];
        switch (result->outcome) {
            case TRY_OK:
            case TRY_ERROR_RETURN_VALUE: /* call to splitn succeeded, strings.array needs a free */
                free(
                    result->strings.array
                );
                break;
            case TRY_ERROR_SPLITN_ERROR:
                break;
            default:
                assert(false && "Enum value out of range.");
                return 1;
        }
    }

    return 0;
}
