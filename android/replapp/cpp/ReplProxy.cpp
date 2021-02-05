#include "hermes/Platform/Intl/PlatformIntl.h"

#include <fbjni/fbjni.h>
#include "hermes/VM/Callable.h"

#define C_STRING(x) #x

using namespace ::facebook;
using namespace ::hermes;

class ReplProxy : public facebook::jni::HybridClass<ReplProxy> {

    std::shared_ptr<vm::Runtime> createAndGetRuntime() {
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

        return vm::Runtime::create(config);
    }

    vm::HermesValue getEvaluateLineFn(const std::shared_ptr<vm::Runtime>& runtime) {

        vm::GCScope gcScope(runtime.get());
        runtime->getHeap().runtimeWillExecute();

        vm::Handle <vm::JSObject> global = runtime->getGlobal();

        // evaluate-line.js will color the output appropriately based on the available of this property in the global object.
        if(vm::ExecutionStatus::EXCEPTION ==
           global->putComputed_RJS(global,
                                   runtime.get(),
                                   vm::StringPrimitive::createNoThrow(runtime.get(), "_replterminaltype"),
                                   vm::StringPrimitive::createNoThrow(runtime.get(), "android"),
                                   vm::PropOpFlags().plusThrowOnError())) {
            throw "Unable to set the terminal type !";
        }

        bool hasColors = true;
        auto propRes = vm::JSObject::getNamed_RJS(
                global, runtime.get(), vm::Predefined::getSymbolID(vm::Predefined::eval));
        if (propRes == vm::ExecutionStatus::EXCEPTION) {
            runtime->printException(
                    llvh::outs(), runtime->makeHandle(runtime->getThrownValue()));
            throw "error getting 'eval' from global";
        }
        auto evalFn = runtime->makeHandle<vm::Callable>(std::move(*propRes));

        llvh::StringRef evaluateLineString =
#include "../../tools/hermes/evaluate-line.js"
        ;

        auto callRes = evalFn->executeCall1(
                evalFn,
                runtime.get(),
                global,
                vm::StringPrimitive::createNoThrow(runtime.get(), evaluateLineString)
                        .getHermesValue());
        if (callRes == vm::ExecutionStatus::EXCEPTION) {
            llvh::raw_ostream &errs = hasColors
                                      ? llvh::errs().changeColor(llvh::raw_ostream::Colors::RED)
                                      : llvh::errs();
            llvh::raw_ostream &outs = hasColors
                                      ? llvh::outs().changeColor(llvh::raw_ostream::Colors::RED)
                                      : llvh::outs();
            errs << "Unable to get REPL util function: evaluateLine.\n";
            runtime->printException(
                    outs, runtime->makeHandle(runtime->getThrownValue()));
            throw "Unable to get REPL util function: evaluateLine";
        }

        return callRes->getHermesValue();
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
        mRuntime->getHeap().runtimeWillExecute();

        vm::MutableHandle<> resHandle{mRuntime.get()};
        vm::GCScopeMarkerRAII gcMarker{mRuntime.get()};

        bool hasColors = true;
        bool threwException = false;

        vm::Handle<vm::Callable> evaluateLineFn = vm::Handle<vm::Callable>::vmcast(&mEvaluateLineFn);

        vm::Handle <vm::JSObject> global = mRuntime->getGlobal();
        auto callRes = evaluateLineFn->executeCall2(
                evaluateLineFn,
                mRuntime.get(),
                global,
                vm::StringPrimitive::createNoThrow(mRuntime.get(), script)
                        .getHermesValue(),
                vm::HermesValue::encodeBoolValue(hasColors));

        if (callRes == vm::ExecutionStatus::EXCEPTION) {
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

            vm::SmallU16String<32> allocator;
            std::string ret;
            ::hermes::convertUTF16ToUTF8WithReplacements(
                    ret, result);

            response.append(ret);
        }

        return response;
    }

    static jni::local_ref<ReplProxy::jhybriddata> initHybrid(jni::alias_ref<jclass>) {
        return makeCxxInstance();
    }

public:
    static constexpr auto kJavaDescriptor = "Lcom/facebook/hermes/replapp/REPLActivity;";

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

private:
    friend class facebook::jni::HybridClass<ReplProxy>;
    ReplProxy()
        : mRuntime(createAndGetRuntime()), mEvaluateLineFn(getEvaluateLineFn(mRuntime)) { }

    std::shared_ptr<vm::Runtime> mRuntime;
    vm::PinnedHermesValue mEvaluateLineFn;
};

extern "C" jint JNIEXPORT JNICALL JNI_OnLoad(JavaVM *jvm, void *reserved) {
    return facebook::jni::initialize(jvm, [] {
        ReplProxy::registerNatives();
    });
}