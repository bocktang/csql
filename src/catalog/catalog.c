/*
 * 系统目录管理器实现
 * 参考早期PostgreSQL的系统目录设计
 */
#include "../include/catalog.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
// 系统表结构
typedef struct system_table
{
 char *name;
 void *data;
 int count;
 int capacity;
} system_table_t;
// 系统目录
typedef struct catalog
{
 system_table_t *tables;       // 表目录
 system_table_t *columns;      // 列目录
 system_table_t *indexes;      // 索引目录
 int table_count;
 int column_count;
 int index_count;
} catalog_t;
// 全局目录实例
static catalog_t *sys_catalog = NULL;
// 初始化系统目录
void catalog_init(void)
{
 if (sys_catalog) return;
 sys_catalog = malloc(sizeof(catalog_t));
 if (!sys_catalog) return;
 // 初始化表目录
 sys_catalog->tables = NULL;
 sys_catalog->table_count = 0;
 // 初始化列目录
 sys_catalog->columns = NULL;
 sys_catalog->column_count = 0;
 // 初始化索引目录
 sys_catalog->indexes = NULL;
 sys_catalog->index_count = 0;
 // 创建系统表(这里可以添加系统表)
 // 例如: pg_class, pg_attribute, pg_index
}
// 关闭系统目录
void catalog_shutdown(void)
{
 if (!sys_catalog)
 {
  return;
 }
 // 释放表目录
 for (int i = 0; i < sys_catalog->table_count; i++)
 {
  free(sys_catalog->tables[i].name);
  if (sys_catalog->tables[i].data)
  {
   free(sys_catalog->tables[i].data);
  }
 }
 free(sys_catalog->tables);
 // 释放列目录
 for (int i = 0; i < sys_catalog->column_count; i++)
 {
  free(sys_catalog->columns[i].name);
  if (sys_catalog->columns[i].data)
  {
   free(sys_catalog->columns[i].data);
  }
 }
 free(sys_catalog->columns);
 // 释放索引目录
 for (int i = 0; i < sys_catalog->index_count; i++)
 {
  free(sys_catalog->indexes[i].name);
  if (sys_catalog->indexes[i].data)
  {
   free(sys_catalog->indexes[i].data);
  }
 }
 free(sys_catalog->indexes);
 free(sys_catalog);
 sys_catalog = NULL;
}
// 添加表到系统目录
int catalog_add_table(const char *table_name, int column_count)
{
 if (!sys_catalog || !table_name)
 {
  return -1;
 }
 // 检查表是否已存在
 for (int i = 0; i < sys_catalog->table_count; i++)
 {
  if (strcmp(sys_catalog->tables[i].name, table_name) == 0)
  {
   return -1;  // 表已存在
  }
 }
 // 扩展表目录
 system_table_t *new_tables = realloc(sys_catalog->tables, (sys_catalog->table_count + 1) * sizeof(system_table_t));
 if (!new_tables)
 {
  return -1;
 }
 sys_catalog->tables = new_tables;
 system_table_t *table = &sys_catalog->tables[sys_catalog->table_count];
 table->name = strdup(table_name);
 table->data = malloc(sizeof(int));  // 存储列数
 if (!table->data)
 {
  free(table->name);
  return -1;
 }
 *((int*)table->data) = column_count;
 table->count = 1;
 table->capacity = 1;
 sys_catalog->table_count++;
 return 0;
}
// 从系统目录删除表
int catalog_remove_table(const char *table_name)
{
 if (!sys_catalog || !table_name)
 {
  return -1;
 }
 for (int i = 0; i < sys_catalog->table_count; i++)
 {
  if (strcmp(sys_catalog->tables[i].name, table_name) == 0)
  {
   // 找到表,删除它
   free(sys_catalog->tables[i].name);
   if (sys_catalog->tables[i].data)
   {
    free(sys_catalog->tables[i].data);
   }
   // 移动后续元素
   for (int j = i; j < sys_catalog->table_count - 1; j++)
   {
    sys_catalog->tables[j] = sys_catalog->tables[j + 1];
   }
   sys_catalog->table_count--;
   return 0;
  }
 }
 return -1;  // 表不存在
}
// 检查表是否存在
bool catalog_table_exists(const char *table_name)
{
 if (!sys_catalog || !table_name) return false;
 for (int i = 0; i < sys_catalog->table_count; i++)
 {
  if (strcmp(sys_catalog->tables[i].name, table_name) == 0)
  {
   return true;
  }
 }
 return false;
}
// 添加列定义
int catalog_add_column(const char *table_name, const char *column_name, int data_type, bool is_primary_key)
{
 if (!sys_catalog || !table_name || !column_name)
 {
  return -1;
 }
 // 检查表是否存在
 if (!catalog_table_exists(table_name))
 {
  return -1;
 }
 // 扩展列目录
 system_table_t *new_columns = realloc(sys_catalog->columns, (sys_catalog->column_count + 1) * sizeof(system_table_t));
 if (!new_columns)
 {
  return -1;
 }
 sys_catalog->columns = new_columns;
 system_table_t *column = &sys_catalog->columns[sys_catalog->column_count];
 // 创建列名(格式: 表名.列名)
 char full_name[256];
 snprintf(full_name, sizeof(full_name), "%s.%s", table_name, column_name);
 column->name = strdup(full_name);
 // 存储列信息
 struct
 {
  int data_type;
  bool is_primary_key;
 } column_info = { data_type, is_primary_key };
 column->data = malloc(sizeof(column_info));
 if (!column->data)
 {
  free(column->name);
  return -1;
 }
 memcpy(column->data, &column_info, sizeof(column_info));
 column->count = 1;
 column->capacity = 1;
 sys_catalog->column_count++;
 return 0;
}
// 列出所有表
void catalog_list_tables(void)
{
 if (!sys_catalog)
 {
  printf("系统目录未初始化\n");
  return;
 }
 printf("\n系统目录中的表:\n");
 printf("|---------------------------------------|\n");
 printf("| ID  | 表名            | 列数          |\n");
 printf("|---------------------------------------|\n");
 for (int i = 0; i < sys_catalog->table_count; i++)
 {
  int column_count = *((int*)sys_catalog->tables[i].data);
  printf("| %3d | %-16s | %-10d |\n", i + 1, sys_catalog->tables[i].name, column_count);
 }
 printf("|---------------------------------------|\n");
}
// 列出表结构
void catalog_describe_table(const char *table_name)
{
 if (!sys_catalog || !table_name)
 {
  printf("无效参数\n");
  return;
 }
 printf("\n表结构: %s\n", table_name);
 printf("|------------------------------------------------|\n");
 printf("| ID  | 列名            | 类型       | 主键      |\n");
 printf("|------------------------------------------------|\n");
 int column_index = 1;
 for (int i = 0; i < sys_catalog->column_count; i++)
 {
  char *full_name = sys_catalog->columns[i].name;
  char *dot = strchr(full_name, '.');
  if (dot)
  {
   *dot = '\0';  // 临时修改字符串
   if (strcmp(full_name, table_name) == 0)
   {
    char *col_name = dot + 1;
    // 获取列信息
    struct
    {
     int data_type;
     bool is_primary_key;
    } *column_info = sys_catalog->columns[i].data;
    const char *type_str = "UNKNOWN";
    switch (column_info->data_type)
    {
     case 1: type_str = "INT"; break;
     case 2: type_str = "FLOAT"; break;
     case 3: type_str = "TEXT"; break;
     case 4: type_str = "BOOL"; break;
    }
    printf("| %3d | %-16s | %-10s | %-8s |\n", column_index++, col_name, type_str, column_info->is_primary_key ? "YES" : "NO");
   }
   *dot = '.';  // 恢复字符串
  }
 }
 printf("|------------------------------------------------|\n");
}
