#ifndef _MB_BASE_H_
#define _MB_BASE_H_

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdint.h>

#if defined(__clang__)
#define COMPILER_CLANG 1
#define COMPILER_MSVC  0
#define COMPILER_GCC   0
#if defined(_WIN32)
#define OS_WINDOWS 1
#define OS_APPLE   0
#define OS_LINUX   0
#elif defined(__gnu_linux__) || defined(__linux__)
#define OS_WINDOWS 0
#define OS_APPLE   0
#define OS_LINUX   1
#elif defined(__APPLE__) && defined(__MACH__)
#define OS_WINDOWS 0
#define OS_APPLE   1
#define OS_LINUX   0
#else
#error This compiler/OS combo is not supported.
#endif
#elif defined(_MSC_VER)
#define COMPILER_CLANG 0
#define COMPILER_MSVC  1
#define COMPILER_GCC   0
#if defined(_WIN32)
#define OS_WINDOWS 1
#define OS_APPLE   0
#define OS_LINUX   0
#else
#error This compiler/OS combo is not supported.
#endif
#elif defined(__GNUC__) || defined(__GNUG__)
#define COMPILER_CLANG 0
#define COMPILER_MSVC  0
#define COMPILER_GCC   1
#if defined(__gnu_linux__) || defined(__linux__)
#define OS_WINDOWS 0
#define OS_APPLE   0
#define OS_LINUX   1
#else
#error This compiler/OS combo is not supported.
#endif
#else
#error Compiler not supported
#endif

#ifdef __cplusplus
#define CPP_LANG 1
#else
#define CPP_LANG 0
#endif

#if CPP_LANG
#define AlignOf(T) alignof(T)
#else // CPP_LANG
#if COMPILER_MSVC
# define AlignOf(T) __alignof(T)
#elif COMPILER_CLANG
# define AlignOf(T) __alignof(T)
#elif COMPILER_GCC
# define AlignOf(T) __alignof__(T)
#else
#error AlignOf not defined for this compiler.
#endif
#endif // CPP_LANG

#define TYPEOF(x)      _typeof_(x)

#define KB(x)        (((uint64_t)(x)) << 10)
#define MB(x)        (((uint64_t)(x)) << 20)
#define GB(x)        (((uint64_t)(x)) << 30)
#define ArrayCount(x) (sizeof(x) / sizeof(x[0]))

#define WrapMacroStatement(x) do { x } while(0)

#define IsPowerOfTwo(x)          (((x)&((x)-1))==0)

#define MinOf(x, y)          ((x) < (y) ? (x) : (y))
#define MaxOf(x, y)          ((x) > (y) ? (x) : (y))

// here b must be power_of_two
#define AlignForwardPow2(x,b)  (assert(IsPowerOfTwo(b)),(((x)+(b)-1)&(~((b)-1))))
#define AlignBackwardPow2(x,b) (assert(IsPowerOfTwo(b)),((x)&(~((b)-1))))

#define MACRO_CONCATENATE_IMPL(s1, s2) s1##s2
#define MACRO_CONCATENATE(s1, s2)      MACRO_CONCATENATE_IMPL(s1, s2)

// used for static stuff.
#ifdef __COUNTER__
#define ANONYMOUS_VARIABLE(str) MACRO_CONCATENATE(str, __COUNTER__)
#else
#define ANONYMOUS_VARIABLE(str) MACRO_CONCATENATE(str, __LINE__)
#endif

#define MACRO_SAME_LINE_ANONVAR(x) MACRO_CONCATENATE(x, __LINE__)

// no format this please (nice macros)
#define DeferBlock(begin, end)         for(int MACRO_SAME_LINE_ANONVAR(_i_)=((begin), 0); !MACRO_SAME_LINE_ANONVAR(_i_);++MACRO_SAME_LINE_ANONVAR(_i_), (end))
#define DeferBlockChecked(begin, end) for(int MACRO_SAME_LINE_ANONVAR(_i_)=2*!(begin); (MACRO_SAME_LINE_ANONVAR(_i_)==2?((end), 0):!MACRO_SAME_LINE_ANONVAR(_i_));++MACRO_SAME_LINE_ANONVAR(_i_),(end))

#define FOREACH_ENUMVAL_EXCLUSIVE(enum_name, var) for(enum_name var = (enum_name)0; var < MACRO_CONCATENATE(enum_name, _COUNT); var = (enum_name)((int)var + 1))
#define FOREACH_ENUMVAL FOREACH_ENUMVAL_EXCLUSIVE

#define MemoryCopy(to, from, size) memcpy((void*)(to), (void*)(from), (size));
#define MemorySet(to, value, size) memset((void*)(to), (value), (size));
#define MemoryZero(to, size)       MemorySet((void*)(to), 0, (size));

#define MacroToString(x) #x
#define mb_MarkNotImplemented(x) assert(!(x))

#if !CPP_LANG
typedef uint8_t b8;
#endif // !CPP_LANG

#endif // _MB_BASE_H_

