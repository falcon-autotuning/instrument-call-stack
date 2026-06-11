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

  CallStack *cs = instrument_call_stack_create("i", "g", 1, "cmd");

  push_callstack(L, cs, 0);
  lua_setglobal(L, "stack");

  assert_int_equal(luaL_dostring(L,
                                 "assert(stack:get_instrument_name() == 'i')\n"
                                 "assert(stack:get_channel_group() == 'g')\n"
                                 "assert(stack:get_channel() == 1)\n"
                                 "assert(stack:get_command() == 'cmd')\n"),
                   LUA_OK);

  instrument_call_stack_free(cs);
  lua_close(L);
}

static void test_push_callstack_global(void **state) {
  (void)state;

  lua_State *L = create_lua();

  CallStack *cs = instrument_call_stack_create("g1", "g2", 3, "g4");

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

  CallStack *cs = instrument_call_stack_create("x", "y", 4, "cmd");

  push_callstack(L, cs, 1);
  lua_setglobal(L, "stack");

  luaL_dostring(L, "stack=nil; collectgarbage()");

  lua_close(L);
}

static void test_lua_not_owned_gc(void **state) {
  (void)state;

  lua_State *L = create_lua();

  CallStack *cs = instrument_call_stack_create("x", "y", 4, "cmd");

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

  CallStack *cs = instrument_call_stack_create(NULL, NULL, -1, NULL);

  push_callstack(L, cs, 0);
  lua_setglobal(L, "stack");

  assert_int_equal(luaL_dostring(L,
                                 "assert(stack:get_instrument_name() == '')\n"
                                 "assert(stack:get_channel_group() == '')\n"
                                 "assert(stack:get_channel() == -1)\n"
                                 "assert(stack:get_command() == '')\n"),
                   LUA_OK);

  instrument_call_stack_free(cs);
  lua_close(L);
}

static void test_lua_to_string_method(void **state) {
  (void)state;

  lua_State *L = create_lua();

  CallStack *cs = instrument_call_stack_create("i", "g", 2, "cmd");

  push_callstack(L, cs, 0);
  lua_setglobal(L, "stack");

  assert_int_equal(luaL_dostring(L, "local s = stack:to_string()\n"
                                    "assert(type(s) == 'string')\n"
                                    "assert(s:match('CallStack%('))\n"
                                    "assert(s:match('i'))\n"
                                    "assert(s:match('g'))\n"
                                    "assert(s:match('2'))\n"
                                    "assert(s:match('cmd'))\n"),
                   LUA_OK);

  instrument_call_stack_free(cs);
  lua_close(L);
}

static void test_lua_tostring_metamethod(void **state) {
  (void)state;

  lua_State *L = create_lua();

  CallStack *cs = instrument_call_stack_create("i2", "g2", 5, "run");

  push_callstack(L, cs, 0);
  lua_setglobal(L, "stack");

  assert_int_equal(luaL_dostring(L, "local s = tostring(stack)\n"
                                    "assert(type(s) == 'string')\n"
                                    "assert(s:match('CallStack%('))\n"
                                    "assert(s:match('i2'))\n"
                                    "assert(s:match('g2'))\n"
                                    "assert(s:match('5'))\n"
                                    "assert(s:match('run'))\n"),
                   LUA_OK);

  instrument_call_stack_free(cs);
  lua_close(L);
}

static void test_lua_constructor_basic(void **state) {
  (void)state;

  lua_State *L = create_lua();

  register_instrument_call_stack(L);

  assert_int_equal(
      luaL_dostring(L, "local cs = instrument_call_stack.new('a','b',7,'c')\n"
                       "assert(cs:get_instrument_name() == 'a')\n"
                       "assert(cs:get_channel_group() == 'b')\n"
                       "assert(cs:get_channel() == 7)\n"
                       "assert(cs:get_command() == 'c')\n"),
      LUA_OK);

  lua_close(L);
}

static void test_lua_constructor_defaults(void **state) {
  (void)state;

  lua_State *L = create_lua();

  register_instrument_call_stack(L);

  assert_int_equal(luaL_dostring(L, "local cs = instrument_call_stack.new()\n"
                                    "assert(cs:get_instrument_name() == '')\n"
                                    "assert(cs:get_channel_group() == '')\n"
                                    "assert(cs:get_channel() == -1)\n"
                                    "assert(cs:get_command() == '')\n"),
                   LUA_OK);

  lua_close(L);
}

static void test_lua_clone_basic(void **state) {
  (void)state;

  lua_State *L = create_lua();

  register_instrument_call_stack(L);

  assert_int_equal(
      luaL_dostring(L, "local cs1 = instrument_call_stack.new('x','y',9,'z')\n"
                       "local cs2 = cs1:clone()\n"
                       "assert(cs2:get_instrument_name() == 'x')\n"
                       "assert(cs2:get_channel_group() == 'y')\n"
                       "assert(cs2:get_channel() == 9)\n"
                       "assert(cs2:get_command() == 'z')\n"),
      LUA_OK);

  lua_close(L);
}

static void test_lua_clone_independence(void **state) {
  (void)state;

  lua_State *L = create_lua();

  register_instrument_call_stack(L);

  /*
    Ensures clone is a different object (not same userdata)
  */
  assert_int_equal(
      luaL_dostring(L, "local cs1 = instrument_call_stack.new('a','b',1,'c')\n"
                       "local cs2 = cs1:clone()\n"
                       "assert(cs1 ~= cs2)\n"),
      LUA_OK);

  lua_close(L);
}

static void test_lua_clone_gc_owned(void **state) {
  (void)state;

  lua_State *L = create_lua();

  register_instrument_call_stack(L);

  /*
    Clone should be owned by Lua and GC-safe
  */
  assert_int_equal(
      luaL_dostring(L,
                    "local cs1 = instrument_call_stack.new('x','y',3,'cmd')\n"
                    "local cs2 = cs1:clone()\n"
                    "cs1 = nil\n"
                    "cs2 = nil\n"
                    "collectgarbage()\n"),
      LUA_OK);

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
      cmocka_unit_test(test_lua_to_string_method),
      cmocka_unit_test(test_lua_tostring_metamethod),
      cmocka_unit_test(test_lua_constructor_basic),
      cmocka_unit_test(test_lua_constructor_defaults),
      cmocka_unit_test(test_lua_clone_basic),
      cmocka_unit_test(test_lua_clone_independence),
      cmocka_unit_test(test_lua_clone_gc_owned),
  };

  return cmocka_run_group_tests(tests, NULL, NULL);
}
