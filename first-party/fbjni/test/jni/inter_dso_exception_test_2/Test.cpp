// Copyright 2004-present Facebook. All Rights Reserved.

#include "Test.h"
#include <jni/tests/inter_dso_exception_test_1/Test.h>
#include <fbjni/fbjni.h>

void inter_dso_exception_test_2a() {
  inter_dso_exception_test_1();
}

bool inter_dso_exception_test_2b() {
  try {
    inter_dso_exception_test_1();
    return false;
  } catch(const facebook::jni::JniException& ex) {
    return true;
  } catch(...) {
    return false;
  }
}
