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

#include <fbjni/fbjni.h>

#ifdef __ANDROID__
#include <sys/prctl.h>
#endif // __ANDROID__

#include <vector>
#include <functional>
#ifndef _WIN32
#include <pthread.h>
#else
#include <windows.h>
#endif

namespace facebook {
namespace jni {

namespace {

JavaVM* g_vm = nullptr;

struct EnvironmentInitializer {
  EnvironmentInitializer(JavaVM* vm) {
      FBJNI_ASSERT(!g_vm);
      FBJNI_ASSERT(vm);
      g_vm = vm;
  }
};

int getEnv(JNIEnv** env) {
  FBJNI_ASSERT(g_vm);
  // g_vm->GetEnv() might not clear the env* in failure cases.
  *env = nullptr;
  jint ret = g_vm->GetEnv((void**)env, JNI_VERSION_1_6);
  // Other possibilites are that JNI_VERSION_1_6 is invalid, or some
  // unknown return was received.
  FBJNI_ASSERT(ret == JNI_OK || ret == JNI_EDETACHED);
  return ret;
}

// Some jni.h define the first arg to AttachCurrentThread as void**,
// and some as JNIEnv**.  This hack allows both to work.

template <typename>
struct AttachTraits;

template <>
struct AttachTraits<jint(JavaVM::*)(JNIEnv**, void*)> {
  using EnvType = JNIEnv*;
};

template <>
struct AttachTraits<jint(JavaVM::*)(void**, void*)> {
  using EnvType = void*;
};

std::string getThreadName() {
#ifdef _WIN32
  return "";
#else // _WIN32
  constexpr int kMaxThreadNameSize = 100;
  int ret = 0;
  char threadName[kMaxThreadNameSize];
#ifdef __ANDROID__
  ret = prctl(PR_GET_NAME, threadName);
#else
  ret = pthread_getname_np(pthread_self(), threadName, sizeof(threadName));
#endif
  if (ret != 0) {
    return "";
  }
  return threadName;
#endif // _WIN32
}

JNIEnv* attachCurrentThread() {
  JavaVMAttachArgs args{JNI_VERSION_1_6, nullptr, nullptr};

  const auto threadName = getThreadName();
  std::vector<char> nameBuf;
  if (threadName.size()) {
    const char* cName = threadName.c_str();
    nameBuf.assign(cName, cName + threadName.size() + 1);
    args.name = nameBuf.data();
  }

  using AttachEnvType =
      typename AttachTraits<decltype(&JavaVM::AttachCurrentThread)>::EnvType;
  AttachEnvType env;
  auto result = g_vm->AttachCurrentThread(&env, &args);
  FBJNI_ASSERT(result == JNI_OK);
  return reinterpret_cast<JNIEnv*>(env);
}

}

/* static */
void Environment::initialize(JavaVM* vm) {
  static EnvironmentInitializer init(vm);
}

namespace {

#ifndef _WIN32
typedef pthread_key_t tls_key_t;
#else
typedef DWORD tls_key_t;
#endif

tls_key_t makeKey() {
  tls_key_t key;
#ifndef _WIN32
  int ret = pthread_key_create(&key, nullptr);
  if (ret != 0) {
    FBJNI_LOGF("pthread_key_create failed: %d", ret);
  }
#else
  key = TlsAlloc();
  if (key == TLS_OUT_OF_INDEXES) {
    FBJNI_LOGF("TlsAlloc failed");
  }
#endif
  return key;
}

tls_key_t getTLKey() {
  static tls_key_t key = makeKey();
  return key;
}

inline detail::TLData* getTLData(tls_key_t key) {
#ifndef _WIN32
  void* raw_data = pthread_getspecific(key);
#else
  LPVOID raw_data = TlsGetValue(key);
  // TODO: Maybe check for errors here?
#endif
  return reinterpret_cast<detail::TLData*>(raw_data);
}

inline void setTLData(tls_key_t key, detail::TLData* data) {
#ifndef _WIN32
  int ret = pthread_setspecific(key, data);
  if (ret != 0) {
    (void) ret;
    FBJNI_LOGF("pthread_setspecific failed: %d", ret);
  }
#else
  BOOL ret = TlsSetValue(key, data);
  if (!ret) {
    FBJNI_LOGF("TlsSetValue failed: %d", GetLastError());
  }
#endif
}

// This returns non-nullptr iff the env was cached from java.  So it
// can return nullptr for a thread which has been registered.
inline JNIEnv* cachedOrNull() {
  detail::TLData* pdata = getTLData(getTLKey());
  return (pdata ? pdata->env : nullptr);
}

}

namespace detail {

// This will return a cached env if there is one, or get one from JNI
// if the thread has already been attached some other way.  If it
// returns nullptr, then the thread has never been registered, or the
// VM has never been set up for fbjni.

JNIEnv* currentOrNull() {
  if (!g_vm) {
    return nullptr;
  }

  detail::TLData* pdata = getTLData(getTLKey());
  if (pdata && pdata->env) {
    return pdata->env;
  }

  JNIEnv* env;
  if (getEnv(&env) != JNI_OK) {
    // If there's a ThreadScope on the stack, we should have gotten a
    // JNIEnv and not ended up here.
    FBJNI_ASSERT(!pdata || !pdata->attached);
  }
  return env;
}

// To understand JniEnvCacher and ThreadScope, it is helpful to
// realize that if a flagged JniEnvCacher is on the stack, then a
// flagged ThreadScope cannot be after it.  If a flagged ThreadCacher
// is on the stack, then a JniEnvCacher *can* be after it.  So,
// ThreadScope's setup and teardown can both assume they are the
// first/last interesting objects, but this is not true of
// JniEnvCacher.

JniEnvCacher::JniEnvCacher(JNIEnv* env)
  : thisCached_(false)
{
  FBJNI_ASSERT(env);

  tls_key_t key = getTLKey();
  detail::TLData* pdata = getTLData(key);
  if (pdata && pdata->env) {
    return;
  }

  if (!pdata) {
    pdata = &data_;
    setTLData(key, pdata);
    pdata->attached = false;
  } else {
    FBJNI_ASSERT(!pdata->env);
  }

  pdata->env = env;

  thisCached_ = true;
}

JniEnvCacher::~JniEnvCacher() {
  if (!thisCached_) {
    return;
  }

  tls_key_t key = getTLKey();
  TLData* pdata = getTLData(key);
  FBJNI_ASSERT(pdata);
  FBJNI_ASSERT(pdata->env != nullptr);
  pdata->env = nullptr;
  if (!pdata->attached) {
    setTLData(key, nullptr);
  }
}

}

ThreadScope::ThreadScope()
  : thisAttached_(false)
{
  if (g_vm == nullptr) {
    throw std::runtime_error("fbjni is uninitialized; no thread can be attached.");
  }

  JNIEnv* env;

  // Check if the thread is attached somehow.
  auto result = getEnv(&env);
  if (result == JNI_OK) {
    return;
  }

  // At this point, it appears there's no thread attached and no env is
  // cached, or we would have returned already.  So there better not
  // be TLData.

  tls_key_t key = getTLKey();
  detail::TLData* pdata = getTLData(key);
  FBJNI_ASSERT(pdata == nullptr);
  setTLData(key, &data_);

  attachCurrentThread();

  data_.env = nullptr;
  data_.attached = true;

  thisAttached_ = true;
}

ThreadScope::~ThreadScope() {
  if (!thisAttached_) {
    return;
  }

  tls_key_t key = getTLKey();
  detail::TLData* pdata = getTLData(key);
  FBJNI_ASSERT(pdata);
  FBJNI_ASSERT(pdata->env == nullptr);
  FBJNI_ASSERT(pdata->attached);
  FBJNI_ASSERT(g_vm);
  g_vm->DetachCurrentThread();
  setTLData(key, nullptr);
}

/* static */
JNIEnv* Environment::current() {
  FBJNI_ASSERT(g_vm);
  JNIEnv* env = detail::currentOrNull();
  if (env == nullptr) {
    throw std::runtime_error("Unable to retrieve jni environment. Is the thread attached?");
  }
  return env;
}

/* static */
JNIEnv* Environment::ensureCurrentThreadIsAttached() {
  FBJNI_ASSERT(g_vm);
  JNIEnv* env = detail::currentOrNull();
  if (env == nullptr) {
    env = attachCurrentThread();
    FBJNI_ASSERT(env);
  }
  return env;
}

/* static */
bool Environment::isGlobalJvmAvailable() {
  return g_vm != nullptr;
}

namespace {
struct JThreadScopeSupport : JavaClass<JThreadScopeSupport> {
  static auto constexpr kJavaDescriptor = "Lcom/facebook/jni/ThreadScopeSupport;";

  // These reinterpret_casts are a totally dangerous pattern. Don't use them. Use HybridData instead.
  static void runStdFunction(std::function<void()>&& func) {
    static const auto method = javaClassStatic()->getStaticMethod<void(jlong)>("runStdFunction");
    method(javaClassStatic(), reinterpret_cast<jlong>(&func));
  }

  static void runStdFunctionImpl(alias_ref<JClass>, jlong ptr) {
    (*reinterpret_cast<std::function<void()>*>(ptr))();
  }

  static void OnLoad() {
    // We need the javaClassStatic so that the class lookup is cached and that
    // runStdFunction can be called from a ThreadScope-attached thread.
    javaClassStatic()->registerNatives({
        makeNativeMethod("runStdFunctionImpl", runStdFunctionImpl),
      });
  }
};
}

/* static */
void ThreadScope::OnLoad() {
  // These classes are required for ScopeWithClassLoader. Ensure they are looked up when loading.
  JThreadScopeSupport::OnLoad();
}

/* static */
void ThreadScope::WithClassLoader(std::function<void()>&& runnable) {
  if (cachedOrNull() == nullptr) {
    ThreadScope ts;
    JThreadScopeSupport::runStdFunction(std::move(runnable));
  } else {
    runnable();
  }
}

} }
