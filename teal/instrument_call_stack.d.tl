global record CallStack
   get_instrument_name: function(self: CallStack): string
   get_channel_group: function(self: CallStack): string
   get_channel: function(self: CallStack): number
   get_command: function(self: CallStack): string
   clone: function(self: CallStack): CallStack
   to_string: function(self: CallStack): string
end

global record instrument_call_stack
   new: function(string, string, number, string): CallStack
end
