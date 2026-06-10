find_package(cmocka REQUIRED)

add_executable(test_call_stack
    test_call_stack.c
)

target_link_libraries(test_call_stack
    instrument-call-stack
    cmocka::cmocka
)

add_test(NAME call_stack_tests COMMAND test_call_stack)

if(BUILD_LUA)
  find_package(Lua REQUIRED)
  # Normalize Lua target across environments
  if(NOT TARGET Lua::Lua)
    add_library(Lua::Lua INTERFACE IMPORTED)
    set_target_properties(Lua::Lua PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES "${LUA_INCLUDE_DIR}"
    INTERFACE_LINK_LIBRARIES "${LUA_LIBRARIES}"
  )
  endif()

  add_executable(test_call_stack_lua
    test_call_stack_lua.c
  )

  target_link_libraries(test_call_stack_lua
    instrument-call-stack
    instrument-call-stack-lua
    Lua::Lua
    cmocka::cmocka
  )

  add_test(NAME call_stack_lua_tests COMMAND test_call_stack_lua)
endif()
