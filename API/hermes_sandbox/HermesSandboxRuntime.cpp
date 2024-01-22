/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "HermesSandboxRuntime.h"

#include "external/hermes_sandbox_impl_compiled.h"

using namespace facebook::jsi;

namespace {

/// Define a helper macro to throw an exception for unimplemented methods. The
/// actual throw is kept in a separate function because throwing generates a lot
/// of code.
[[noreturn]] void throwUnimplementedImpl(const char *name) {
  throw JSINativeException(std::string("Unimplemented function ") + name);
}

#define THROW_UNIMPLEMENTED() throwUnimplementedImpl(__func__)

class HermesSandboxRuntimeImpl : public facebook::hermes::HermesSandboxRuntime {
 public:
  HermesSandboxRuntimeImpl() {}
  ~HermesSandboxRuntimeImpl() override {}

  Value evaluateJavaScript(
      const std::shared_ptr<const Buffer> &buffer,
      const std::string &sourceURL) override {
    THROW_UNIMPLEMENTED();
  }

  Value evaluateHermesBytecode(
      const std::shared_ptr<const Buffer> &buffer,
      const std::string &sourceURL) override {
    THROW_UNIMPLEMENTED();
  }

  std::shared_ptr<const PreparedJavaScript> prepareJavaScript(
      const std::shared_ptr<const Buffer> &buffer,
      std::string sourceURL) override {
    THROW_UNIMPLEMENTED();
  }

  Value evaluatePreparedJavaScript(
      const std::shared_ptr<const PreparedJavaScript> &js) override {
    THROW_UNIMPLEMENTED();
  }

  bool drainMicrotasks(int maxMicrotasksHint = -1) override {
    THROW_UNIMPLEMENTED();
  }

  Object global() override {
    THROW_UNIMPLEMENTED();
  }

  std::string description() override {
    THROW_UNIMPLEMENTED();
  }

  bool isInspectable() override {
    THROW_UNIMPLEMENTED();
  }

  Instrumentation &instrumentation() override {
    THROW_UNIMPLEMENTED();
  }

 protected:
  PointerValue *cloneSymbol(const Runtime::PointerValue *pv) override {
    THROW_UNIMPLEMENTED();
  }
  PointerValue *cloneBigInt(const Runtime::PointerValue *pv) override {
    THROW_UNIMPLEMENTED();
  }
  PointerValue *cloneString(const Runtime::PointerValue *pv) override {
    THROW_UNIMPLEMENTED();
  }
  PointerValue *cloneObject(const Runtime::PointerValue *pv) override {
    THROW_UNIMPLEMENTED();
  }
  PointerValue *clonePropNameID(const Runtime::PointerValue *pv) override {
    THROW_UNIMPLEMENTED();
  }

  PropNameID createPropNameIDFromAscii(const char *str, size_t length)
      override {
    THROW_UNIMPLEMENTED();
  }
  PropNameID createPropNameIDFromUtf8(const uint8_t *utf8, size_t length)
      override {
    THROW_UNIMPLEMENTED();
  }
  PropNameID createPropNameIDFromString(const String &str) override {
    THROW_UNIMPLEMENTED();
  }
  PropNameID createPropNameIDFromSymbol(const Symbol &sym) override {
    THROW_UNIMPLEMENTED();
  }
  std::string utf8(const PropNameID &) override {
    THROW_UNIMPLEMENTED();
  }
  bool compare(const PropNameID &, const PropNameID &) override {
    THROW_UNIMPLEMENTED();
  }

  std::string symbolToString(const Symbol &) override {
    THROW_UNIMPLEMENTED();
  }

  BigInt createBigIntFromInt64(int64_t) override {
    THROW_UNIMPLEMENTED();
  }
  BigInt createBigIntFromUint64(uint64_t) override {
    THROW_UNIMPLEMENTED();
  }
  bool bigintIsInt64(const BigInt &) override {
    THROW_UNIMPLEMENTED();
  }
  bool bigintIsUint64(const BigInt &) override {
    THROW_UNIMPLEMENTED();
  }
  uint64_t truncate(const BigInt &) override {
    THROW_UNIMPLEMENTED();
  }
  String bigintToString(const BigInt &, int) override {
    THROW_UNIMPLEMENTED();
  }

