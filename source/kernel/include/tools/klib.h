#ifndef KLIB_H
#define KLIB_H

#include <stdarg.h>
#include "comm/types.h"

// 向上对齐到页边界
static inline uint32_t up2 (uint32_t size, uint32_t bound) {
    return (size + bound - 1) & ~(bound - 1);
}

// 向下对齐到界边界
static inline uint32_t down2 (uint32_t size, uint32_t bound) {
    // size & 0xFFFFF000 将低位清空，使其是bound4096的倍数
    return size & ~(bound - 1);
}

int strings_count (char ** start);
char * get_file_name (char * name);

void kernel_strcpy (char * dest, const char * src);
void kernel_strncpy(char * dest, const char * src, int size);
int kernel_strncmp (const char * s1, const char * s2, int size);
int kernel_strlen(const char * str);
void kernel_memcpy (void * dest, void * src, int size);
void kernel_memset(void * dest, uint8_t v, int size);
int kernel_memcmp (void * d1, void * d2, int size);
void kernel_itoa(char * buf, int num, int base);
void kernel_sprintf(char * buffer, const char * fmt, ...);
void kernel_vsprintf(char * buffer, const char * fmt, va_list args);

#ifndef RELEASE
#define ASSERT(condition)    \
    if (!(condition)) panic(__FILE__, __LINE__, __func__, #condition)
void panic (const char * file, int line, const char * func, const char * cond);
#else
#define ASSERT(condition)    ((void)0)
#endif

#endif
