#ifndef _TYPES_H_
#define _TYPES_H_


#define UINT32_MAX ((uint32_t)0-1)

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef unsigned char uint8;
typedef unsigned short uint16;
typedef uint32_t uint32;
typedef unsigned long long uint64;

typedef signed char int8;
typedef short int16;
typedef int int32;
typedef long long int64;

typedef void (*voidFuncPtr)(void);
typedef void (*voidArgumentFuncPtr)(void *);

#define __io volatile
#define __attr_flash __attribute__((section (".USER_FLASH")))
#define __packed __attribute__((__packed__))
#define __deprecated __attribute__((__deprecated__))
#define __weak __attribute__((weak))
#ifndef __always_inline
#define __always_inline __attribute__((always_inline))
#endif
#ifndef __unused
#define __unused __attribute__((unused))
#endif

#ifndef NULL
#define NULL 0
#endif

//#ifndef offsetof
//#define offsetof(type, member) __builtin_offsetof(type, member)
//#endif

#ifdef __cplusplus
}
#endif

#define HIGH   true
#define LOW    false

#endif // _TYPES_H_
