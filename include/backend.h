#ifndef CSQL_BACKEND_H
#define CSQL_BACKEND_H
void backend_init(void);
int backend_create_table(void *stmt);
int backend_insert(void *stmt);
void *backend_select(void *stmt);
void backend_free_result(void *result);
#endif
