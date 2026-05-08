#ifndef CSQL_NODES_H
#define CSQL_NODES_H
// AST节点类型
typedef enum
{
 AST_SELECT,
 AST_CREATE,
 AST_INSERT,
 AST_UPDATE,
 AST_DELETE,
 AST_EXPRESSION
} ast_type_t;
// 数据类型
typedef enum
{
 TYPE_INT,
 TYPE_FLOAT,
 TYPE_TEXT,
 TYPE_BOOL,
 TYPE_UNKNOWN
} data_type_t;
// 抽象语法树基类
typedef struct ast_node
{
 ast_type_t type;
} ast_t;
typedef struct column_def
{
 char *name;
 data_type_t type;
 int primary_key;
} column_def_t;
// SELECT语句
typedef struct ast_select
{
 ast_type_t type;
 char **columns;
 int column_count;
 int all_columns;
 char *from;
 ast_t *where;
} ast_select_t;
// CREATE TABLE语句
typedef struct ast_create
{
 ast_type_t type;
 char *table;
 column_def_t **columns;
 int column_count;
} ast_create_t;
// INSERT语句
typedef struct ast_insert
{
 ast_type_t type;
 char *table;
 ast_t **values;
 int value_count;
} ast_insert_t;
// UPDATE SET项
typedef struct ast_update_set
{
 char *column;
 ast_t *value;
} ast_update_set_t;
// UPDATE语句
typedef struct ast_update
{
 ast_type_t type;
 char *table;
 ast_update_set_t **set_items;
 int set_count;
 ast_t *where;
} ast_update_t;
// DELETE语句
typedef struct ast_delete
{
 ast_type_t type;
 char *from;
 ast_t *where;
} ast_delete_t;
// 表达式
typedef struct ast_expression
{
 ast_type_t type;
 char *value;
 ast_t *left;
 ast_t *right;
 int op;
} ast_expression_t;
#endif
