/*
 * 工具函数实现
 */
#include "../include/utils.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
// 安全分配内存
void* xmalloc(size_t size)
{
 void *ptr = malloc(size);
 if (!ptr)
 {
  fprintf(stderr, "内存分配失败: 请求大小%zu字节\n", size);
  exit(EXIT_FAILURE);
 }
 return ptr;
}
// 安全分配并清零内存
void* xcalloc(size_t num, size_t size)
{
 void *ptr = calloc(num, size);
 if (!ptr)
 {
  fprintf(stderr, "内存分配失败: 请求%zu*%zu字节\n", num, size);
  exit(EXIT_FAILURE);
 }
 return ptr;
}
// 安全重新分配内存
void* xrealloc(void *ptr, size_t new_size)
{
 void *new_ptr = realloc(ptr, new_size);
 if (!new_ptr && new_size > 0)
 {
  fprintf(stderr, "内存重新分配失败: 新大小%zu字节\n", new_size);
  exit(EXIT_FAILURE);
 }
 return new_ptr;
}
// 安全复制字符串
char* xstrdup(const char *str)
{
 if (!str)
 {
  return NULL;
 }
 char *new_str = strdup(str);
 if (!new_str)
 {
  fprintf(stderr, "字符串复制失败\n");
  exit(EXIT_FAILURE);
 }
 return new_str;
}
// 安全释放内存
void xfree(void *ptr)
{
 if (ptr)
 {
  free(ptr);
 }
}
// 字符串分割
char** str_split(const char *str, const char *delim, int *count)
{
 if (!str || !delim || !count)
 {
  return NULL;
 }
 char *str_copy = xstrdup(str);
 char **result = NULL;
 int capacity = 0;
 *count = 0;
 char *token = strtok(str_copy, delim);
 while (token)
 {
  if (*count >= capacity)
  {
   capacity = capacity == 0 ? 8 : capacity * 2;
   result = xrealloc(result, capacity * sizeof(char*));
  }
  result[*count] = xstrdup(token);
  (*count)++;
  token = strtok(NULL, delim);
 }
 xfree(str_copy);
 return result;
}
// 释放字符串数组
void str_array_free(char **array, int count)
{
 if (!array) return;
 for (int i = 0; i < count; i++)
 {
  xfree(array[i]);
 }
 xfree(array);
}
// 去除字符串两端的空白字符
char* str_trim(char *str)
{
 if (!str)
 {
  return NULL;
 }
 // 去除尾部空白
 char *end = str + strlen(str) - 1;
 while (end >= str && (*end == ' ' || *end == '\t' || *end == '\n' || *end == '\r'))
 {
  *end = '\0';
  end--;
 }
 // 去除头部空白
 char *start = str;
 while (*start && (*start == ' ' || *start == '\t' || *start == '\n' || *start == '\r'))
 {
  start++;
 }
 if (start != str)
 {
  memmove(str, start, strlen(start) + 1);
 }
 return str;
}
// 检查字符串是否以指定前缀开头
bool str_startswith(const char *str, const char *prefix)
{
 if (!str || !prefix)
 {
  return false;
 }
 size_t str_len = strlen(str);
 size_t prefix_len = strlen(prefix);
 if (prefix_len > str_len)
 {
  return false;
 }
 return strncmp(str, prefix, prefix_len) == 0;
}
// 检查字符串是否以指定后缀结尾
bool str_endswith(const char *str, const char *suffix)
{
 if (!str || !suffix)
 {
  return false;
 }
 size_t str_len = strlen(str);
 size_t suffix_len = strlen(suffix);
 if (suffix_len > str_len)
 {
  return false;
 }
 return strcmp(str + str_len - suffix_len, suffix) == 0;
}
// 转换为小写
void str_tolower(char *str)
{
 if (!str)
 {
  return;
 }
 for (; *str; str++)
 {
  *str = tolower(*str);
 }
}
// 转换为大写
void str_toupper(char *str)
{
 if (!str)
 {
  return;
 }
 for (; *str; str++)
 {
  *str = toupper(*str);
 }
}
