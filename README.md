# instrument-call-stack

`instrument-call-stack` is a small C99 library for representing and transporting instrument call stack metadata. It provides a simple, opaque data structure with fixed-size storage and supports serialization for use across process or language boundaries. Optional Lua bindings allow read-only access to the same data model from embedded scripting environments.

The library is designed for use in instrumentation systems, telemetry pipelines, and cross-language integrations where predictable memory layout and minimal allocation overhead are required.

***

## Overview

A `CallStack` represents four pieces of metadata:

* Instrument name
* Channel group
* Channel
* Command

Each field is stored as a fixed-size string, allowing for a compact and stable in-memory representation. This enables efficient serialization and avoids dynamic allocation within the object itself.

***

## C API

### Creation and lifetime

```c
CallStack *instrument_call_stack_create(
    const char *instrument_name,
    const char *channel_group,
    const char *channel,
    const char *command);

void instrument_call_stack_free(CallStack *stack);
```

The object is immutable after creation and must be released with `instrument_call_stack_free`.

***

### Field access

```c
const char *instrument_call_stack_get_instrument_name(const CallStack *);
const char *instrument_call_stack_get_channel_group(const CallStack *);
const char *instrument_call_stack_get_channel(const CallStack *);
const char *instrument_call_stack_get_command(const CallStack *);
```

Returned strings are owned by the `CallStack` and must not be modified or freed.

***

### Serialization

```c
char *instrument_call_stack_serialize(const CallStack *);
CallStack *instrument_call_stack_deserialize(const char *buffer);
```

Serialization produces a fixed-size binary representation suitable for transmission or storage. The returned buffer must be freed with `free()`.

***

## Lua Bindings

Lua bindings are optional and controlled by the `BUILD_LUA` build option. When enabled, `CallStack` objects can be exposed to Lua as userdata with read-only access to their fields.

### Available methods

```lua
stack:get_instrument_name()
stack:get_channel_group()
stack:get_channel()
stack:get_command()
```

***

### Passing objects to Lua

```c
push_callstack_global(L, stack, owned, "stack");
```

This makes the object available to Lua as a global variable. The `owned` flag determines whether Lua is responsible for freeing the underlying object when it is garbage collected.

***

### Module registration

```c
register_instrument_call_stack(L);
```

Registers a module table in the Lua state.

***

## Teal Support

This library can be used from Teal (typed Lua) via the provided
type definitions.

See `docs/teal.md` for details.

***

## Building

The project uses CMake and a Makefile wrapper. Dependencies are managed via vcpkg.

### Configure and build

```bash
make build
```

Build configuration is controlled through CMake presets. The default presets enable tests and can optionally enable Lua bindings.

### Enabling Lua

Lua support is enabled via:

```bash
cmake --preset <preset> -DBUILD_LUA=ON
```

Alternatively, presets may already define this flag.

***

## Running Tests

Tests are built and run through the same workflow:

```bash
make test
```

This executes both the core C tests and, when enabled, the Lua binding tests.

***

## Design Considerations

The library uses fixed-size storage to ensure predictable memory usage and straightforward serialization. This makes it well suited for systems where ABI stability and minimal overhead are prioritized. The API enforces immutability, allowing safe concurrent read access as long as object lifetime is externally managed.

Lua bindings are intentionally limited to read-only operations to preserve consistency with the C data model and prevent unexpected mutations across language boundaries.

***

## License

See [LICENSE](LICENSE) for details.
