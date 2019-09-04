// Copyright 2004-present Facebook. All Rights Reserved.

#pragma once

#include <fbjni/detail/Log.h>

#define EXPECT(X)                         \
  do {                                    \
    if (!(X)) {                           \
      FBJNI_LOGE("[%s:%d] Expectation failed: %s", __FILE__, __LINE__,  #X);  \
      return JNI_FALSE;                   \
    }                                     \
  } while (false)

