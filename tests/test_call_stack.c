#include <stdarg.h>
#include <stddef.h>
#include <cmocka.h>

#include <stdlib.h>
#include <string.h>

#include "instrument-call-stack/instrument-call-stack.h"

/* =========================
   Test Helpers
   ========================= */

static const char *VALID_NAME = "instr";
static const char *VALID_GROUP = "group";
static int VALID_CHANNEL = 2;
static const char *VALID_COMMAND = "cmd";

/* Generate long string (longer than max) */
static void fill_long_string(char *buf, size_t size) {
  for (size_t i = 0; i < size - 1; i++) {
    buf[i] = 'A' + (i % 26);
  }
  buf[size - 1] = '\0';
}

/* =========================
   CREATE + FREE TESTS
   ========================= */

static void test_create_valid(void **state) {
  (void)state;

  CallStack *cs = instrument_call_stack_create(VALID_NAME, VALID_GROUP,
                                               VALID_CHANNEL, VALID_COMMAND);

  assert_non_null(cs);

  instrument_call_stack_free(cs);
}

static void test_create_null_inputs(void **state) {
  (void)state;

  CallStack *cs = instrument_call_stack_create(NULL, NULL, 0, NULL);
  assert_non_null(cs);

  assert_string_equal(instrument_call_stack_get_instrument_name(cs), "");
  assert_string_equal(instrument_call_stack_get_channel_group(cs), "");
  assert_int_equal(instrument_call_stack_get_channel(cs), 0);
  assert_string_equal(instrument_call_stack_get_command(cs), "");

  instrument_call_stack_free(cs);
}

static void test_free_null(void **state) {
  (void)state;

  /* Should not crash */
  instrument_call_stack_free(NULL);
}

/* =========================
   GETTERS TESTS
   ========================= */

static void test_getters_valid(void **state) {
  (void)state;

  CallStack *cs = instrument_call_stack_create(VALID_NAME, VALID_GROUP,
                                               VALID_CHANNEL, VALID_COMMAND);

  assert_string_equal(instrument_call_stack_get_instrument_name(cs),
                      VALID_NAME);
  assert_string_equal(instrument_call_stack_get_channel_group(cs), VALID_GROUP);
  assert_int_equal(instrument_call_stack_get_channel(cs), VALID_CHANNEL);
  assert_string_equal(instrument_call_stack_get_command(cs), VALID_COMMAND);

  instrument_call_stack_free(cs);
}

static void test_getters_null_stack(void **state) {
  (void)state;

  assert_null(instrument_call_stack_get_instrument_name(NULL));
  assert_null(instrument_call_stack_get_channel_group(NULL));
  assert_true(instrument_call_stack_get_channel(NULL) == -1);
  assert_null(instrument_call_stack_get_command(NULL));
}

/* =========================
   STRING TRUNCATION TEST
   ========================= */

static void test_truncation(void **state) {
  (void)state;

  char long_str[INST_STACK_MAX_STRING_LEN * 2];
  fill_long_string(long_str, sizeof(long_str));

  CallStack *cs =
      instrument_call_stack_create(long_str, long_str, VALID_CHANNEL, long_str);

  const char *name = instrument_call_stack_get_instrument_name(cs);

  /* Ensure null-termination */
  assert_true(strlen(name) <= INST_STACK_MAX_STRING_LEN - 1);

  /* Ensure last byte is null */
  assert_int_equal(name[INST_STACK_MAX_STRING_LEN - 1], '\0');

  instrument_call_stack_free(cs);
}

/* =========================
   SERIALIZATION TESTS
   ========================= */

static void test_serialize_valid(void **state) {
  (void)state;

  CallStack *cs = instrument_call_stack_create(VALID_NAME, VALID_GROUP,
                                               VALID_CHANNEL, VALID_COMMAND);

  char *blob = instrument_call_stack_serialize(cs);

  assert_non_null(blob);

  /* Ensure expected size behavior */
  /* We cannot directly check size, but we know it's fixed */
  free(blob);
  instrument_call_stack_free(cs);
}

static void test_serialize_null(void **state) {
  (void)state;

  const char *blob = instrument_call_stack_serialize(NULL);
  assert_null(blob);
}

/* =========================
   DESERIALIZATION TESTS
   ========================= */

