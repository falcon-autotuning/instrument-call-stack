#include <lua.h>
#include <lauxlib.h>

#include "instrument-call-stack/instrument-call-stack-lua.h"

/* =========================
   Lua wrapper struct
   ========================= */

typedef struct {
  CallStack *stack;
  int owned;
} lua_callstack;

/* =========================
   Helper
   ========================= */

static lua_callstack *check_callstack(lua_State *L) {
  return (lua_callstack *)luaL_checkudata(L, 1, "CallStack");
}

/* =========================
   Methods
   ========================= */

static int l_get_instrument_name(lua_State *L) {
  lua_callstack *cs = check_callstack(L);
  lua_pushstring(L, instrument_call_stack_get_instrument_name(cs->stack));
  return 1;
}

static int l_get_channel_group(lua_State *L) {
  lua_callstack *cs = check_callstack(L);
  lua_pushstring(L, instrument_call_stack_get_channel_group(cs->stack));
  return 1;
}

static int l_get_channel(lua_State *L) {
  lua_callstack *cs = check_callstack(L);
  lua_pushstring(L, instrument_call_stack_get_channel(cs->stack));
  return 1;
}

static int l_get_command(lua_State *L) {
  lua_callstack *cs = check_callstack(L);
  lua_pushstring(L, instrument_call_stack_get_command(cs->stack));
  return 1;
}

/* =========================
   GC
   ========================= */

static int l_callstack_gc(lua_State *L) {
  lua_callstack *cs = check_callstack(L);

  if (cs->owned && cs->stack) {
    instrument_call_stack_free(cs->stack);
    cs->stack = NULL;
  }

  return 0;
}

/* =========================
   Methods table
   ========================= */

static const luaL_Reg callstack_methods[] = {
    {"get_instrument_name", l_get_instrument_name},
    {"get_channel_group", l_get_channel_group},
    {"get_channel", l_get_channel},
    {"get_command", l_get_command},
    {NULL, NULL}};

/* =========================
   Public API
   ========================= */

void push_callstack(lua_State *L, CallStack *stack, int owned) {
  lua_callstack *cs =
      (lua_callstack *)lua_newuserdata(L, sizeof(lua_callstack));

  cs->stack = stack;
  cs->owned = owned;

  luaL_getmetatable(L, "CallStack");
  lua_setmetatable(L, -2);
}

void push_callstack_global(lua_State *L, CallStack *stack, int owned,
                           const char *name) {
  push_callstack(L, stack, owned);
  lua_setglobal(L, name);
}

void register_instrument_call_stack(lua_State *L) {
  luaopen_instrument_call_stack(L);
  lua_setglobal(L, "instrument_call_stack");
}

/* =========================
   Module Init
   ========================= */

int luaopen_instrument_call_stack(lua_State *L) {
  /* Create metatable */
  luaL_newmetatable(L, "CallStack");

  /* __gc */
  lua_pushcfunction(L, l_callstack_gc);
  lua_setfield(L, -2, "__gc");

  /* __index */
  lua_newtable(L);
  luaL_setfuncs(L, callstack_methods, 0);
  lua_setfield(L, -2, "__index");

  lua_pop(L, 1);

  /* return module table (empty for now) */
  lua_newtable(L);
  return 1;
}
