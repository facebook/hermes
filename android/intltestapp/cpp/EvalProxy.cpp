#include <fbjni/fbjni.h>

#include <hermes/hermes.h>
#include <jsi/decorator.h>

using namespace ::facebook;
using namespace ::hermes;
using namespace facebook::hermes;
using namespace facebook::jsi;

struct EvalProxy : facebook::jni::JavaClass<EvalProxy> {
    static constexpr auto kJavaDescriptor = "Lcom/facebook/hermes/intltestapp/EvalActivity;";

    static std::string nativeEvalScript(jni::alias_ref<EvalProxy> thiz,
                                        facebook::jni::alias_ref<facebook::jni::JString> jScript) {
        std::string script = jScript->toStdString();

        ::hermes::vm::RuntimeConfig runtimeConfig_ = vm::RuntimeConfig::Builder()
                .withES6Intl(true)
                .build();

        std::unique_ptr<HermesRuntime> hermesRuntime =
                makeHermesRuntime(runtimeConfig_);

        try {
            auto res = hermesRuntime->evaluateJavaScript(
                    std::make_shared<facebook::jsi::StringBuffer>(script), "");

            if (res.isUndefined()) {
                return std::string("undefined");
            } else if (res.isString()) {
                return res.asString(*hermesRuntime).utf8(*hermesRuntime);
            } else if (res.isNumber()) {
                return std::to_string(res.asNumber());
            } else if (res.isNull()) {
                return std::string("NULL");
            } else if (res.isObject()) {
                return std::string("[Object]");
            } else if (res.isBool()) {
                return std::string("Boolean");
            } else if (res.isSymbol()) {
                return std::string("Symbol");
            } else {
                return std::string("Uknown");
            }
        }
        catch (const JSError& ex) {
                std::string exc = ex.what();
                return exc;
        }
    }

    static void registerNatives() {
        javaClassStatic()->registerNatives({
            makeNativeMethod("nativeEvalScript", EvalProxy::nativeEvalScript)
        });
    }
};


extern "C" jint JNIEXPORT JNICALL JNI_OnLoad(JavaVM *jvm, void *reserved) {
    return facebook::jni::initialize(jvm, [] {
        EvalProxy::registerNatives();
    });
}
