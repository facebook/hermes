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

#include "Exceptions.h"
#include "Hybrid.h"

namespace facebook {
namespace jni {

namespace detail {

#ifdef __i386__
// X86 ABI forces 16 byte stack allignment on calls. Unfortunately
// sometimes Dalvik chooses not to obey the ABI:
// - https://code.google.com/p/android/issues/detail?id=61012
// - https://android.googlesource.com/platform/ndk/+/81696d2%5E!/
// Therefore, we tell the compiler to re-align the stack on entry
// to our JNI functions.
#define JNI_ENTRY_POINT __attribute__((force_align_arg_pointer))
#else
#define JNI_ENTRY_POINT
#endif

template <typename R>
struct CreateDefault {
  static R create() {
    return R{};
  }
};

template <>
struct CreateDefault<void> {
  static void create() {}
};

template <typename R>
using Converter = Convert<typename std::decay<R>::type>;

template <typename F, typename R, typename C, typename... Args>
struct CallWithJniConversions {
  static typename Converter<R>::jniType call(JniType<C> obj, typename Converter<Args>::jniType... args, F func) {
    return Converter<R>::toJniRet(func(obj, Converter<Args>::fromJni(args)...));
  }
};

template <typename F, typename C, typename... Args>
struct CallWithJniConversions<F, void, C, Args...> {
  static void call(JniType<C> obj, typename Converter<Args>::jniType... args, F func) {
    func(obj, Converter<Args>::fromJni(args)...);
  }
};

// registration wrapper for legacy JNI-style functions
template<typename F, F func, typename C, typename R, typename... Args>
struct BareJniWrapper {
  JNI_ENTRY_POINT static R call(JNIEnv* env, jobject obj, Args... args) {
    detail::JniEnvCacher jec(env);
    try {
      return (*func)(env, static_cast<JniType<C>>(obj), args...);
    } catch (...) {
      translatePendingCppExceptionToJavaException();
      return CreateDefault<R>::create();
    }
  }
};

// registration wrappers for functions, with autoconversion of arguments.
template<typename F, typename C, typename R, typename... Args>
struct FunctionWrapper {
  using jniRet = typename Converter<R>::jniType;
  static jniRet call(JNIEnv* env, jobject obj, typename Converter<Args>::jniType... args, F funcPtr) {
    detail::JniEnvCacher jec(env);
    try {
      return CallWithJniConversions<F, R, JniType<C>, Args...>::call(
          static_cast<JniType<C>>(obj), args..., funcPtr);
    } catch (...) {
      translatePendingCppExceptionToJavaException();
      return CreateDefault<jniRet>::create();
    }
  }
};

// registration wrappers for functions, with autoconversion of arguments.
// This is a separate class from FunctionWrapper because
// MethodWrapper::call does not want FunctionWrapper::call to be a
// JNI_ENTRY_POINT and thus not inlinable. However, we still want a
// JNI_ENTRY_POINT for top-level functions.
template<typename F, F func, typename C, typename R, typename... Args>
struct FunctionWrapperWithJniEntryPoint {
  using jniRet = typename FunctionWrapper<F, C, R, Args...>::jniRet;
  JNI_ENTRY_POINT static jniRet call(JNIEnv* env, jobject obj, typename Converter<Args>::jniType... args) {
    return FunctionWrapper<F, C, R, Args...>::call(env, obj, args..., func);
  }
};

// registration wrappers for non-static methods, with autoconvertion of arguments.
template<typename M, M method, typename C, typename R, typename... Args>
struct MethodWrapper {
  using jhybrid = typename C::jhybridobject;
  static R dispatch(alias_ref<jhybrid> ref, Args&&... args) {
    try {
      // This is usually a noop, but if the hybrid object is a
      // base class of other classes which register JNI methods,
      // this will get the right type for the registered method.
      auto cobj = static_cast<C*>(ref->cthis());
      return (cobj->*method)(std::forward<Args>(args)...);
    } catch (const std::exception& ex) {
      C::mapException(ex);
      throw;
    }
  }

  JNI_ENTRY_POINT static typename Converter<R>::jniType call(
      JNIEnv* env, jobject obj, typename Converter<Args>::jniType... args) {
    return FunctionWrapper<R(*)(alias_ref<jhybrid>, Args&&...), jhybrid, R, Args...>::call(env, obj, args..., dispatch);
  }
};

template<typename F, F func, typename C, typename R, typename... Args>
constexpr inline void* exceptionWrapJNIMethod(R (*)(JNIEnv*, C, Args... args)) {
  // This intentionally erases the real type; JNI will do it anyway
  return (void*)(&(BareJniWrapper<F, func, C, R, Args...>::call));
}

template<typename F, F func, typename C, typename R, typename... Args>
constexpr inline void* exceptionWrapJNIMethod(R (*)(alias_ref<C>, Args... args)) {
  // This intentionally erases the real type; JNI will do it anyway
  return (void*)(&(FunctionWrapperWithJniEntryPoint<F, func, C, R, Args...>::call));
}

template<typename M, M method, typename C, typename R, typename... Args>
constexpr inline void* exceptionWrapJNIMethod(R (C::*method0)(Args... args)) {
  (void)method0;
  // This intentionally erases the real type; JNI will do it anyway
  return (void*)(&(MethodWrapper<M, method, C, R, Args...>::call));
}

template<typename R, typename C, typename... Args>
inline constexpr const auto& /* detail::SimpleFixedString<_> */ makeDescriptor(R (*)(JNIEnv*, C, Args... args)) {
  return jmethod_traits<R(Args...)>::kDescriptor;
}

template<typename R, typename C, typename... Args>
inline constexpr const auto& /* detail::SimpleFixedString<_> */ makeDescriptor(R (*)(alias_ref<C>, Args... args)) {
  return jmethod_traits_from_cxx<R(Args...)>::kDescriptor;
}

template<typename R, typename C, typename... Args>
inline constexpr const auto& /* detail::SimpleFixedString<_> */ makeDescriptor(R (C::*)(Args... args)) {
  return jmethod_traits_from_cxx<R(Args...)>::kDescriptor;
}

template<typename R, typename ...Args>
template<R(*func)(Args...)>
JNI_ENTRY_POINT R CriticalMethod<R(*)(Args...)>::call(alias_ref<jclass>, Args... args) noexcept {
  static_assert(
    IsJniPrimitive<R>() || std::is_void<R>(),
    "Critical Native Methods may only return primitive JNI types, or void.");
  static_assert(
    AreJniPrimitives<Args...>(),
    "Critical Native Methods may only use primitive JNI types as parameters");

  return func(std::forward<Args>(args)...);
}

template<typename R, typename ...Args>
template<R(*func)(Args...)>
inline constexpr auto CriticalMethod<R(*)(Args...)>::desc() {
  return makeDescriptor(call<func>);
}

}

}}
