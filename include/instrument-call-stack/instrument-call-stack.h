#pragma once

/** Maximum length (including null terminator) for each string field */
#define INST_STACK_MAX_STRING_LEN 64
#define INST_CALL_STACK_SERIALIZED_MAX_SIZE (4 * INST_STACK_MAX_STRING_LEN)

#ifdef __cplusplus
extern "C" {
#endif

#include "instrument-call-stack/instrument-call-stack-export.h"

/**
 * @brief Opaque handle representing an instrument call stack.
 *
 * @details The internal structure is hidden from users of the API.
 * Instances must be created and destroyed using the provided API functions.
 */
typedef struct CallStack CallStack;

/**
 * @brief Creates a new instrument call stack instance.
 *
 * @param instrument_name Name of the instrument (null-terminated string).
 * @param channel_group   Channel group identifier.
 * @param channel         Channel identifier.
 * @param command         Command name or identifier.
 *
 * @return Pointer to a newly allocated CallStack instance, or NULL on failure.
 *
 * @note All input strings are copied internally and truncated if they exceed
 *       INST_STACK_MAX_STRING_LEN - 1 characters.
 */
INSTRUMENT_CALL_STACK_EXPORT CallStack *
instrument_call_stack_create(const char *instrument_name,
                             const char *channel_group, int channel,
                             const char *command);

/**
 * @brief Frees a CallStack instance.
 *
 * @param stack Pointer to the CallStack to free. Safe to pass NULL.
 */
INSTRUMENT_CALL_STACK_EXPORT void instrument_call_stack_free(CallStack *stack);

/**
 * @brief Retrieves the instrument name.
 *
 * @param stack CallStack instance.
 * @return Pointer to an internal null-terminated string.
 *
 * @warning The returned pointer is owned by the CallStack and must NOT be
 * modified or freed by the caller.
 */
INSTRUMENT_CALL_STACK_EXPORT const char *
instrument_call_stack_get_instrument_name(const CallStack *stack);

/**
 * @brief Retrieves the channel group.
 */
INSTRUMENT_CALL_STACK_EXPORT const char *
instrument_call_stack_get_channel_group(const CallStack *stack);

/**
 * @brief Retrieves the channel.
 */
INSTRUMENT_CALL_STACK_EXPORT const int
instrument_call_stack_get_channel(const CallStack *stack);

/**
 * @brief Retrieves the command.
 */
INSTRUMENT_CALL_STACK_EXPORT const char *
instrument_call_stack_get_command(const CallStack *stack);

/**
 * @brief Serializes the call stack into a portable representation.
 *
 * @details The returned buffer contains a self-contained representation of the
 * CallStack that can be transmitted or stored and later reconstructed using
 * instrument_call_stack_deserialize().
 *
 * @param stack CallStack instance.
 *
 * @return Pointer to a serialized buffer (null-terminated or binary-safe
 * depending on implementation).
 *
 * @note The returned memory is owned by the library and must not be modified.
 *       If dynamic allocation is used, a matching free API should also be
 * provided.
 */
INSTRUMENT_CALL_STACK_EXPORT char *
instrument_call_stack_serialize(const CallStack *stack);

/**
 * @brief Deserializes a CallStack from a serialized representation.
 *
 * @param buffer Serialized data.
 *
 * @return Pointer to a newly allocated CallStack instance, or NULL on failure.
 */
INSTRUMENT_CALL_STACK_EXPORT CallStack *
instrument_call_stack_deserialize(const char *buffer);

#ifdef __cplusplus
}
#endif
