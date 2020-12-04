#include <fbjni/fbjni.h>

extern "C" jint JNIEXPORT JNICALL JNI_OnLoad(JavaVM *jvm, void *reserved) {
  return facebook::jni::initialize(jvm, [] {});
}