  String createStringFromAscii(const char *str, size_t length) override {
    THROW_UNIMPLEMENTED();
  }
  String createStringFromUtf8(const uint8_t *utf8, size_t length) override {
    THROW_UNIMPLEMENTED();
  }
  std::string utf8(const String &) override {
    THROW_UNIMPLEMENTED();
  }

  Object createObject() override {
    THROW_UNIMPLEMENTED();
  }
  Object createObject(std::shared_ptr<HostObject> ho) override {
    THROW_UNIMPLEMENTED();
  }
  std::shared_ptr<HostObject> getHostObject(const Object &) override {
    THROW_UNIMPLEMENTED();
  }
  HostFunctionType &getHostFunction(const Function &) override {
    THROW_UNIMPLEMENTED();
  }

  bool hasNativeState(const Object &) override {
    THROW_UNIMPLEMENTED();
  }
  std::shared_ptr<NativeState> getNativeState(const Object &) override {
    THROW_UNIMPLEMENTED();
  }
  void setNativeState(const Object &, std::shared_ptr<NativeState> state)
      override {
    THROW_UNIMPLEMENTED();
  }

  Value getProperty(const Object &, const PropNameID &name) override {
    THROW_UNIMPLEMENTED();
  }
  Value getProperty(const Object &, const String &name) override {
    THROW_UNIMPLEMENTED();
  }
  bool hasProperty(const Object &, const PropNameID &name) override {
    THROW_UNIMPLEMENTED();
  }
  bool hasProperty(const Object &, const String &name) override {
    THROW_UNIMPLEMENTED();
  }
  void setPropertyValue(
      const Object &,
      const PropNameID &name,
      const Value &value) override {
    THROW_UNIMPLEMENTED();
  }
  void setPropertyValue(const Object &, const String &name, const Value &value)
      override {
    THROW_UNIMPLEMENTED();
  }

  bool isArray(const Object &) const override {
    THROW_UNIMPLEMENTED();
  }
  bool isArrayBuffer(const Object &) const override {
    THROW_UNIMPLEMENTED();
  }
  bool isFunction(const Object &) const override {
    THROW_UNIMPLEMENTED();
  }
  bool isHostObject(const Object &) const override {
    THROW_UNIMPLEMENTED();
  }
  bool isHostFunction(const Function &) const override {
    THROW_UNIMPLEMENTED();
  }
  Array getPropertyNames(const Object &) override {
    THROW_UNIMPLEMENTED();
  }

  WeakObject createWeakObject(const Object &) override {
    THROW_UNIMPLEMENTED();
  }
  Value lockWeakObject(const WeakObject &) override {
    THROW_UNIMPLEMENTED();
  }

  Array createArray(size_t length) override {
    THROW_UNIMPLEMENTED();
  }
  ArrayBuffer createArrayBuffer(
      std::shared_ptr<MutableBuffer> buffer) override {
    THROW_UNIMPLEMENTED();
  }
  size_t size(const Array &) override {
    THROW_UNIMPLEMENTED();
  }
  size_t size(const ArrayBuffer &) override {
    THROW_UNIMPLEMENTED();
  }
  uint8_t *data(const ArrayBuffer &) override {
    THROW_UNIMPLEMENTED();
  }
  Value getValueAtIndex(const Array &, size_t i) override {
    THROW_UNIMPLEMENTED();
  }
  void setValueAtIndexImpl(const Array &, size_t i, const Value &value)
      override {
    THROW_UNIMPLEMENTED();
  }

  Function createFunctionFromHostFunction(
      const PropNameID &name,
      unsigned int paramCount,
      HostFunctionType func) override {
    THROW_UNIMPLEMENTED();
  }
  Value call(
      const Function &,
      const Value &jsThis,
      const Value *args,
      size_t count) override {
    THROW_UNIMPLEMENTED();
  }
  Value callAsConstructor(const Function &, const Value *args, size_t count)
      override {
    THROW_UNIMPLEMENTED();
  }

  bool strictEquals(const Symbol &a, const Symbol &b) const override {
    THROW_UNIMPLEMENTED();
  }
  bool strictEquals(const BigInt &a, const BigInt &b) const override {
    THROW_UNIMPLEMENTED();
  }
  bool strictEquals(const String &a, const String &b) const override {
    THROW_UNIMPLEMENTED();
  }
  bool strictEquals(const Object &a, const Object &b) const override {
    THROW_UNIMPLEMENTED();
  }

  bool instanceOf(const Object &o, const Function &f) override {
    THROW_UNIMPLEMENTED();
  }

