// Copyright 2004-present Facebook. All Rights Reserved.

#include "no_rtti.h"

#include <stdexcept>

namespace nortti {
struct NoRttiException : std::exception {
};

void throwException() {
  throw NoRttiException();
}
}
