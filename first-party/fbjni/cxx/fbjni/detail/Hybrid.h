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

#pragma once

#include <memory>
#include <type_traits>

#include <fbjni/detail/SimpleFixedString.h>

#include "CoreClasses.h"

namespace facebook {
namespace jni {

namespace detail {

class BaseHybridClass {
public:
  virtual ~BaseHybridClass() {}
};

struct HybridData : public JavaClass<HybridData> {
  constexpr static auto kJavaDescriptor = "Lcom/facebook/jni/HybridData;";
  static local_ref<HybridData> create();
};

class HybridDestructor : public JavaClass<HybridDestructor> {
  public:
    static auto constexpr kJavaDescriptor = "Lcom/facebook/jni/HybridData$Destructor;";

  detail::BaseHybridClass* getNativePointer() const;

  void setNativePointer(std::unique_ptr<detail::BaseHybridClass> new_value);
};

template<typename T>
detail::BaseHybridClass* getNativePointer(const T* t) {
  return getHolder(t)->getNativePointer();
}

template<typename T>
void setNativePointer(const T* t, std::unique_ptr<detail::BaseHybridClass> new_value) {
  getHolder(t)->setNativePointer(std::move(new_value));
}

// Save space: use unified getHolder implementation.
template<typename T, typename Alloc>
void setNativePointer(basic_strong_ref<T, Alloc> t, std::unique_ptr<detail::BaseHybridClass> new_value) {
  getHolder(&*t)->setNativePointer(std::move(new_value));
}

// Save space: use unified getHolder implementation.
template<typename T>
void setNativePointer(alias_ref<T> t, std::unique_ptr<detail::BaseHybridClass> new_value) {
  getHolder(&*t)->setNativePointer(std::move(new_value));
}

// Inline rather than in cpp file so that consumers can call into
// their own copy directly rather than into the DSO containing fbjni,
// saving space for the symbol table.
inline JField<HybridDestructor::javaobject> getDestructorField(const local_ref<JClass>& c) {
  return c->template getField<HybridDestructor::javaobject>("mDestructor");
}

template<typename T>
local_ref<HybridDestructor::javaobject> getHolder(const T* t) {
  static auto holderField = getDestructorField(t->getClass());
  return t->getFieldValue(holderField);
}

// JavaClass for HybridClassBase
struct HybridClassBase : public JavaClass<HybridClassBase> {
  constexpr static auto kJavaDescriptor = "Lcom/facebook/jni/HybridClassBase;";

  static bool isHybridClassBase(alias_ref<jclass> jclass) {
    return HybridClassBase::javaClassStatic()->isAssignableFrom(jclass);
  }
};

template <typename Base, typename Enabled = void>
struct HybridTraits {
  // This static assert should actually always fail if we don't use one of the
  // specializations below.
  static_assert(
      std::is_base_of<JObject, Base>::value ||
      std::is_base_of<BaseHybridClass, Base>::value,
      "The base of a HybridClass must be either another HybridClass or derived from JObject.");
};

template <>
struct HybridTraits<BaseHybridClass> {
 using CxxBase = BaseHybridClass;
 using JavaBase = JObject;
};

template <typename Base>
struct HybridTraits<
    Base,
    typename std::enable_if<std::is_base_of<BaseHybridClass, Base>::value>::type> {
 using CxxBase = Base;
 using JavaBase = typename Base::JavaPart;
};

template <typename Base>
struct HybridTraits<
    Base,
    typename std::enable_if<std::is_base_of<JObject, Base>::value>::type> {
 using CxxBase = BaseHybridClass;
 using JavaBase = Base;
};

// convert to HybridClass* from jhybridobject
template <typename T>
struct Convert<
  T, typename std::enable_if<
    std::is_base_of<BaseHybridClass, typename std::remove_pointer<T>::type>::value>::type> {
  typedef typename std::remove_pointer<T>::type::jhybridobject jniType;
  static T fromJni(jniType t) {
    if (t == nullptr) {
      return nullptr;
    }
    return wrap_alias(t)->cthis();
  }
  // There is no automatic return conversion for objects.
};

template<typename T>
struct RefReprType<T, typename std::enable_if<std::is_base_of<BaseHybridClass, T>::value, void>::type> {
  static_assert(std::is_same<T, void>::value,
      "HybridFoo (where HybridFoo derives from HybridClass<HybridFoo>) is not supported in this context. "
      "For an xxx_ref<HybridFoo>, you may want: xxx_ref<HybridFoo::javaobject> or HybridFoo*.");
  using Repr = T;
};


}

template <typename T, typename Base = detail::BaseHybridClass>
class HybridClass : public detail::HybridTraits<Base>::CxxBase {
public:
  struct JavaPart : JavaClass<JavaPart, typename detail::HybridTraits<Base>::JavaBase> {
    // At this point, T is incomplete, and so we cannot access
    // T::kJavaDescriptor directly. jtype_traits support this escape hatch for
    // such a case.
    static constexpr const char* kJavaDescriptor = nullptr;
    static constexpr auto /* detail::SimpleFixedString<_> */ get_instantiated_java_descriptor();
    static constexpr auto /* detail::SimpleFixedString<_> */ get_instantiated_base_name();

