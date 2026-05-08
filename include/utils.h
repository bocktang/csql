/*
 * 工具函数头文件
 */
#ifndef CSQL_UTILS_H
#define CSQL_UTILS_H
#include <stdbool.h>
#include <stdlib.h>
#include <ctype.h>
// 内存管理
void* xmalloc(size_t size);
void* xcalloc(size_t num, size_t size);
void* xrealloc(void *ptr, size_t new_size);
void xfree(void *ptr);
// 字符串管理
char* xstrdup(const char *str);
char** str_split(const char *str, const char *delim, int *count);
void str_array_free(char **array, int count);
char* str_trim(char *str);
bool str_startswith(const char *str, const char *prefix);
bool str_endswith(const char *str, const char *suffix);
void str_tolower(char *str);
void str_toupper(char *str);
#endif // CSQL_UTILS_H
