#ifndef CSQL_H
#define CSQL_H
#ifdef __cplusplus
extern "C"
{
 #endif
 // 版本信息
 #define CSQL_VERSION_STRING "0.1.0"
 // 列定义结构体
 typedef struct
 {
  char name[32];
  char type[16];  // 存储列类型,如"INT"、"TEXT"等
  int is_primary_key;
 } ColumnDef;
 // 数据库上下文
 typedef struct csql_ctx csql_ctx_t;
 // 查询结果
 typedef struct csql_result csql_result_t;
 // 数据库API
 csql_ctx_t* csql_init(void);
 void csql_close(csql_ctx_t *ctx);
 // SQL执行
 csql_result_t* csql_execute(csql_ctx_t *ctx, const char *sql);
 // 结果集处理
 void csql_free_result(csql_result_t *result);
 int csql_result_row_count(csql_result_t *result);
 int csql_result_column_count(csql_result_t *result);
 const char* csql_result_column_name(csql_result_t *result, int col);
 const char* csql_result_cell(csql_result_t *result, int row, int col);
 // 错误处理
 const char* csql_error_message(csql_ctx_t *ctx);
 int csql_affected_rows(csql_ctx_t *ctx);
 // 描述表结构
 void csql_describe_table(const char *table_name);
 // 版本信息
 const char* csql_version(void);
 #ifdef __cplusplus
}
#endif
#endif // CSQL_H
