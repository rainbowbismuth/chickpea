#ifndef CHICKPEA_LUA_ALLOC_H
#define CHICKPEA_LUA_ALLOC_H

#include "chickpea.h"

void lua_alloc_debug_mem_use(void);
void lua_alloc_free(void *ptr);
void *lua_alloc_realloc(void *ptr, size_t size);

#endif // CHICKPEA_LUA_ALLOC_H
