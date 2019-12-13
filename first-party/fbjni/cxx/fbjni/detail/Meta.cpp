/**
 * Copyright 2018-present, Facebook, Inc.
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

// These two includes must be before Meta.h or we have circular include issues.
#include "CoreClasses.h"
#include "TypeTraits.h"
#include "Meta.h"

namespace facebook {
namespace jni {

/* static */ constexpr detail::SimpleFixedString<1> jtype_traits<void>::kDescriptor;

#define DEFINE_CONSTANTS_FOR_FIELD_AND_ARRAY_TRAIT(TYPE, DSC)           \
  /* static */ constexpr decltype(detail::makeSimpleFixedString(#DSC)) jtype_traits<TYPE>::kDescriptor; \
  /* static */ constexpr decltype(jtype_traits<TYPE>::kDescriptor) jtype_traits<TYPE>::kBaseName; \
  /* static */ constexpr decltype(detail::makeSimpleFixedString("[" #DSC)) jtype_traits<TYPE ## Array>::kDescriptor; \
  /* static */ constexpr decltype(jtype_traits<TYPE ## Array>::kDescriptor) jtype_traits<TYPE ## Array>::kBaseName;

DEFINE_CONSTANTS_FOR_FIELD_AND_ARRAY_TRAIT(jboolean, Z)
DEFINE_CONSTANTS_FOR_FIELD_AND_ARRAY_TRAIT(jbyte,    B)
DEFINE_CONSTANTS_FOR_FIELD_AND_ARRAY_TRAIT(jchar,    C)
DEFINE_CONSTANTS_FOR_FIELD_AND_ARRAY_TRAIT(jshort,   S)
DEFINE_CONSTANTS_FOR_FIELD_AND_ARRAY_TRAIT(jint,     I)
DEFINE_CONSTANTS_FOR_FIELD_AND_ARRAY_TRAIT(jlong,    J)
DEFINE_CONSTANTS_FOR_FIELD_AND_ARRAY_TRAIT(jfloat,   F)
DEFINE_CONSTANTS_FOR_FIELD_AND_ARRAY_TRAIT(jdouble,  D)

} // namespace jni
} // namespace facebook
