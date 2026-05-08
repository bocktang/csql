#include "../include/lexer.h"
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
lexer_t* lexer_create(const char *source)
{
 lexer_t *lexer = malloc(sizeof(lexer_t));
 if (!lexer)
 {
  return NULL;
 }
 lexer->source = source;
 lexer->current = 0;
 lexer->line = 1;
 return lexer;
}
void lexer_destroy(lexer_t *lexer)
{
 if (lexer)
 {
  free(lexer);
 }
}
static token_t make_token(lexer_t *lexer, token_type_t type, int length)
{
 token_t token;
 token.type = type;
 token.start = lexer->current - length;
 token.length = length;
 token.line = lexer->line;
 token.lexeme = lexer->source + token.start;
 return token;
}
token_t lexer_next_token(lexer_t *lexer)
{
 // 跳过空白字符
 while (isspace(lexer->source[lexer->current]))
 {
  if (lexer->source[lexer->current] == '\n')
  {
   lexer->line++;
  }
  lexer->current++;
 }
 if (lexer->source[lexer->current] == '\0')
 {
  return make_token(lexer, TOKEN_EOF, 0);
 }
 char c = lexer->source[lexer->current];
 lexer->current++;
 // 处理单字符标记
 switch (c)
 {
  case '*': return make_token(lexer, TOKEN_STAR, 1);
  case ',': return make_token(lexer, TOKEN_COMMA, 1);
  case ';': return make_token(lexer, TOKEN_SEMICOLON, 1);
  case '(': return make_token(lexer, TOKEN_LEFT_PAREN, 1);
  case ')': return make_token(lexer, TOKEN_RIGHT_PAREN, 1);
  case '=': return make_token(lexer, TOKEN_EQUAL, 1);
  case '>': return make_token(lexer, TOKEN_GREATER, 1);
  case '<': return make_token(lexer, TOKEN_LESS, 1);
 }
 // 处理标识符和关键字
 if (isalpha(c) || c == '_')
 {
  int start = lexer->current - 1;
  while (isalnum(lexer->source[lexer->current]) || lexer->source[lexer->current] == '_')
  {
   lexer->current++;
  }
  int length = lexer->current - start;
  // 检查关键字
  char keyword[32];
  strncpy(keyword, lexer->source + start, length);
  keyword[length] = '\0';
  if (strcasecmp(keyword, "SELECT") == 0)
  {
   return make_token(lexer, TOKEN_SELECT, length);
  }
  if (strcasecmp(keyword, "CREATE") == 0)
  {
   return make_token(lexer, TOKEN_CREATE, length);
  }
  if (strcasecmp(keyword, "TABLE") == 0)
  {
   return make_token(lexer, TOKEN_TABLE, length);
  }
  if (strcasecmp(keyword, "INSERT") == 0)
  {
   return make_token(lexer, TOKEN_INSERT, length);
  }
  if (strcasecmp(keyword, "INTO") == 0)
  {
   return make_token(lexer, TOKEN_INTO, length);
  }
  if (strcasecmp(keyword, "VALUES") == 0)
  {
   return make_token(lexer, TOKEN_VALUES, length);
  }
  if (strcasecmp(keyword, "UPDATE") == 0)
  {
   return make_token(lexer, TOKEN_UPDATE, length);
  }
  if (strcasecmp(keyword, "SET") == 0)
  {
   return make_token(lexer, TOKEN_SET, length);
  }
  if (strcasecmp(keyword, "DELETE") == 0)
  {
   return make_token(lexer, TOKEN_DELETE, length);
  }
  if (strcasecmp(keyword, "FROM") == 0)
  {
   return make_token(lexer, TOKEN_FROM, length);
  }
  if (strcasecmp(keyword, "WHERE") == 0)
  {
   return make_token(lexer, TOKEN_WHERE, length);
  }
  if (strcasecmp(keyword, "PRIMARY") == 0)
  {
   return make_token(lexer, TOKEN_PRIMARY, length);
  }
  if (strcasecmp(keyword, "KEY") == 0)
  {
   return make_token(lexer, TOKEN_KEY, length);
  }
  return make_token(lexer, TOKEN_IDENTIFIER, length);
 }
 // 处理数字
 if (isdigit(c))
 {
  int start = lexer->current - 1;
  while (isdigit(lexer->source[lexer->current]) || lexer->source[lexer->current] == '.')
  {
   lexer->current++;
  }
  int length = lexer->current - start;
  return make_token(lexer, TOKEN_NUMBER, length);
 }
 // 处理字符串
 if (c == '\'')
 {
  int start = lexer->current;
  while (lexer->source[lexer->current] != '\'' && lexer->source[lexer->current] != '\0')
  {
   if (lexer->source[lexer->current] == '\n')
   {
    lexer->line++;
   }
   lexer->current++;
  }
  if (lexer->source[lexer->current] == '\'')
  {
   lexer->current++;
   int length = lexer->current - start;
   return make_token(lexer, TOKEN_STRING, length);
  }
  else
  {
   return make_token(lexer, TOKEN_ERROR, 1);
  }
 }
 // 未知字符
 return make_token(lexer, TOKEN_ERROR, 1);
}
