#ifndef TRY_H_
#define TRY_H_

#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>

#pragma region Main_Interface

/*------------------------------------------------------------------------------+
 |                                  Interface                                   |
 +------------------------------------------------------------------------------*/

#pragma region Control_Flow

/*-------------------------------------------------------> Control Flow <-------*/

#define TRY { jmp_buf _tryCurrJmp; _tryCurrState = &(_TryState){ &_tryCurrJmp, _tryCurrState }; if(!setjmp(*_tryCurrState->jmp))

#define HANDLE_WHEN else if

#define OTHERWISE else

#define RETHROW longjmp(*_tryCurrState->jmp, !0);

#define END_ATTEMPT }

#pragma endregion Control_Flow

#pragma region Exceptions

/*---------------------------------------------------------> Exceptions <-------*/

#define EXCEPTION_DEFINITION(exception) Exception exception = { .branch = NULL, .name = #exception, .cause = NULL, .data = NULL,

#define END_EXCEPTION_DEFINITION };

///Holds current exception.
#define EXCEPTION ((const Exception*)_tryCurrException)

//Checks if the current exception is from specified branch
#define EXCEPTION_IS(exception) (_tryExceptionIs(&exception))

//Check if the current exception equals to specified exception
#define EXCEPTION_EQUALS(exception) (EXCEPTION == &exception)

///Raises a new exception.
#define RAISE(exception)\ 
{\ 
    jmp_buf* jmp;\
    if(!_tryCurrState)\
        UNHANDLED_EXCEPTION\
    jmp = _tryCurrState->jmp;\
    _tryCurrState = _tryCurrState->prev;\
    _tryCurrException = &(exception); longjmp(*jmp, !0);\
}

#pragma endregion Exceptions

#pragma endregion Main_Interface

#pragma region 

/*------------------------------------------------------------------------------+
 |                            ---> Override Me <---                             |
 +------------------------------------------------------------------------------*/

#ifndef UNHANDLED_EXCEPTION
#define UNHANDLED_EXCEPTION \
    _try_eprintf(\
        "Unhandled %s%s%s",\
        EXCEPTION->name? EXCEPTION->name : "exception",\
        EXCEPTION->cause? ": " : ".",\
        EXCEPTION->cause? EXCEPTION->cause : ""\
    ); exit(-1);
#endif //UNHANDLED_EXCEPTION

#pragma region Implementation

/*------------------------------------------------------------------------------+
 |                                Implementation                                |
 +------------------------------------------------------------------------------*/

#pragma region Control_Flow

/*-------------------------------------------------------> Control Flow <-------*/

///Holds try-block state
typedef struct _TryState {
    jmp_buf* jmp;
    ///Previous state
    struct _TryState* prev;
} _TryState;

///Current try-block state
_TryState* _tryCurrState = NULL;

#pragma endregion Control_Flow

#pragma region Exceptions

/*---------------------------------------------------------> Exceptions <-------*/

typedef struct _Exception {
    struct _Exception* const branch;
    char* name;
    char* cause;
    void* data;
} Exception;

Exception* _tryCurrException = NULL;

//compatibility
#if (!defined(__STDC_VERSION__) || __STDC_VERSION__ < 199901L) && !defined(inline)
#define inline
#endif

///Checks if exception is from specified exception branch
static inline int _tryExceptionIs(const Exception* exception)
{
    Exception* current = (Exception*)exception;
    while(current != NULL)
    {
        if(current == _tryCurrException)
            return !0;
        current = (Exception*)current->branch;
    }
    return 0;
}

#pragma endregion Exceptions

#ifndef _try_eprintf
#ifdef __STDC_VERSION__
#if __STDC_VERSION__ >= 201112L  //`__VA_ARGS__` support && safer
#define _try_eprintf(format, ...) fprintf_s(stderr, format, __VA_ARGS__)
#elif __STDC_VERSION__ >= 199901L //`__VA_ARGS__` support
#define _try_eprintf(format, ...) fprintf(stderr, format, __VA_ARGS__)
#else //older than C99
#define _try_eprintf printf
#endif
#else //no `__STDC_VERSION__`
#define _try_eprintf printf
#endif
#endif //_try_eprintf

#pragma endregion Implementation

#endif //TRY_H_