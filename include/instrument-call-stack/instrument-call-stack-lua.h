#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <lua.h>

#include "instrument-call-stack.h"
#include "instrument-call-stack/instrument-call-stack-export.h"

/**
 * @brief Push a CallStack into the Lua stack as userdata.
 *
 * @param L Lua state.
 * @param stack Pointer to an existing CallStack instance.
 * @param owned If non-zero, Lua takes ownership and will free it via GC.
 *
 * @note The userdata is pushed onto the Lua stack.
 *       The caller is responsible for assigning it (e.g. lua_setglobal).
 */
INSTRUMENT_CALL_STACK_EXPORT void push_callstack(lua_State *L, CallStack *stack,
                                                 int owned);

/**
 * @brief Lua module initializer.
 *
 * @param L Lua state.
 * @return Number of values returned to Lua (module table).
 *
 * @note Allows usage via require("instrument_call_stack").
 */
INSTRUMENT_CALL_STACK_EXPORT int luaopen_instrument_call_stack(lua_State *L);

/**
 * @brief Push a CallStack into Lua and assign it to a global variable.
 *
 * @param L Lua state.
 * @param stack CallStack pointer.
 * @param owned If non-zero, Lua owns lifetime.
 * @param name Name of the global variable.
 */
INSTRUMENT_CALL_STACK_EXPORT void push_callstack_global(lua_State *L,
                                                        CallStack *stack,
                                                        int owned,
                                                        const char *name);

/**
 * @brief Register the CallStack module as a global Lua table.
 *
 * @param L Lua state.
 *
 * @note After calling this, Lua will have:
 *       instrument_call_stack = { ... }
 */
INSTRUMENT_CALL_STACK_EXPORT void register_instrument_call_stack(lua_State *L);

/**
 * @brief Extract a CallStack pointer from a Lua userdata.
 *
 * @param L Lua state
 * @param index Stack index
 * @return CallStack* or NULL if not valid
 */
INSTRUMENT_CALL_STACK_EXPORT
CallStack *lua_check_callstack(lua_State *L, int index);

#ifdef __cplusplus
}
#endif
