#include "hermes/Platform/Intl/PlatformIntl.h"

#include <fbjni/fbjni.h>
#include "hermes/VM/Callable.h"

#define C_STRING(x) #x

using namespace ::facebook;
using namespace ::hermes;

struct ReplProxy : facebook::jni::HybridClass<ReplProxy> {

    static constexpr auto kJavaDescriptor = "Lcom/facebook/hermes/intltestapp/REPLActivity;";

    std::shared_ptr<vm::Runtime> mRuntime;

    static void registerNatives() {
        javaClassStatic()->registerNatives({
                                                   makeNativeMethod("initHybrid",
                                                           ReplProxy::initHybrid),
                                                   makeNativeMethod("nativeEvalScript",
                                                           ReplProxy::nativeEvalScript),
                                                   makeNativeMethod("nativeCollect",
                                                           ReplProxy::nativeCollect),
                                                   makeNativeMethod("nativeHeapStats",
                                                           ReplProxy::nativeHeapStats)
                                           });
    }

    void GCCallback(vm::GCEventKind kind, const char *pExtraInfo) {
        static const auto method =
                javaClassStatic()->getStaticMethod<void(
                        facebook::jni::local_ref<facebook::jni::JString>)>(
                        "onGCEvent");
        std::string info(pExtraInfo);
        info.append(" : ");
        info.append(kind == vm::GCEventKind::CollectionStart ? "Start" : "End");
        method(javaClassStatic(), facebook::jni::make_jstring(info));
    }

    ReplProxy() {
        vm::RuntimeConfig config = vm::RuntimeConfig::Builder()
                .withGCConfig(
                        vm::GCConfig::Builder()
                                .withInitHeapSize(32 << 20)
                                .withMaxHeapSize(512 << 20)
                                .withSanitizeConfig(vm::GCSanitizeConfig::Builder()
                                                            .withSanitizeRate(0.0)
                                                            .withRandomSeed(-1)
                                                            .build())
                                .withShouldRecordStats(false)
                                .withCallback([this](vm::GCEventKind gcEventKind,
                                                     const char *extraInfo) {
                                    GCCallback(gcEventKind, extraInfo);
                                })
                                .build())
                .withVMExperimentFlags(vm::RuntimeConfig::getDefaultVMExperimentFlags())
                .withES6Promise(vm::RuntimeConfig::getDefaultES6Promise())
                .withES6Proxy(vm::RuntimeConfig::getDefaultES6Proxy())
                .withES6Symbol(vm::RuntimeConfig::getDefaultES6Symbol())
                .withES6Intl(true)
                .withEnableHermesInternal(true)
                .withEnableHermesInternalTestMethods(true)
                .withAllowFunctionToStringWithRuntimeSource(false)
                .build();

        mRuntime = vm::Runtime::create(config);

    }

    static jni::local_ref<ReplProxy::jhybriddata> initHybrid(jni::alias_ref<jclass>) {
        return makeCxxInstance();
    }

    void nativeCollect(facebook::jni::alias_ref<facebook::jni::JString> jCause) {
        mRuntime->collect(jCause->toStdString());
    }

    std::string nativeHeapStats() {
        std::string s;
        llvh::raw_string_ostream os(s);
        mRuntime->printHeapStats(os);
        return s;
    }

    std::string nativeEvalScript(facebook::jni::alias_ref<facebook::jni::JString> jScript) {
        std::string script = jScript->toStdString();
        std::string response;

        vm::GCScope gcScope(mRuntime.get());

        vm::Handle <vm::JSObject> global = mRuntime->getGlobal();
        auto propRes = vm::JSObject::getNamed_RJS(
                global, mRuntime.get(), vm::Predefined::getSymbolID(vm::Predefined::eval));
        if (propRes == vm::ExecutionStatus::EXCEPTION) {
            mRuntime->printException(
                    llvh::outs(), mRuntime->makeHandle(mRuntime->getThrownValue()));
            return "error getting 'eval' from global";
        }
        auto evalFn = mRuntime->makeHandle<vm::Callable>(std::move(*propRes));

        bool hasColors = true; //oscompat::should_color(STDOUT_FILENO);

        llvh::StringRef evaluateLineString =

#include "evaluate-line.js"
        ;

        auto callRes = evalFn->executeCall1(
                evalFn,
                mRuntime.get(),
                global,
                vm::StringPrimitive::createNoThrow(mRuntime.get(), evaluateLineString)
                        .getHermesValue());
        if (callRes == vm::ExecutionStatus::EXCEPTION) {
            llvh::raw_ostream &errs = hasColors
                                      ? llvh::errs().changeColor(llvh::raw_ostream::Colors::RED)
                                      : llvh::errs();
            llvh::raw_ostream &outs = hasColors
                                      ? llvh::outs().changeColor(llvh::raw_ostream::Colors::RED)
                                      : llvh::outs();
            errs << "Unable to get REPL util function: evaluateLine.\n";
            mRuntime->printException(
                    outs, mRuntime->makeHandle(mRuntime->getThrownValue()));
            return "Unable to get REPL util function: evaluateLine";
        }

        vm::Handle <vm::JSFunction> evaluateLineFn =
                mRuntime->makeHandle<vm::JSFunction>(std::move(*callRes));

        mRuntime->getHeap().runtimeWillExecute();

        vm::MutableHandle<> resHandle{mRuntime.get()};

        // Ensure we don't keep accumulating handles.
        vm::GCScopeMarkerRAII gcMarker{mRuntime.get()};

        bool threwException = false;

        if ((callRes = evaluateLineFn->executeCall2(
                evaluateLineFn,
                mRuntime.get(),
                global,
                vm::StringPrimitive::createNoThrow(mRuntime.get(), script)
                        .getHermesValue(),
                vm::HermesValue::encodeBoolValue(hasColors))) ==
            vm::ExecutionStatus::EXCEPTION) {
            mRuntime->printException(
                    hasColors ? llvh::outs().changeColor(llvh::raw_ostream::Colors::RED)
                              : llvh::outs(),
                    mRuntime->makeHandle(mRuntime->getThrownValue()));
            llvh::outs().resetColor();
            threwException = true;
        } else {
            resHandle = std::move(*callRes);
        }

        if (resHandle->isUndefined()) {
            response.append("undefined");
        } else {
            auto stringView = vm::StringPrimitive::createStringView(
                    mRuntime.get(), vm::Handle<vm::StringPrimitive>::vmcast(resHandle));

            vm::SmallU16String<32> tmp;
            vm::UTF16Ref result = stringView.getUTF16Ref(tmp);

            response.append(std::string(result.begin(), result.end()));
        }

        return response;
    }
};

extern "C" jint JNIEXPORT JNICALL JNI_OnLoad(JavaVM *jvm, void *reserved) {
    return facebook::jni::initialize(jvm, [] {
        ReplProxy::registerNatives();
    });
}