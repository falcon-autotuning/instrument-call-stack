#include <stdarg.h>
#include <stddef.h>
#include <cmocka.h>

#include <lauxlib.h>
#include <lualib.h>

#include "instrument-call-stack/instrument-call-stack.h"
#include "instrument-call-stack/instrument-call-stack-lua.h"

/* =========================
   Helpers
   ========================= */

static lua_State *create_lua(void) {
  lua_State *L = luaL_newstate();
  assert_non_null(L);

  luaL_openlibs(L);

  luaopen_instrument_call_stack(L);
  lua_pop(L, 1);

  return L;
}

/* =========================
   Tests
   ========================= */

static void test_lua_getters_basic(void **state) {
  (void)state;

  lua_State *L = create_lua();

  CallStack *cs = instrument_call_stack_create("i", "g", "c", "cmd");

  push_callstack(L, cs, 0);
  lua_setglobal(L, "stack");

  assert_int_equal(luaL_dostring(L,
                                 "assert(stack:get_instrument_name() == 'i')\n"
                                 "assert(stack:get_channel_group() == 'g')\n"
                                 "assert(stack:get_channel() == 'c')\n"
                                 "assert(stack:get_command() == 'cmd')\n"),
                   LUA_OK);

  instrument_call_stack_free(cs);
  lua_close(L);
}

static void test_push_callstack_global(void **state) {
  (void)state;

  lua_State *L = create_lua();

  CallStack *cs = instrument_call_stack_create("g1", "g2", "g3", "g4");

  push_callstack_global(L, cs, 0, "global_stack");

  assert_int_equal(
      luaL_dostring(L, "assert(global_stack:get_command() == 'g4')"), LUA_OK);

  instrument_call_stack_free(cs);
  lua_close(L);
}

static void test_register_module(void **state) {
  (void)state;

  lua_State *L = luaL_newstate();
  luaL_openlibs(L);

  register_instrument_call_stack(L);

  assert_int_equal(luaL_dostring(L, "assert(instrument_call_stack ~= nil)"),
                   LUA_OK);

  lua_close(L);
}

static void test_lua_owned_gc(void **state) {
  (void)state;

  lua_State *L = create_lua();

  CallStack *cs = instrument_call_stack_create("x", "y", "z", "cmd");

  push_callstack(L, cs, 1);
  lua_setglobal(L, "stack");

  luaL_dostring(L, "stack=nil; collectgarbage()");

  lua_close(L);
}

static void test_lua_not_owned_gc(void **state) {
  (void)state;

  lua_State *L = create_lua();

  CallStack *cs = instrument_call_stack_create("x", "y", "z", "cmd");

  push_callstack(L, cs, 0);
  lua_setglobal(L, "stack");

  luaL_dostring(L, "stack=nil; collectgarbage()");

  assert_non_null(cs);

  instrument_call_stack_free(cs);
  lua_close(L);
}

static void test_lua_null_safety(void **state) {
  (void)state;

  lua_State *L = create_lua();

  CallStack *cs = instrument_call_stack_create(NULL, NULL, NULL, NULL);

  push_callstack(L, cs, 0);
  lua_setglobal(L, "stack");

  assert_int_equal(luaL_dostring(L,
                                 "assert(stack:get_instrument_name() == '')\n"
                                 "assert(stack:get_channel_group() == '')\n"
                                 "assert(stack:get_channel() == '')\n"
                                 "assert(stack:get_command() == '')\n"),
                   LUA_OK);

  instrument_call_stack_free(cs);
  lua_close(L);
}

/* =========================
   MAIN
   ========================= */

int main(void) {
  const struct CMUnitTest tests[] = {
      cmocka_unit_test(test_lua_getters_basic),
      cmocka_unit_test(test_push_callstack_global),
      cmocka_unit_test(test_register_module),
      cmocka_unit_test(test_lua_owned_gc),
      cmocka_unit_test(test_lua_not_owned_gc),
      cmocka_unit_test(test_lua_null_safety),
  };

  return cmocka_run_group_tests(tests, NULL, NULL);
}
