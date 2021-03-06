#ifndef __CRT_HPP__
#define __CRT_HPP__
#include <stddef.h>

#ifdef __cplusplus
extern "C"
{
#endif
  void *memcpy(void * dst, const void * src, size_t count);
  void *memset(void *dst, int val, size_t count);
  char *strstr(const char * str1, const char * str2);
#ifdef __cplusplus
};
#endif

#endif
