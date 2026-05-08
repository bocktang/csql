#ifndef CSQL_PARSER_H
#define CSQL_PARSER_H
#include "lexer.h"
#include "nodes.h"
// 解析器结构
typedef struct parser
{
 lexer_t *lexer;
 token_t current;
 token_t previous;
 int had_error;
 int panic_mode;
} parser_t;
// 创建和销毁解析器
parser_t* parser_create(lexer_t *lexer);
void parser_destroy(parser_t *parser);
// 解析函数
ast_t* parser_parse(parser_t *parser);
// 解析控制函数
void parser_advance(parser_t *parser);
int parser_check(parser_t *parser, int type);
int parser_match(parser_t *parser, int type);
token_t parser_previous(parser_t *parser);
// 错误处理
void parser_error_at_current(parser_t *parser, const char *message);
void parser_error(parser_t *parser, const char *message);
#endif // CSQL_PARSER_H
