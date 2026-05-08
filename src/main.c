#include "../include/csql.h"
#include <stdio.h>
#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <string.h>
#include <ctype.h>
static void print_help(void)
{
 printf("\n");
 printf("csql命令帮助:\n");
 printf("================\n");
 printf("  元命令(以点开头):\n");
 printf("    .help或\\?      - 显示此帮助信息\n");
 printf("    .exit或\\q      - 退出程序\n");
 printf("    .tables         - 显示所有表\n");
 printf("    .version        - 显示版本信息\n");
 printf("    .clear或\\c     - 清屏\n");
 printf("\n");
 printf("  SQL命令:\n");
 printf("    CREATE TABLE <table> (<columns>)  - 创建表\n");
 printf("    INSERT INTO <table> VALUES (...)  - 插入数据\n");
 printf("    SELECT ... FROM <table> [WHERE]   - 查询数据\n");
 printf("\n");
 printf("  示例:\n");
 printf("    CREATE TABLE users (id INT PRIMARY KEY, name TEXT, age INT);\n");
 printf("    INSERT INTO users VALUES (1, 'Alice', 30);\n");
 printf("    SELECT * FROM users WHERE age > 25;\n");
 printf("\n");
 printf("  特殊命令:\n");
 printf("    \\d 表名         - 显示表结构\n");
 printf("\n");
 printf("  注意:\n");
 printf("    - 语句以分号(;)结束\n");
 printf("    - 字符串用单引号(')包围\n");
 printf("    - 支持中文和英文\n");
 printf("\n");
}
static int handle_meta_command(const char *command)
{
 if (!command || command[0] == '\0')
 {
  return 0;
 }
 char cmd[256];
 strncpy(cmd, command, sizeof(cmd) - 1);
 cmd[sizeof(cmd) - 1] = '\0';
 for (int i = 0; cmd[i]; i++)
 {
  cmd[i] = tolower(cmd[i]);
 }
 if (cmd[0] == '.')
 {
  if (strcmp(cmd, ".help") == 0 || strcmp(cmd, ".?") == 0)
  {
   print_help();
   return 1;
  }
  else if (strcmp(cmd, ".exit") == 0 || strcmp(cmd, ".quit") == 0 || strcmp(cmd, ".q") == 0)
  {
   return 2;
  }
  else if (strcmp(cmd, ".tables") == 0)
  {
   // 这里应该从数据库中获取表列表
   printf("\n表列表:\n");
   printf("|-------------------------------------|\n");
   printf("| ID  | 表名            | 记录数      |\n");
   printf("|-------------------------------------|\n");
   printf("| 1   | users           | 0           |\n");
   printf("|-------------------------------------|\n");
   return 1;
  }
  else if (strcmp(cmd, ".version") == 0)
  {
   printf("csql版本: 0.1.0\n");
   return 1;
  }
  else if (strcmp(cmd, ".clear") == 0 || strcmp(cmd, ".c") == 0)
  {
   printf("\033[2J\033[1;1H");  // 清屏
   return 1;
  }
  else
  {
   printf("未知命令: %s\n", command);
   printf("输入 .help 查看可用命令\n");
   return 1;
  }
 }
 if (cmd[0] == '\\')
 {
  if (strcmp(cmd, "\\?") == 0)
  {
   print_help();
   return 1;
  }
  else if (strcmp(cmd, "\\q") == 0)
  {
   return 2;
  }
  else if (strcmp(cmd, "\\c") == 0)
  {
   printf("\033[2J\033[1;1H");  // 清屏
   return 1;
  }
  else if (strncmp(cmd, "\\d", 2) == 0)
  {
   // 提取表名
   char *table_name = cmd + 2;
   while (*table_name == ' ')
   {
    table_name++;
   }
   if (*table_name == '\0')
   {
    printf("用法: \\d 表名\n");
    return 1;
   }
   // 调用csql_describe_table函数
   csql_describe_table(table_name);
   return 1;
  }
 }
 return 0;
}
int main(int argc, char *argv[])
{
 // printf("csql v0.1.0\n");
 printf("csql v0.1.0, Writed by Bock.Tang\n");
 printf("输入'.help'获取帮助,'.exit'退出\n");
 // printf("使用\\d 表名查看表结构\n\n");
 printf("使用'[\\d 表名]'查看表结构\n\n");
 csql_ctx_t *ctx = csql_init();
 if (!ctx)
 {
  fprintf(stderr, "初始化数据库失败\n");
  return 1;
 }
 using_history();
 char *line = NULL;
 while (1)
 {
  line = readline("csql> ");
  if (!line)
  {
    break;
  }
  if (line[0] == '\0')
  {
   free(line);
   continue;
  }
  add_history(line);
  int meta_result = handle_meta_command(line);
  if (meta_result == 2)
  {
   free(line);
   break;
  }
  else if (meta_result == 1)
  {
   free(line);
   continue;
  }
  csql_result_t *result = csql_execute(ctx, line);
  if (result)
  {
   int rows = csql_result_row_count(result);
   int cols = csql_result_column_count(result);
   if (rows > 0 && cols > 0)
   {
    printf("\n");
    for (int i = 0; i < cols; i++)
    {
     const char *col_name = csql_result_column_name(result, i);
     printf("%-20s", col_name ? col_name : "NULL");
    }
    printf("\n");
    for (int i = 0; i < cols; i++)
    {
     printf("--------------------");
    }
    printf("\n");
    for (int i = 0; i < rows; i++)
    {
     for (int j = 0; j < cols; j++)
     {
      const char *cell = csql_result_cell(result, i, j);
      printf("%-20s", cell ? cell : "NULL");
     }
     printf("\n");
    }
    printf("\n(%d 行)\n\n", rows);
   }
   else
   {
    const char *msg = csql_error_message(ctx);
    if (msg && msg[0] != '\0')
    {
     printf("%s\n", msg);
    }
   }
   csql_free_result(result);
  }
  else
  {
   const char *error = csql_error_message(ctx);
   if (error && error[0] != '\0')
   {
    printf("错误: %s\n", error);
   }
  }
  free(line);
 }
 csql_close(ctx);
 printf("\n再见\n");
 return 0;
}
