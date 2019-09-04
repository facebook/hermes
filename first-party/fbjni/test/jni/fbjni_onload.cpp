// Copyright 2004-present Facebook. All Rights Reserved.

#include <jni.h>

#include <fbjni/fbjni.h>

using namespace facebook::jni;

void RegisterFbjniTests();
void RegisterTestHybridClass();
void RegisterPrimitiveArrayTests();
void RegisterIteratorTests();
void RegisterByteBufferTests();
void RegisterReadableByteChannelTests();

jint JNI_OnLoad(JavaVM* vm, void*) {
  return facebook::jni::initialize(vm, [] {
      RegisterFbjniTests();
      RegisterTestHybridClass();
      RegisterPrimitiveArrayTests();
      RegisterIteratorTests();
      RegisterByteBufferTests();
      RegisterReadableByteChannelTests();
  });
}
