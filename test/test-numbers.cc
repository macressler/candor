#include "test.h"

TEST_START("numbers test")
  // Basics
  FUN_TEST("return 1.2345678", {
    assert(HValue::As<HNumber>(result)->value() == 1.2345678);
  })

  FUN_TEST("return 1.5 + 1.5", {
    assert(HValue::As<HNumber>(result)->value() == 3);
  })

  FUN_TEST("return 3.5 - 1.5", {
    assert(HValue::As<HNumber>(result)->value() == 2);
  })

  FUN_TEST("return 1.5 * 1.5", {
    assert(HValue::As<HNumber>(result)->value() == 2.25);
  })

  FUN_TEST("return 7.0 / 2.0", {
    assert(HValue::As<HNumber>(result)->value() == 3.5);
  })

  // Conversion to unboxed
  FUN_TEST("return 1.5 | 3.5", {
    assert(HValue::As<HNumber>(result)->value() == 3);
  })

  FUN_TEST("return 2.5 & 3.5", {
    assert(HValue::As<HNumber>(result)->value() == 2);
  })

  FUN_TEST("return 1.5 ^ 3.5", {
    assert(HValue::As<HNumber>(result)->value() == 2);
  })

  // Logic
  FUN_TEST("return 1 < 2", {
    assert(HValue::As<HBoolean>(result)->is_true());
  })

  FUN_TEST("return 1 == 1", {
    assert(HValue::As<HBoolean>(result)->is_true());
  })

  FUN_TEST("return 1.0 < 2.5", {
    assert(HValue::As<HBoolean>(result)->is_true());
  })

  FUN_TEST("return 1.0 > 2.5", {
    assert(HValue::As<HBoolean>(result)->is_false());
  })

  FUN_TEST("return 1.0 >= 1.0", {
    assert(HValue::As<HBoolean>(result)->is_true());
  })

  FUN_TEST("return 1.0 == 1.0", {
    assert(HValue::As<HBoolean>(result)->is_true());
  })

  FUN_TEST("return 1.0 === 1.0", {
    assert(HValue::As<HBoolean>(result)->is_true());
  })

  // Negative (unboxed)
  FUN_TEST("return 0 - 1", {
    assert(HValue::As<HNumber>(result)->value() == -1);
  })

  FUN_TEST("return 0 - 1 - 2", {
    assert(HValue::As<HNumber>(result)->value() == -3);
  })

  // Conversion to heap
  FUN_TEST("return 5 + 0.5", {
    assert(HValue::As<HNumber>(result)->value() == 5.5);
  })

  FUN_TEST("return 0.5 + 5", {
    assert(HValue::As<HNumber>(result)->value() == 5.5);
  })

  // Conversion on overflow
  FUN_TEST("return 4611686018427387904 * 1000000", {
    assert(HValue::As<HNumber>(result)->value() == 4611686018427387904000000.0);
  })

  FUN_TEST("return 4611686018427387904 + 4611686018427387904 + "
           "4611686018427387904 + 4611686018427387904", {
    assert(HValue::As<HNumber>(result)->value() == 18446744073709551616.0);
  })

  FUN_TEST("return 0 - 4611686018427387904 - 4611686018427387904 - "
           "4611686018427387904 - 4611686018427387904", {
    assert(HValue::As<HNumber>(result)->value() == -18446744073709551616.0);
  })
TEST_END("numbers test")