static void test_deserialize_valid(void **state) {
  (void)state;

  CallStack *original = instrument_call_stack_create(
      VALID_NAME, VALID_GROUP, VALID_CHANNEL, VALID_COMMAND);

  char *blob = instrument_call_stack_serialize(original);
  assert_non_null(blob);

  CallStack *copy = instrument_call_stack_deserialize(blob);
  assert_non_null(copy);

  assert_string_equal(instrument_call_stack_get_instrument_name(copy),
                      VALID_NAME);
  assert_string_equal(instrument_call_stack_get_channel_group(copy),
                      VALID_GROUP);
  assert_int_equal(instrument_call_stack_get_channel(copy), VALID_CHANNEL);
  assert_string_equal(instrument_call_stack_get_command(copy), VALID_COMMAND);

  free(blob);
  instrument_call_stack_free(original);
  instrument_call_stack_free(copy);
}

static void test_deserialize_null(void **state) {
  (void)state;

  CallStack *cs = instrument_call_stack_deserialize(NULL);
  assert_null(cs);
}

/* =========================
   ROUNDTRIP TEST
   ========================= */

static void test_roundtrip(void **state) {
  (void)state;

  CallStack *original = instrument_call_stack_create(
      VALID_NAME, VALID_GROUP, VALID_CHANNEL, VALID_COMMAND);

  char *blob = instrument_call_stack_serialize(original);
  CallStack *copy = instrument_call_stack_deserialize(blob);

  assert_string_equal(instrument_call_stack_get_instrument_name(copy),
                      instrument_call_stack_get_instrument_name(original));

  assert_string_equal(instrument_call_stack_get_channel_group(copy),
                      instrument_call_stack_get_channel_group(original));

  assert_int_equal(instrument_call_stack_get_channel(copy),
                   instrument_call_stack_get_channel(original));

  assert_string_equal(instrument_call_stack_get_command(copy),
                      instrument_call_stack_get_command(original));

  free(blob);
  instrument_call_stack_free(original);
  instrument_call_stack_free(copy);
}

/* =========================
   CORRUPTED BUFFER TEST
   ========================= */

static void test_deserialize_corrupted(void **state) {
  (void)state;

  char *blob = malloc(INST_CALL_STACK_SERIALIZED_MAX_SIZE);
  assert_non_null(blob);

  /* Fill with garbage */
  memset(blob, 0xFF, INST_CALL_STACK_SERIALIZED_MAX_SIZE);

  CallStack *cs = instrument_call_stack_deserialize(blob);
  assert_non_null(cs);

  /* Should still be null-terminated safely */
  assert_true(instrument_call_stack_get_instrument_name(
                  cs)[INST_STACK_MAX_STRING_LEN - 1] == '\0');

  free(blob);
  instrument_call_stack_free(cs);
}

static void test_c_string_truncation(void **state) {
  (void)state;

  CallStack *cs = instrument_call_stack_create(VALID_NAME, VALID_GROUP,
                                               VALID_CHANNEL, VALID_COMMAND);

  char *blob = instrument_call_stack_serialize(cs);
  assert_non_null(blob);

  size_t len = strlen(blob);

  // strlen stops at first '\0'
  assert_true(len < INST_CALL_STACK_SERIALIZED_MAX_SIZE);

  // The first field should still match
  assert_string_equal(blob, VALID_NAME);

  free(blob);
  instrument_call_stack_free(cs);
}

/* =========================
   MAIN
   ========================= */

int main(void) {
  const struct CMUnitTest tests[] = {
      cmocka_unit_test(test_create_valid),
      cmocka_unit_test(test_create_null_inputs),
      cmocka_unit_test(test_free_null),

      cmocka_unit_test(test_getters_valid),
      cmocka_unit_test(test_getters_null_stack),

      cmocka_unit_test(test_truncation),

      cmocka_unit_test(test_serialize_valid),
      cmocka_unit_test(test_serialize_null),

      cmocka_unit_test(test_deserialize_valid),
      cmocka_unit_test(test_deserialize_null),

      cmocka_unit_test(test_roundtrip),
      cmocka_unit_test(test_deserialize_corrupted),
      cmocka_unit_test(test_c_string_truncation)};

  return cmocka_run_group_tests(tests, NULL, NULL);
}