    using HybridType = T;

    // This will reach into the java object and extract the C++ instance from
    // the mHybridData and return it.
    T* cthis() const;

    friend class HybridClass;
    friend T;
  };

  using jhybridobject = typename JavaPart::javaobject;
  using javaobject = typename JavaPart::javaobject;
  typedef detail::HybridData::javaobject jhybriddata;

  static alias_ref<JClass> javaClassStatic() {
    return JavaPart::javaClassStatic();
  }

  static local_ref<JClass> javaClassLocal() {
    std::string className(T::kJavaDescriptor + 1, strlen(T::kJavaDescriptor) - 2);
    return findClassLocal(className.c_str());
  }

protected:
  typedef HybridClass HybridBase;

  // This ensures that a C++ hybrid part cannot be created on its own
  // by default.  If a hybrid wants to enable this, it can provide its
  // own public ctor, or change the accessibility of this to public.
  using detail::HybridTraits<Base>::CxxBase::CxxBase;

  static void registerHybrid(std::initializer_list<JNINativeMethod> methods) {
    javaClassLocal()->registerNatives(methods);
  }

  static local_ref<detail::HybridData> makeHybridData(std::unique_ptr<T> cxxPart) {
    auto hybridData = detail::HybridData::create();
    setNativePointer(hybridData, std::move(cxxPart));
    return hybridData;
  }

  template <typename... Args>
  static local_ref<detail::HybridData> makeCxxInstance(Args&&... args) {
    return makeHybridData(std::unique_ptr<T>(new T(std::forward<Args>(args)...)));
  }

  template <typename... Args>
  static void setCxxInstance(alias_ref<jhybridobject> o, Args&&... args) {
    setNativePointer(o, std::unique_ptr<T>(new T(std::forward<Args>(args)...)));
  }

public:
  // Factory method for creating a hybrid object where the arguments
  // are used to initialize the C++ part directly without passing them
  // through java.  This method requires the Java part to have a ctor
  // which takes a HybridData, and for the C++ part to have a ctor
  // compatible with the arguments passed here.  For safety, the ctor
  // can be private, and the hybrid declared a friend of its base, so
  // the hybrid can only be created from here.
  //
  // Exception behavior: This can throw an exception if creating the
  // C++ object fails, or any JNI methods throw.
  template <typename... Args>
  static local_ref<JavaPart> newObjectCxxArgs(Args&&... args) {
    static bool isHybrid = detail::HybridClassBase::isHybridClassBase(javaClassStatic());
    auto cxxPart = std::unique_ptr<T>(new T(std::forward<Args>(args)...));

    local_ref<JavaPart> result;
    if (isHybrid) {
      result = JavaPart::newInstance();
      setNativePointer(result, std::move(cxxPart));
    }
    else {
      auto hybridData = makeHybridData(std::move(cxxPart));
      result = JavaPart::newInstance(hybridData);
    }

    return result;
  }

