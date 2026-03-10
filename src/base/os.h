#ifndef _MB_OS_H_
#define _MB_OS_H_

#include <stdint.h>

/*
 * Core functions
 */
void     mb_os_init(void);
uint64_t mb_os_now_microseconds(void);

/*
 * File operations
 */

/*
 * Memory functions
 */
void *mb_os_reserve_memory(uint64_t size);
void  mb_os_commit_memory(void *ptr, uint64_t size);
void  mb_os_uncommit_memory(void *ptr, uint64_t size);
void  mb_os_free_memory(void *ptr, uint64_t size);

#endif // _MYQY_OS_H_
