#include "../include/parser.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
// 错误处理函数
void parser_error_at_current(parser_t *parser, const char *message)
{
 if (!parser)
 {
  return;
 }
 if (parser->panic_mode)
 {
  return;
 }
 parser->panic_mode = 1;
 parser->had_error = 1;
 fprintf(stderr, "错误: %s\n", message);
}
void parser_error(parser_t *parser, const char *message)
{
 if (!parser)
 {
  return;
 }
 if (parser->panic_mode)
 {
  return;
 }
 parser->panic_mode = 1;
 parser->had_error = 1;
 fprintf(stderr, "解析错误: %s\n", message);
}
// 前进到下一个标记
void parser_advance(parser_t *parser)
{
 if (!parser || !parser->lexer)
 {
  return;
 }
 parser->previous = parser->current;
 parser->current = lexer_next_token(parser->lexer);
}
// 检查当前标记类型
int parser_check(parser_t *parser, int type)
{
 if (!parser)
 {
  return 0;
 }
 return parser->current.type == type;
}
// 匹配并消费一个标记
int parser_match(parser_t *parser, int type)
{
 if (!parser_check(parser, type))
 {
  return 0;
 }
 parser_advance(parser);
 return 1;
}
// 获取前一个标记
token_t parser_previous(parser_t *parser)
{
 static token_t empty_token = {TOKEN_EOF, 0, 0, 1, NULL};
 if (!parser)
 {
  return empty_token;
 }
 return parser->previous;
}
// 创建解析器
parser_t* parser_create(lexer_t *lexer)
{
 if (!lexer)
 {
  return NULL;
 }
 parser_t *parser = malloc(sizeof(parser_t));
 if (!parser)
 {
  return NULL;
 }
 parser->lexer = lexer;
 parser->had_error = 0;
 parser->panic_mode = 0;
 // 初始化标记
 parser->current.type = TOKEN_EOF;
 parser->current.start = 0;
 parser->current.length = 0;
 parser->current.line = 1;
 parser->current.lexeme = "";
 parser->previous.type = TOKEN_EOF;
 parser->previous.start = 0;
 parser->previous.length = 0;
 parser->previous.line = 1;
 parser->previous.lexeme = "";
 // 获取第一个标记
 parser_advance(parser);
 return parser;
}
// 销毁解析器
void parser_destroy(parser_t *parser)
{
 if (parser)
 {
  free(parser);
 }
}
// 主解析函数
ast_t* parser_parse(parser_t *parser)
{
 if (!parser)
 {
  return NULL;
 }
 // 简化实现: 总是返回一个空节点
 ast_expression_t *expr = malloc(sizeof(ast_expression_t));
 if (!expr) return NULL;
 expr->type = AST_EXPRESSION;
 expr->value = strdup("parsed");
 expr->left = NULL;
 expr->right = NULL;
 expr->op = 0;
 return (ast_t*)expr;
}
