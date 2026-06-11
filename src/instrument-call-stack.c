#include "instrument-call-stack/instrument-call-stack.h"

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

  /* Allocate fixed-size blob */
  char *buffer = (char *)malloc(INST_CALL_STACK_SERIALIZED_MAX_SIZE);
  if (!buffer) {
    return NULL;
  }

  memcpy(buffer, stack, sizeof(CallStack));
  return buffer;
}

CallStack *instrument_call_stack_deserialize(const char *buffer) {
  if (!buffer)
    return NULL;

  CallStack *stack = (CallStack *)malloc(sizeof(CallStack));
  if (!stack) {
    return NULL;
  }

  memcpy(stack, buffer, sizeof(CallStack));

  /* Enforce null termination defensively */
  stack->instrument_name[INST_STACK_MAX_STRING_LEN - 1] = '\0';
  stack->channel_group[INST_STACK_MAX_STRING_LEN - 1] = '\0';
  stack->command[INST_STACK_MAX_STRING_LEN - 1] = '\0';

  return stack;
}