 // TODO? Create reusable interface for Allocatable classes and use it to
  // strengthen type-checking (and possibly provide a default
  // implementation of allocate().)
  template <typename... Args>
  static local_ref<jhybridobject> allocateWithCxxArgs(Args&&... args) {
    auto hybridData = makeCxxInstance(std::forward<Args>(args)...);
    static auto allocateMethod =
        javaClassStatic()->template getStaticMethod<jhybridobject(jhybriddata)>("allocate");
    return allocateMethod(javaClassStatic(), hybridData.get());
  }

  // Factory method for creating a hybrid object where the arguments
  // are passed to the java ctor.
  template <typename... Args>
  static local_ref<JavaPart> newObjectJavaArgs(Args&&... args) {
    return JavaPart::newInstance(std::move(args)...);
  }

  // If a hybrid class throws an exception which derives from
  // std::exception, it will be passed to mapException on the hybrid
  // class, or nearest ancestor.  This allows boilerplate exception
  // translation code (for example, calling throwNewJavaException on a
  // particular java class) to be hoisted to a common function.  If
  // mapException returns, then the std::exception will be translated
  // to Java.
  static void mapException(const std::exception& ex) {
    (void)ex;
  }
};

[[noreturn]] inline void throwNPE() {
  throwNewJavaException("java/lang/NullPointerException", "java.lang.NullPointerException");
}

// Detect whether the given class is a hybrid. If not, grab its
// mHybridData field.
// Returns a null field if the class is a hybrid, the ID of the mHybridData field if not.
template<typename T, typename B>
inline JField<detail::HybridData::javaobject> detectHybrid(alias_ref<jclass> jclass) {
  bool isHybrid = detail::HybridClassBase::isHybridClassBase(jclass);
  if (isHybrid) {
    return JField<detail::HybridData::javaobject>(nullptr);
  } else {
    auto result = HybridClass<T, B>::JavaPart::javaClassStatic()->template getField<detail::HybridData::javaobject>("mHybridData");
    if (!result) {
      throwNPE();
    }
    return result;
  }
}

inline detail::BaseHybridClass* getHybridDataFromField(const JObject* self, const JField<detail::HybridData::javaobject>& field) {
  const bool isHybrid = !field;
  if (isHybrid) {
    return getNativePointer(self);
  } else {
    auto hybridData = self->getFieldValue(field);
    if (!hybridData) {
      throwNPE();
    }
    return getNativePointer(&*hybridData);
  }
}

template <typename T, typename B>
inline T* HybridClass<T, B>::JavaPart::cthis() const {
  detail::BaseHybridClass* result = nullptr;
  static const auto hybridDataField = detectHybrid<T, B>(this->getClass());
  result = getHybridDataFromField(this, hybridDataField);

  // I'd like to use dynamic_cast here, but -fno-rtti is the default.
  return static_cast<T*>(result);
};

template <typename T, typename B>
constexpr auto /* detail::SimpleFixedString<_> */ HybridClass<T, B>::JavaPart::get_instantiated_java_descriptor() {
  constexpr auto len = detail::constexpr_strlen(T::kJavaDescriptor);
  return detail::SimpleFixedString<len>(T::kJavaDescriptor, len);
}

template <typename T, typename B>
/* static */ constexpr auto /* detail::SimpleFixedString<_> */ HybridClass<T, B>::JavaPart::get_instantiated_base_name() {
  constexpr auto len = detail::constexpr_strlen(T::kJavaDescriptor);
  return detail::SimpleFixedString<len>(T::kJavaDescriptor + 1, len - 2);
}

// Given a *_ref object which refers to a hybrid class, this will reach inside
// of it, find the mHybridData, extract the C++ instance pointer, cast it to
// the appropriate type, and return it.
template <typename T>
inline auto cthis(T jthis) -> decltype(jthis->cthis()) {
  return jthis->cthis();
}

void HybridDataOnLoad();

}
}
