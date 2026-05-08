/*
 * 系统目录管理器头文件
 */
#ifndef CSQL_CATALOG_H
#define CSQL_CATALOG_H
#include <stdbool.h>
// 系统目录API
void catalog_init(void);
void catalog_shutdown(void);
int catalog_add_table(const char *table_name, int column_count);
int catalog_remove_table(const char *table_name);
bool catalog_table_exists(const char *table_name);
int catalog_add_column(const char *table_name, const char *column_name, int data_type, bool is_primary_key);
void catalog_list_tables(void);
void catalog_describe_table(const char *table_name);
#endif // CSQL_CATALOG_H