  void setExternalMemoryPressure(const Object &obj, size_t amount) override {}
};

} // namespace

/// Provide implementations for the functions imported by the sandbox.

extern "C" {

/// These definitions comes from wasi/api.h in Emscripten.
#define WASI_EINVAL 28
#define WASI_ENOSYS 52
#define WASI_CLOCKID_REALTIME 0
#define WASI_CLOCKID_MONOTONIC 1

/* import: 'env' '__syscall_lstat64' */
u32 w2c_env_0x5F_syscall_lstat64(struct w2c_env *, u32, u32) {
  return WASI_ENOSYS;
}

/* import: 'env' '__syscall_newfstatat' */
u32 w2c_env_0x5F_syscall_newfstatat(struct w2c_env *, u32, u32, u32, u32) {
  return WASI_ENOSYS;
}

/* import: 'env' '__syscall_stat64' */
u32 w2c_env_0x5F_syscall_stat64(struct w2c_env *, u32, u32) {
  return WASI_ENOSYS;
}

/* import: 'env' '__syscall_unlinkat' */
u32 w2c_env_0x5F_syscall_unlinkat(struct w2c_env *, u32, u32, u32) {
  return WASI_ENOSYS;
}

/* import: 'wasi_snapshot_preview1' 'environ_get' */
u32 w2c_wasi__snapshot__preview1_environ_get(
    struct w2c_wasi__snapshot__preview1 *,
    u32,
    u32) {
  return WASI_ENOSYS;
}

/* import: 'wasi_snapshot_preview1' 'environ_sizes_get' */
u32 w2c_wasi__snapshot__preview1_environ_sizes_get(
    struct w2c_wasi__snapshot__preview1 *,
    u32,
    u32) {
  return WASI_ENOSYS;
}

/* import: 'wasi_snapshot_preview1' 'fd_close' */
u32 w2c_wasi__snapshot__preview1_fd_close(
    struct w2c_wasi__snapshot__preview1 *,
    u32) {
  return WASI_ENOSYS;
}

/* import: 'wasi_snapshot_preview1' 'fd_fdstat_get' */
u32 w2c_wasi__snapshot__preview1_fd_fdstat_get(
    struct w2c_wasi__snapshot__preview1 *,
    u32,
    u32) {
  return WASI_ENOSYS;
}

/* import: 'wasi_snapshot_preview1' 'fd_seek' */
u32 w2c_wasi__snapshot__preview1_fd_seek(
    struct w2c_wasi__snapshot__preview1 *,
    u32,
    u64,
    u32,
    u32) {
  return WASI_ENOSYS;
}

/* import: 'wasi_snapshot_preview1' 'fd_write' */
u32 w2c_wasi__snapshot__preview1_fd_write(
    struct w2c_wasi__snapshot__preview1 *,
    u32,
    u32,
    u32,
    u32) {
  return WASI_ENOSYS;
}

/* import: 'wasi_snapshot_preview1' 'clock_time_get' */
u32 w2c_wasi__snapshot__preview1_clock_time_get(
    struct w2c_wasi__snapshot__preview1 *,
    u32,
    u64,
    u32) {
  return WASI_ENOSYS;
}

/* import: 'env' 'emscripten_notify_memory_growth' */
void w2c_env_emscripten_notify_memory_growth(struct w2c_env *, u32) {}

/* import: 'hermes_import' 'getentropy' */
u32 w2c_hermes__import_getentropy(struct w2c_hermes__import *, u32, u32) {
  return WASI_ENOSYS;
}

/* import: 'wasi_snapshot_preview1' 'proc_exit' */
void w2c_wasi__snapshot__preview1_proc_exit(
    struct w2c_wasi__snapshot__preview1 *,
    u32) {
  abort();
}
}

namespace facebook {
namespace hermes {

/*static*/ bool HermesSandboxRuntime::isHermesBytecode(
    const uint8_t *data,
    size_t len) {
  // "Hermes" in ancient Greek encoded in UTF-16BE and truncated to 8 bytes.
  constexpr uint64_t MAGIC = 0x1F1903C103BC1FC6;
  return (len >= sizeof(MAGIC) && memcmp(data, &MAGIC, sizeof(MAGIC)) == 0);
}

std::unique_ptr<HermesSandboxRuntime> makeHermesSandboxRuntime() {
  return std::make_unique<HermesSandboxRuntimeImpl>();
}

} // namespace hermes
} // namespace facebook
