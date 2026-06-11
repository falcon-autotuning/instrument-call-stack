# Using instrument-call-stack with Teal

The Lua binding exposes `CallStack` objects as userdata with methods.  
These can be used in Teal via a type declaration file.

See the `teal/iinstrument_call_stack.d.tl` for explicit types.

## Example

```lua
local stack: CallStack = stack  -- injected from C

local name: string = stack:get_instrument_name()
local command: string = stack:get_command()
