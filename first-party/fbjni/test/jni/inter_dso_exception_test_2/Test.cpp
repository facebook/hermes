/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "Test.h"
#include <inter_dso_exception_test_1/Test.h>
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
