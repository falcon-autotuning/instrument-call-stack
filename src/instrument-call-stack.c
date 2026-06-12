#include "instrument-call-stack/instrument-call-stack.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct CallStack {
  char instrument_name[INST_STACK_MAX_STRING_LEN];
  char channel_group[INST_STACK_MAX_STRING_LEN];
  int channel;
  char command[INST_STACK_MAX_STRING_LEN];
};

static void copy_string(char *dest, const char *src) {
  if (!dest)
    return;

  if (!src) {
    dest[0] = '\0';
    return;
  }

  strncpy(dest, src, INST_STACK_MAX_STRING_LEN - 1);
  dest[INST_STACK_MAX_STRING_LEN - 1] = '\0';
}

/* =========================
   Public API
   ========================= */

CallStack *instrument_call_stack_create(const char *instrument_name,
                                        const char *channel_group, int channel,
                                        const char *command) {
  CallStack *stack = (CallStack *)malloc(sizeof(CallStack));
  if (!stack) {
    return NULL;
  }

  copy_string(stack->instrument_name, instrument_name);
  copy_string(stack->channel_group, channel_group);
  stack->channel = channel;
  copy_string(stack->command, command);

  return stack;
}

void instrument_call_stack_free(CallStack *stack) {
  if (stack) {
    free(stack);
  }
}

const char *instrument_call_stack_get_instrument_name(const CallStack *stack) {
  if (!stack)
    return NULL;
  return stack->instrument_name;
}

const char *instrument_call_stack_get_channel_group(const CallStack *stack) {
  if (!stack)
    return NULL;
  return stack->channel_group;
}

const int instrument_call_stack_get_channel(const CallStack *stack) {
  if (!stack)
    return -1;
  return stack->channel;
}

const char *instrument_call_stack_get_command(const CallStack *stack) {
  if (!stack)
    return NULL;
  return stack->command;
}

char *instrument_call_stack_serialize(const CallStack *stack) {
  if (!stack)
    return NULL;

  // Worst-case size estimate:
  // 3 strings (64 each) + delimiters + int + null
  size_t max_size = 3 * INST_STACK_MAX_STRING_LEN + 32; // safe margin

  char *buffer = (char *)malloc(max_size);
  if (!buffer)
    return NULL;

  snprintf(buffer, max_size, "%s|%s|%d|%s", stack->instrument_name,
           stack->channel_group, stack->channel, stack->command);

  return buffer;
}

CallStack *instrument_call_stack_deserialize(const char *buffer) {
  if (!buffer)
    return NULL;

  CallStack *stack = (CallStack *)malloc(sizeof(CallStack));
  if (!stack)
    return NULL;

  memset(stack, 0, sizeof(CallStack));

  char instrument[INST_STACK_MAX_STRING_LEN] = {0};
  char group[INST_STACK_MAX_STRING_LEN] = {0};
  char command[INST_STACK_MAX_STRING_LEN] = {0};
  int channel = 0;

  int parsed = sscanf(buffer, "%63[^|]|%63[^|]|%d|%63[^|]", instrument, group,
                      &channel, command);

  if (parsed != 4) {
    free(stack);
    return NULL;
  }

  strncpy(stack->instrument_name, instrument, INST_STACK_MAX_STRING_LEN - 1);
  strncpy(stack->channel_group, group, INST_STACK_MAX_STRING_LEN - 1);
  strncpy(stack->command, command, INST_STACK_MAX_STRING_LEN - 1);

  stack->channel = channel;

  return stack;
}

CallStack *instrument_call_stack_clone(const CallStack *stack) {
  if (!stack)
    return NULL;

  return instrument_call_stack_create(
      instrument_call_stack_get_instrument_name(stack),
      instrument_call_stack_get_channel_group(stack),
      instrument_call_stack_get_channel(stack),
      instrument_call_stack_get_command(stack));
}
