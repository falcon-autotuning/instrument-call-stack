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

  assert_int_equal(luaL_dostring(L, "local cs = instrument_call_stack.new{\n"
                                    "  instrument = 'a',\n"
                                    "  group = 'b',\n"
                                    "  channel = 7,\n"
                                    "  command = 'c'\n"
                                    "}\n"
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

  assert_int_equal(luaL_dostring(L, "local cs = instrument_call_stack.new{\n"
                                    "  instrument = 'x',\n"
                                    "  command = 'y'\n"
                                    "}\n"
                                    "assert(cs:get_instrument_name() == 'x')\n"
                                    "assert(cs:get_channel_group() == '')\n"
                                    "assert(cs:get_channel() == -1)\n"
                                    "assert(cs:get_command() == 'y')\n"),
                   LUA_OK);

  lua_close(L);
}

static void test_lua_clone_basic(void **state) {
  (void)state;

  lua_State *L = create_lua();

  register_instrument_call_stack(L);

  assert_int_equal(luaL_dostring(L, "local cs1 = instrument_call_stack.new{\n"
                                    "  instrument = 'x',\n"
                                    "  group = 'y',\n"
                                    "  channel = 9,\n"
                                    "  command = 'z'\n"
                                    "}\n"
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

  assert_int_equal(luaL_dostring(L, "local cs1 = instrument_call_stack.new{\n"
                                    "  instrument = 'a',\n"
                                    "  group = 'b',\n"
                                    "  channel = 1,\n"
                                    "  command = 'c'\n"
                                    "}\n"
                                    "local cs2 = cs1:clone()\n"
                                    "assert(cs1 ~= cs2)\n"),
                   LUA_OK);

  lua_close(L);
}

static void test_lua_clone_gc_owned(void **state) {
  (void)state;

  lua_State *L = create_lua();

  register_instrument_call_stack(L);

  assert_int_equal(luaL_dostring(L, "local cs1 = instrument_call_stack.new{\n"
                                    "  instrument = 'x',\n"
                                    "  group = 'y',\n"
                                    "  channel = 3,\n"
                                    "  command = 'cmd'\n"
                                    "}\n"
                                    "local cs2 = cs1:clone()\n"
                                    "cs1 = nil\n"
                                    "cs2 = nil\n"
                                    "collectgarbage()\n"),
                   LUA_OK);

  lua_close(L);
}

static void test_lua_constructor_missing_required(void **state) {
  (void)state;

  lua_State *L = create_lua();
  register_instrument_call_stack(L);

  assert_true(luaL_dostring(L, "local ok, err = pcall(function()\n"
                               "  instrument_call_stack.new{}\n"
                               "end)\n"
                               "assert(ok == false)\n") == LUA_OK);

  lua_close(L);
}

static void test_lua_check_callstack_valid(void **state) {
  (void)state;

  lua_State *L = create_lua();

  CallStack *original = instrument_call_stack_create("i", "g", 1, "cmd");

  push_callstack(L, original, 0);

  CallStack *extracted = lua_check_callstack(L, -1);

  assert_non_null(extracted);
  assert_string_equal(instrument_call_stack_get_command(extracted), "cmd");

  instrument_call_stack_free(original);
  lua_close(L);
}

static void test_lua_check_callstack_non_userdata(void **state) {
  (void)state;

  lua_State *L = create_lua();

  lua_pushstring(L, "not a callstack");

  CallStack *extracted = lua_check_callstack(L, -1);

  assert_null(extracted);

  lua_close(L);
}

static void test_lua_check_callstack_wrong_metatable(void **state) {
  (void)state;

  lua_State *L = create_lua();

  // Create unrelated userdata
  void *ud = lua_newuserdata(L, sizeof(int));
  (void)ud;

  luaL_newmetatable(L, "NotCallStack");
  lua_setmetatable(L, -2);

  CallStack *extracted = lua_check_callstack(L, -1);

  assert_null(extracted);

  lua_close(L);
}

static void test_lua_check_callstack_null_internal(void **state) {
  (void)state;

  lua_State *L = create_lua();

  // Manually push userdata with NULL stack
  typedef struct {
    CallStack *stack;
    int owned;
  } lua_callstack;

  lua_callstack *cs =
      (lua_callstack *)lua_newuserdata(L, sizeof(lua_callstack));

  cs->stack = NULL;
  cs->owned = 0;

  luaL_newmetatable(L, "CallStack");
  lua_setmetatable(L, -2);

  CallStack *extracted = lua_check_callstack(L, -1);

  assert_null(extracted);

  lua_close(L);
}

static void test_lua_check_callstack_indexing(void **state) {
  (void)state;

  lua_State *L = create_lua();

  CallStack *cs = instrument_call_stack_create("a", "", -1, "b");

  push_callstack(L, cs, 0);

  lua_pushnumber(L, 42); // push extra value

  // CallStack is now at -2
  CallStack *extracted = lua_check_callstack(L, -2);

  assert_non_null(extracted);
  assert_string_equal(instrument_call_stack_get_instrument_name(extracted),
                      "a");

  instrument_call_stack_free(cs);
  lua_close(L);
}

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
      cmocka_unit_test(test_lua_constructor_missing_required),
      cmocka_unit_test(test_lua_check_callstack_valid),
      cmocka_unit_test(test_lua_check_callstack_non_userdata),
      cmocka_unit_test(test_lua_check_callstack_wrong_metatable),
      cmocka_unit_test(test_lua_check_callstack_null_internal),
      cmocka_unit_test(test_lua_check_callstack_indexing),
  };

  return cmocka_run_group_tests(tests, NULL, NULL);
}
