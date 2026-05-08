#ifndef CSQL_LEXER_H
#define CSQL_LEXER_H
typedef enum
{
 TOKEN_EOF = 0,
 TOKEN_ERROR,
 TOKEN_IDENTIFIER,
 TOKEN_NUMBER,
 TOKEN_STRING,
 TOKEN_SELECT,
 TOKEN_CREATE,
 TOKEN_TABLE,
 TOKEN_INSERT,
 TOKEN_INTO,
 TOKEN_VALUES,
 TOKEN_UPDATE,
 TOKEN_SET,
 TOKEN_DELETE,
 TOKEN_FROM,
 TOKEN_WHERE,
 TOKEN_STAR,
 TOKEN_COMMA,
 TOKEN_SEMICOLON,
 TOKEN_LEFT_PAREN,
 TOKEN_RIGHT_PAREN,
 TOKEN_EQUAL,
 TOKEN_GREATER,
 TOKEN_LESS,
 TOKEN_PRIMARY,
 TOKEN_KEY
} token_type_t;
typedef struct
{
 token_type_t type;
 int start;          // 在源代码中的起始位置
 int length;         // 标记长度
 int line;           // 行号
 const char *lexeme; // 标记的实际字符串
} token_t;
typedef struct
{
 const char *source;
 int current;
 int line;
} lexer_t;
lexer_t* lexer_create(const char *source);
void lexer_destroy(lexer_t *lexer);
token_t lexer_next_token(lexer_t *lexer);
#endif
