#include "../include/backend.h"
#include <stdlib.h>
void backend_init(void) { }
int backend_create_table(void *stmt) { return 0; }
int backend_insert(void *stmt) { return 0; }
void *backend_select(void *stmt) { return NULL; }
void backend_free_result(void *result) { }
