#include "../include/csql.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#define MAX_RECORDS 1000
#define MAX_TABLES 10
#define MAX_COLUMNS 10
// 记录结构
typedef struct
{
 char *values[MAX_COLUMNS];
} Record;
// 表结构
typedef struct
{
 char name[32];
 ColumnDef columns[MAX_COLUMNS];
 int column_count;
 int primary_key_count;
 int primary_key_indices[MAX_COLUMNS];
 Record records[MAX_RECORDS];
 int record_count;
} Table;
// 数据库上下文
typedef struct
{
 Table tables[MAX_TABLES];
 int table_count;
 char error_msg[256];
} csql_ctx_impl;
// 查询结果
typedef struct
{
 char **column_names;
 void ***data;
 int row_count;
 int column_count;
} csql_result_impl;
// 全局数据库实例
static csql_ctx_impl *g_db = NULL;
// 查找表
static Table* find_table(const char *table_name)
{
 if (!g_db || !table_name)
 {
  return NULL;
 }
 for (int i = 0; i < g_db->table_count; i++)
 {
  if (strcmp(g_db->tables[i].name, table_name) == 0)
  {
   return &g_db->tables[i];
  }
 }
 return NULL;
}
// 检查主键是否重复
static int check_primary_key_duplicate(Table *table, char *values[])
{
 if (!table || table->primary_key_count == 0)
 {
  return 0;
 }
 for (int i = 0; i < table->record_count; i++)
 {
  int match = 1;
  for (int j = 0; j < table->primary_key_count; j++)
  {
   int pk_idx = table->primary_key_indices[j];
   char *existing_value = table->records[i].values[pk_idx];
   char *new_value = values[pk_idx];
   if (!existing_value || !new_value)
   {
    continue;
   }
   if (strcmp(existing_value, new_value) == 0)
   {
    match = 1;
   }
   else
   {
    match = 0;
    break;
   }
  }
  if (match)
  {
   return 1;  // 找到重复
  }
 }
 return 0;  // 无重复
}
// 解析列定义
static int parse_table_definition(const char *sql, ColumnDef *columns, int *primary_key_count, int *primary_key_indices)
{
 *primary_key_count = 0;
 const char *paren_start = strchr(sql, '(');
 if (!paren_start)
 {
  return 0;
 }
 const char *paren_end = strrchr(paren_start, ')');
 if (!paren_end)
 {
  return 0;
 }
 int len = paren_end - paren_start - 1;
 char *def_str = malloc(len + 2);
 strncpy(def_str, paren_start + 1, len);
 def_str[len] = ',';
 def_str[len + 1] = '\0';
 int col_count = 0;
 int start = 0;
 for (int i = 0; i <= len; i++)
 {
  if (def_str[i] == ',')
  {
   char col_def[256];
   int def_len = i - start;
   strncpy(col_def, def_str + start, def_len);
   col_def[def_len] = '\0';
   char *ptr = col_def;
   while (*ptr == ' ') ptr++;
   // 提取列名
   int name_len = 0;
   char column_name[32] = {0};
   while (ptr[name_len] && ptr[name_len] != ' ')
   {
    column_name[name_len] = ptr[name_len];
    name_len++;
   }
   column_name[name_len] = '\0';
   strcpy(columns[col_count].name, column_name);
   ptr += name_len;
   while (*ptr == ' ')
   {
    ptr++;
   }
   // 提取列类型
   int type_len = 0;
   char column_type[16] = {0};
   while (*ptr && *ptr != ' ' && *ptr != ',')
   {
    if (type_len < 15)
    {
     column_type[type_len++] = toupper(*ptr);
    }
    ptr++;
   }
   column_type[type_len] = '\0';
   strcpy(columns[col_count].type, column_type);
   // 检查主键
   columns[col_count].is_primary_key = 0;
   char upper_def[256];
   strcpy(upper_def, col_def);
   for (int j = 0; upper_def[j]; j++)
   {
    upper_def[j] = toupper(upper_def[j]);
   }
   if (strstr(upper_def, "PRIMARY KEY") != NULL)
   {
    columns[col_count].is_primary_key = 1;
    primary_key_indices[*primary_key_count] = col_count;
    (*primary_key_count)++;
   }
   col_count++;
   start = i + 1;
  }
 }
 free(def_str);
 return col_count;
}
// 解析INSERT值
static int parse_insert_values(const char *sql, char *values[], int max_values)
{
 const char *values_ptr = strcasestr(sql, "VALUES");
 if (!values_ptr)
 {
  return 0;
 }
 values_ptr += 6;
 const char *paren_start = strchr(values_ptr, '(');
 if (!paren_start)
 {
  return 0;
 }
 const char *paren_end = strchr(paren_start, ')');
 if (!paren_end)
 {
  return 0;
 }
 int len = paren_end - paren_start - 1;
 char *values_str = malloc(len + 2);
 strncpy(values_str, paren_start + 1, len);
 values_str[len] = ',';
 values_str[len + 1] = '\0';
 int value_count = 0;
 int in_string = 0;
 int start = 0;
 for (int i = 0; i <= len; i++)
 {
  if (values_str[i] == '\'' && (i == 0 || values_str[i-1] != '\\'))
  {
   in_string = !in_string;
  }
  else if (!in_string && values_str[i] == ',')
  {
   int val_len = i - start;
   char *value = malloc(val_len + 1);
   strncpy(value, values_str + start, val_len);
   value[val_len] = '\0';
   // 去除首尾空格
   char *trimmed = value;
   while (*trimmed == ' ')
   {
    trimmed++;
   }
   char *end = trimmed + strlen(trimmed) - 1;
   while (end > trimmed && (*end == ' ' || *end == '\''))
   {
    end--;
   }
   // 处理字符串引号
   if (trimmed[0] == '\'')
   {
    trimmed++;
    if (end >= trimmed && *end == '\'')
    {
     *end = '\0';
    }
    else
    {
     *(end + 1) = '\0';
    }
   }
   else
   {
    *(end + 1) = '\0';
   }
   // 再次去除空格
   char *final = trimmed;
   while (*final == ' ')
   {
    final++;
   }
   end = final + strlen(final) - 1;
   while (end > final && *end == ' ')
   {
    end--;
   }
   *(end + 1) = '\0';
   values[value_count] = strdup(final);
   value_count++;
   free(value);
   start = i + 1;
  }
 }
 free(values_str);
 return value_count;
}
// 解析WHERE条件
static int parse_where_condition(const char *sql, char *column, char *op, char *value)
{
 const char *where_ptr = strcasestr(sql, "WHERE");
 if (!where_ptr)
 {
  return 0;
 }
 where_ptr += 5;
 while (*where_ptr == ' ')
 {
  where_ptr++;
 }
 int i = 0;
 while (isalnum(where_ptr[i]) || where_ptr[i] == '_')
 {
  column[i] = where_ptr[i];
  i++;
 }
 column[i] = '\0';
 const char *ptr = where_ptr + i;
 while (*ptr == ' ')
 {
  ptr++;
 }
 i = 0;
 if (strncmp(ptr, ">=", 2) == 0)
 {
  strcpy(op, ">=");
  ptr += 2;
 }
 else if (strncmp(ptr, "<=", 2) == 0)
 {
  strcpy(op, "<=");
  ptr += 2;
 }
 else if (strncmp(ptr, "!=", 2) == 0)
 {
  strcpy(op, "!=");
  ptr += 2;
 }
 else if (*ptr == '>' || *ptr == '<' || *ptr == '=')
 {
  op[0] = *ptr;
  op[1] = '\0';
  ptr++;
 }
 else
 {
  return 0;
 }
 while (*ptr == ' ')
 {
  ptr++;
 }
 i = 0;
 if (*ptr == '\'')
 {
  ptr++;
  while (ptr[i] != '\'' && ptr[i] != '\0')
  {
   value[i] = ptr[i];
   i++;
  }
  value[i] = '\0';
 }
 else
 {
  while (isdigit(ptr[i]) || ptr[i] == '.' || ptr[i] == '-')
  {
   value[i] = ptr[i];
   i++;
  }
  value[i] = '\0';
 }
 return 1;
}
// 检查记录是否满足条件
static int check_record_condition(Record *record, Table *table, const char *column, const char *op, const char *value)
{
 int col_idx = -1;
 for (int i = 0; i < table->column_count; i++)
 {
  if (strcmp(table->columns[i].name, column) == 0)
  {
   col_idx = i;
   break;
  }
 }
 if (col_idx < 0)
 {
  return 0;
 }
 char *record_value = record->values[col_idx];
 if (!record_value)
 {
  return 0;
 }
 // 数字比较
 if (strcmp(table->columns[col_idx].type, "INT") == 0 || strcmp(table->columns[col_idx].type, "FLOAT") == 0)
 {
  double rec_val = atof(record_value);
  double cond_val = atof(value);
  if (strcmp(op, "=") == 0)
  {
   return rec_val == cond_val;
  }
  if (strcmp(op, ">") == 0)
  {
   return rec_val > cond_val;
  }
  if (strcmp(op, "<") == 0)
  {
   return rec_val < cond_val;
  }
  if (strcmp(op, ">=") == 0)
  {
   return rec_val >= cond_val;
  }
  if (strcmp(op, "<=") == 0)
  {
   return rec_val <= cond_val;
  }
  if (strcmp(op, "!=") == 0)
  {
   return rec_val != cond_val;
  }
 }
 else
 {
  // 字符串比较
  if (strcmp(op, "=") == 0)
  {
   return strcmp(record_value, value) == 0;
  }
  if (strcmp(op, "!=") == 0)
  {
   return strcmp(record_value, value) != 0;
  }
 }
 return 0;
}
// 解析UPDATE SET子句
static int parse_update_set(const char *sql, char *column, char *value)
{
 const char *set_ptr = strcasestr(sql, "SET");
 if (!set_ptr)
 {
  return 0;
 }
 set_ptr += 3;
 const char *where_ptr = strcasestr(set_ptr, "WHERE");
 int set_len = where_ptr ? (where_ptr - set_ptr) : strlen(set_ptr);
 char set_str[256];
 strncpy(set_str, set_ptr, set_len);
 set_str[set_len] = '\0';
 char *equal_ptr = strchr(set_str, '=');
 if (!equal_ptr)
 {
  return 0;
 }
 *equal_ptr = '\0';
 char *col_start = set_str;
 while (*col_start == ' ')
 {
  col_start++;
 }
 char *col_end = col_start + strlen(col_start) - 1;
 while (col_end > col_start && *col_end == ' ')
 {
  *col_end-- = '\0';
 }
 char *val_start = equal_ptr + 1;
 while (*val_start == ' ')
 {
  val_start++;
 }
 char *val_end = val_start + strlen(val_start) - 1;
 while (val_end > val_start && (*val_end == ' ' || *val_end == '\''))
 {
  val_end--;
 }
 // 去除字符串引号
 if (*val_start == '\'' && *val_end == '\'')
 {
  val_start++;
  *val_end = '\0';
 }
 else
 {
  *(val_end + 1) = '\0';
 }
 strcpy(column, col_start);
 strcpy(value, val_start);
 return 1;
}
// 初始化数据库
csql_ctx_t* csql_init(void)
{
 if (g_db)
 {
  return (csql_ctx_t*)g_db;
 }
 g_db = malloc(sizeof(csql_ctx_impl));
 if (!g_db)
 {
  return NULL;
 }
 g_db->table_count = 0;
 strcpy(g_db->error_msg, "");
 return (csql_ctx_t*)g_db;
}
// 关闭数据库
void csql_close(csql_ctx_t *ctx)
{
 (void)ctx;
 if (!g_db)
 {
  return;
 }
 for (int t = 0; t < g_db->table_count; t++)
 {
  Table *table = &g_db->tables[t];
  for (int i = 0; i < table->record_count; i++)
  {
   for (int j = 0; j < table->column_count; j++)
   {
    if (table->records[i].values[j])
    {
     free(table->records[i].values[j]);
    }
   }
  }
 }
 free(g_db);
 g_db = NULL;
}
// 执行SQL
csql_result_t* csql_execute(csql_ctx_t *ctx, const char *sql)
{
 (void)ctx;
 if (!g_db || !sql)
 {
  return NULL;
 }
 strcpy(g_db->error_msg, "");
 csql_result_impl *result = malloc(sizeof(csql_result_impl));
 if (!result)
 {
  return NULL;
 }
 result->column_names = NULL;
 result->data = NULL;
 result->row_count = 0;
 result->column_count = 0;
 char sql_lower[1024];
 strncpy(sql_lower, sql, sizeof(sql_lower) - 1);
 sql_lower[sizeof(sql_lower) - 1] = '\0';
 for (int i = 0; sql_lower[i]; i++)
 {
  sql_lower[i] = tolower(sql_lower[i]);
 }
 // CREATE TABLE
 if (strstr(sql_lower, "create table") != NULL)
 {
  if (g_db->table_count >= MAX_TABLES)
  {
   strcpy(g_db->error_msg, "表数量已达上限");
   return (csql_result_t*)result;
  }
  const char *table_start = sql + 12;
  while (*table_start == ' ')
  {
   table_start++;
  }
  int name_len = 0;
  char table_name[32] = {0};
  while (table_start[name_len] && table_start[name_len] != ' ' && table_start[name_len] != '(')
  {
   table_name[name_len] = table_start[name_len];
   name_len++;
  }
  table_name[name_len] = '\0';
  Table *table = &g_db->tables[g_db->table_count];
  strcpy(table->name, table_name);
  int primary_key_indices[MAX_COLUMNS];
  table->column_count = parse_table_definition(sql, table->columns, &table->primary_key_count, primary_key_indices);
  for (int i = 0; i < table->primary_key_count; i++)
  {
   table->primary_key_indices[i] = primary_key_indices[i];
  }
  table->record_count = 0;
  g_db->table_count++;
  printf("表创建成功: %s, 列数: %d", table_name, table->column_count);
  if (table->primary_key_count > 0)
  {
   printf(", 主键: ");
   for (int i = 0; i < table->primary_key_count; i++)
   {
    int idx = table->primary_key_indices[i];
    printf("%s", table->columns[idx].name);
    if (i < table->primary_key_count - 1)
    {
     printf(", ");
    }
   }
  }
  printf("\n");
  strcpy(g_db->error_msg, "CREATE TABLE 执行成功");
 }
 // INSERT INTO
 else if (strstr(sql_lower, "insert into") != NULL)
 {
  const char *into_ptr = strcasestr(sql, "INTO");
  if (!into_ptr)
  {
   strcpy(g_db->error_msg, "INSERT 语法错误");
   return (csql_result_t*)result;
  }
  into_ptr += 4;
  while (*into_ptr == ' ')
  {
   into_ptr++;
  }
  int name_len = 0;
  char table_name[32] = {0};
  while (into_ptr[name_len] && into_ptr[name_len] != ' ')
  {
   table_name[name_len] = into_ptr[name_len];
   name_len++;
  }
  table_name[name_len] = '\0';
  Table *table = find_table(table_name);
  if (!table)
  {
   sprintf(g_db->error_msg, "表'%s'不存在", table_name);
   return (csql_result_t*)result;
  }
  if (table->record_count >= MAX_RECORDS)
  {
   strcpy(g_db->error_msg, "记录数已达上限");
   return (csql_result_t*)result;
  }
  char *values[MAX_COLUMNS] = {NULL};
  int value_count = parse_insert_values(sql, values, table->column_count);
  if (value_count != table->column_count)
  {
   strcpy(g_db->error_msg, "值的数量与列数不匹配");
   for (int i = 0; i < value_count; i++)
   {
    if (values[i])
    {
     free(values[i]);
    }
   }
   return (csql_result_t*)result;
  }
  // 检查主键是否重复
  if (table->primary_key_count > 0)
  {
   if (check_primary_key_duplicate(table, values))
   {
    char error_msg[256];
    if (table->primary_key_count == 1)
    {
     int pk_idx = table->primary_key_indices[0];
     snprintf
     (
      error_msg, sizeof(error_msg),
      "duplicate key value violates unique constraint \"%s_pkey\"\n"
      "详细信息: 键值(%s)=(%s)已存在",
      table_name,
      table->columns[pk_idx].name,
      values[pk_idx] ? values[pk_idx] : "NULL"
     );
    }
    else
    {
     char key_values[256] = "";
     for (int j = 0; j < table->primary_key_count; j++)
     {
      int pk_idx = table->primary_key_indices[j];
      char temp[64];
      snprintf
      (
       temp, sizeof(temp), "%s=%s",
       table->columns[pk_idx].name,
       values[pk_idx] ? values[pk_idx] : "NULL"
      );
      if (j > 0)
      {
       strcat(key_values, ", ");
      }
      strcat(key_values, temp);
     }
     snprintf
     (
      error_msg, sizeof(error_msg),
      "duplicate key value violates unique constraint \"%s_pkey\"\n"
      "详细信息: 键值(%s)已存在",
      table_name, key_values
     );
    }
    strcpy(g_db->error_msg, error_msg);
    printf("?主键冲突: %s\n", error_msg);
    for (int i = 0; i < value_count; i++)
    {
     if (values[i])
     {
      free(values[i]);
     }
    }
    return (csql_result_t*)result;
   }
  }
  Record *record = &table->records[table->record_count];
  for (int i = 0; i < table->column_count; i++)
  {
   record->values[i] = strdup(values[i] ? values[i] : "NULL");
  }
  table->record_count++;
  for (int i = 0; i < value_count; i++)
  {
   if (values[i])
   {
    free(values[i]);
   }
  }
  printf("?插入记录到表%s,总记录数: %d\n", table_name, table->record_count);
  strcpy(g_db->error_msg, "INSERT 执行成功");
 }
 // UPDATE
 else if (strstr(sql_lower, "update") != NULL)
 {
  const char *table_start = sql + 6;
  while (*table_start == ' ')
  {
   table_start++;
  }
  int name_len = 0;
  char table_name[32] = {0};
  while (table_start[name_len] && table_start[name_len] != ' ')
  {
   table_name[name_len] = table_start[name_len];
   name_len++;
  }
  table_name[name_len] = '\0';
  Table *table = find_table(table_name);
  if (!table)
  {
   sprintf(g_db->error_msg, "表'%s'不存在", table_name);
   return (csql_result_t*)result;
  }
  char set_column[32] = "", set_value[256] = "";
  if (!parse_update_set(sql, set_column, set_value))
  {
   strcpy(g_db->error_msg, "UPDATE语法错误: 缺少SET子句");
   return (csql_result_t*)result;
  }
  char where_column[32] = "", where_op[3] = "", where_value[256] = "";
  int has_where = parse_where_condition(sql, where_column, where_op, where_value);
  int updated_count = 0;
  for (int i = 0; i < table->record_count; i++)
  {
   if (has_where && !check_record_condition(&table->records[i], table, where_column, where_op, where_value))
   {
    continue;
   }
   // 查找要更新的列
   int col_idx = -1;
   for (int j = 0; j < table->column_count; j++)
   {
    if (strcmp(table->columns[j].name, set_column) == 0)
    {
     col_idx = j;
     break;
    }
   }
   if (col_idx >= 0)
   {
    if (table->records[i].values[col_idx])
    {
     free(table->records[i].values[col_idx]);
    }
    table->records[i].values[col_idx] = strdup(set_value);
    updated_count++;
   }
  }
  printf("? UPDATE: 更新了表%s中的%d条记录\n", table_name, updated_count);
  if (has_where)
  {
   printf("   条件: WHERE %s %s %s\n", where_column, where_op, where_value);
  }
  printf("   设置: %s = '%s'\n", set_column, set_value);
  sprintf(g_db->error_msg, "UPDATE执行成功,更新了%d条记录", updated_count);
 }
 // DELETE
 else if (strstr(sql_lower, "delete from") != NULL)
 {
  const char *from_ptr = strcasestr(sql, "FROM");
  if (!from_ptr)
  {
   strcpy(g_db->error_msg, "DELETE语法错误");
   return (csql_result_t*)result;
  }
  from_ptr += 4;
  while (*from_ptr == ' ')
  {
   from_ptr++;
  }
  int name_len = 0;
  char table_name[32] = {0};
  while (from_ptr[name_len] && from_ptr[name_len] != ' ' && from_ptr[name_len] != ';')
  {
   table_name[name_len] = from_ptr[name_len];
   name_len++;
  }
  table_name[name_len] = '\0';
  Table *table = find_table(table_name);
  if (!table)
  {
   sprintf(g_db->error_msg, "表'%s'不存在", table_name);
   return (csql_result_t*)result;
  }
  char where_column[32] = "", where_op[3] = "", where_value[256] = "";
  int has_where = parse_where_condition(sql, where_column, where_op, where_value);
  int deleted_count = 0;
  int to_delete[MAX_RECORDS] = {0};
  for (int i = 0; i < table->record_count; i++)
  {
   if (!has_where || check_record_condition(&table->records[i], table, where_column, where_op, where_value))
   {
    to_delete[i] = 1;
    deleted_count++;
   }
  }
  if (deleted_count > 0)
  {
   Record new_records[MAX_RECORDS];
   int new_count = 0;
   for (int i = 0; i < table->record_count; i++)
   {
    if (!to_delete[i])
    {
     new_records[new_count] = table->records[i];
     new_count++;
    }
    else
    {
     for (int j = 0; j < table->column_count; j++)
     {
      if (table->records[i].values[j])
      {
       free(table->records[i].values[j]);
      }
     }
    }
   }
   table->record_count = new_count;
   for (int i = 0; i < new_count; i++)
   {
    table->records[i] = new_records[i];
   }
   printf("? DELETE: 从表%s删除了%d条记录\n", table_name, deleted_count);
   if (has_where)
   {
    printf("   条件: WHERE %s %s %s\n", where_column, where_op, where_value);
   }
   sprintf(g_db->error_msg, "DELETE执行成功,删除了%d条记录", deleted_count);
  }
  else
  {
   strcpy(g_db->error_msg, "DELETE执行成功,但没有记录被删除");
  }
 }
 // SELECT
 else if (strstr(sql_lower, "select") != NULL)
 {
  const char *from_ptr = strcasestr(sql, "FROM");
  if (!from_ptr)
  {
   strcpy(g_db->error_msg, "SELECT语句缺少FROM子句");
   return (csql_result_t*)result;
  }
  from_ptr += 4;
  while (*from_ptr == ' ')
  {
   from_ptr++;
  }
  int name_len = 0;
  char table_name[32] = {0};
  while (from_ptr[name_len] && from_ptr[name_len] != ' ' && from_ptr[name_len] != ';')
  {
   table_name[name_len] = from_ptr[name_len];
   name_len++;
  }
  table_name[name_len] = '\0';
  Table *table = find_table(table_name);
  if (!table)
  {
   sprintf(g_db->error_msg, "表'%s'不存在", table_name);
   return (csql_result_t*)result;
  }
  char where_column[32] = "", where_op[3] = "", where_value[256] = "";
  int has_where = parse_where_condition(sql, where_column, where_op, where_value);
  int match_count = 0;
  for (int i = 0; i < table->record_count; i++)
  {
   if (!has_where || check_record_condition(&table->records[i], table, where_column, where_op, where_value))
   {
    match_count++;
   }
  }
  if (match_count > 0)
  {
   result->column_count = table->column_count;
   result->column_names = malloc(table->column_count * sizeof(char*));
   for (int i = 0; i < table->column_count; i++)
   {
    result->column_names[i] = strdup(table->columns[i].name);
   }
   result->row_count = match_count;
   result->data = malloc(match_count * sizeof(void**));
   int data_idx = 0;
   for (int i = 0; i < table->record_count; i++)
   {
    if (!has_where || check_record_condition(&table->records[i], table, where_column, where_op, where_value))
    {
     result->data[data_idx] = malloc(table->column_count * sizeof(void*));
     for (int j = 0; j < table->column_count; j++)
     {
      result->data[data_idx][j] = strdup(table->records[i].values[j] ? table->records[i].values[j] : "NULL");
     }
     data_idx++;
    }
   }
   printf("? 查询: 返回%d/%d行", match_count, table->record_count);
   if (has_where)
   {
    printf(" (WHERE %s %s %s)", where_column, where_op, where_value);
   }
   printf("\n");
   strcpy(g_db->error_msg, "SELECT执行成功");
  }
  else
  {
   strcpy(g_db->error_msg, "SELECT执行成功,但无匹配记录");
  }
 }
 else
 {
  strcpy(g_db->error_msg, "不支持的SQL语句");
 }
 return (csql_result_t*)result;
}
// 描述表结构
void csql_describe_table(const char *table_name)
{
 if (!g_db)
 {
  printf("数据库未初始化\n");
  return;
 }
 Table *table = find_table(table_name);
 if (!table)
 {
  printf("表%s不存在\n", table_name);
  return;
 }
 printf("\n表结构: %s\n", table_name);
 printf("|--------------------------------|\n");
 printf("| 列名 | 类型       | 主键       |\n");
 printf("|--------------------------------|\n");
 for (int i = 0; i < table->column_count; i++)
 {
  printf("| %-5s| %-10s | %-10s |\n", table->columns[i].name, table->columns[i].type, table->columns[i].is_primary_key ? "YES" : "NO");
 }
 printf("|--------------------------------|\n");
}
// 释放结果
void csql_free_result(csql_result_t *result)
{
 if (!result)
 {
  return;
 }
 csql_result_impl *impl = (csql_result_impl*)result;
 if (impl->column_names)
 {
  for (int i = 0; i < impl->column_count; i++)
  {
   free(impl->column_names[i]);
  }
  free(impl->column_names);
 }
 if (impl->data)
 {
  for (int i = 0; i < impl->row_count; i++)
  {
   for (int j = 0; j < impl->column_count; j++)
   {
    free(impl->data[i][j]);
   }
   free(impl->data[i]);
  }
  free(impl->data);
  }
 free(impl);
}
int csql_result_row_count(csql_result_t *result)
{
 if (!result) return 0;
 return ((csql_result_impl*)result)->row_count;
}
int csql_result_column_count(csql_result_t *result)
{
 if (!result)
 {
  return 0;
 }
 return ((csql_result_impl*)result)->column_count;
}
const char* csql_result_column_name(csql_result_t *result, int col)
{
 if (!result || col < 0)
 {
  return NULL;
 }
 csql_result_impl *impl = (csql_result_impl*)result;
 if (col >= impl->column_count || !impl->column_names)
 {
  return NULL;
 }
 return impl->column_names[col];
}
const char* csql_result_cell(csql_result_t *result, int row, int col)
{
 if (!result || row < 0 || col < 0)
 {
  return NULL;
 }
 csql_result_impl *impl = (csql_result_impl*)result;
 if (row >= impl->row_count || col >= impl->column_count || !impl->data)
 {
  return NULL;
 }
 return (const char*)impl->data[row][col];
}
const char* csql_error_message(csql_ctx_t *ctx)
{
 (void)ctx;
 if (!g_db) return "数据库未初始化";
 return g_db->error_msg;
}
int csql_affected_rows(csql_ctx_t *ctx)
{
 return 0;
}
