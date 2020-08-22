#ifndef CHICKPEA_LUA_ALLOC_H
#define CHICKPEA_LUA_ALLOC_H

#include "chickpea.h"

void lua_alloc_init(void);
void lua_alloc_free(void *ptr);
void *lua_alloc_realloc(void *ptr, size_t new_size);

#endif // CHICKPEA_LUA_ALLOC_H
