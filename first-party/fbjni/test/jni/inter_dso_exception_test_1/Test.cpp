// Copyright 2004-present Facebook. All Rights Reserved.

#include "Test.h"

#include <fbjni/fbjni.h>

void inter_dso_exception_test_1() {
  throw facebook::jni::JniException();
}
