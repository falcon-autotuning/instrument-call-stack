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
  lua_pushnumber(L, instrument_call_stack_get_channel(cs->stack));
  return 1;
}

static int l_get_command(lua_State *L) {
  lua_callstack *cs = check_callstack(L);
  lua_pushstring(L, instrument_call_stack_get_command(cs->stack));
  return 1;
}

static int l_callstack_new(lua_State *L) {
  if (!lua_istable(L, 1)) {
    return luaL_error(L, "instrument_call_stack.new expects a table");
  }

  const char *instrument = "";
  const char *group = "";
  const char *command = "";
  int channel = -1;

  /* instrument (required) */
  lua_getfield(L, 1, "instrument");
  if (lua_isnil(L, -1)) {
    lua_pop(L, 1);
    return luaL_error(L, "instrument is required");
  }
  instrument = luaL_checkstring(L, -1);
  lua_pop(L, 1);

  /* group (optional) */
  lua_getfield(L, 1, "group");
  if (!lua_isnil(L, -1)) {
    group = luaL_checkstring(L, -1);
  }
  lua_pop(L, 1);

  /* channel (optional) */
  lua_getfield(L, 1, "channel");
  if (!lua_isnil(L, -1)) {
    channel = (int)luaL_checkinteger(L, -1);
  }
  lua_pop(L, 1);

  /* command (required) */
  lua_getfield(L, 1, "command");
  if (lua_isnil(L, -1)) {
    lua_pop(L, 1);
    return luaL_error(L, "command is required");
  }
  command = luaL_checkstring(L, -1);
  lua_pop(L, 1);

  CallStack *cs =
      instrument_call_stack_create(instrument, group, channel, command);

  if (!cs) {
    return luaL_error(L, "Failed to create CallStack");
  }

  push_callstack(L, cs, 1);
  return 1;
}

static int l_callstack_clone(lua_State *L) {
  lua_callstack *cs = check_callstack(L);

  CallStack *copy = instrument_call_stack_create(
      instrument_call_stack_get_instrument_name(cs->stack),
      instrument_call_stack_get_channel_group(cs->stack),
      instrument_call_stack_get_channel(cs->stack),
      instrument_call_stack_get_command(cs->stack));

  if (!copy) {
    return luaL_error(L, "Failed to clone CallStack");
  }

  push_callstack(L, copy, 1); // new Lua-owned object
  return 1;
}

static int l_callstack_tostring(lua_State *L) {
  lua_callstack *cs = check_callstack(L);

  lua_pushfstring(L, "CallStack(%s,%s,%d,%s)",
                  instrument_call_stack_get_instrument_name(cs->stack),
                  instrument_call_stack_get_channel_group(cs->stack),
                  instrument_call_stack_get_channel(cs->stack),
                  instrument_call_stack_get_command(cs->stack));

  return 1;
}
static int l_callstack_to_string(lua_State *L) {
  return l_callstack_tostring(L);
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
    {"clone", l_callstack_clone},
    {"to_string", l_callstack_to_string},
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

static const luaL_Reg module_funcs[] = {{"new", l_callstack_new}, {NULL, NULL}};

int luaopen_instrument_call_stack(lua_State *L) {
  luaL_newmetatable(L, "CallStack");

  lua_pushcfunction(L, l_callstack_tostring);
  lua_setfield(L, -2, "__tostring");

  lua_pushcfunction(L, l_callstack_gc);
  lua_setfield(L, -2, "__gc");

  lua_newtable(L);
  luaL_setfuncs(L, callstack_methods, 0);
  lua_setfield(L, -2, "__index");

  lua_pop(L, 1);

  lua_newtable(L);
  luaL_setfuncs(L, module_funcs, 0);

  return 1;
}
