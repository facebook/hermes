// Copyright 2004-present Facebook. All Rights Reserved.

#pragma once

#include <jsi/jsi.h>

// This file contains objects to help API users create their own
// runtime adapters, i.e. if you want to compose runtimes to add your
// own behavior.

namespace facebook {
namespace jsi {

// Use this to wrap host functions. It will pass the member runtime as
// the first arg to the callback.
struct HostFunctionAdapter {
  Value
  operator()(Runtime&, const Value& thisVal, const Value* args, size_t count);

  Runtime& rt;
  HostFunctionType hf;
};

// Use this to wrap host objects, it will pass the member runtime as the
// first argument to all of the member functions a host object has.
struct HostObjectAdapter : public HostObject {
  HostObjectAdapter(Runtime& rt, std::shared_ptr<HostObject> ho);

  Value get(Runtime&, const PropNameID& name) override;
  void set(Runtime&, const PropNameID& name, const Value& value) override;
  std::vector<PropNameID> getPropertyNames(Runtime& rt) override;

  Runtime& rt;
  std::shared_ptr<HostObject> ho;
};

} // namespace jsi
} // namespace facebook
