// Copyright 2004-present Facebook. All Rights Reserved.

#include <jsi/adapter.h>

namespace facebook {
namespace jsi {

Value HostFunctionAdapter::
operator()(Runtime&, const Value& thisVal, const Value* args, size_t count) {
  return hf(rt, thisVal, args, count);
}

HostObjectAdapter::HostObjectAdapter(Runtime& r, std::shared_ptr<HostObject> h)
    : rt(r), ho(std::move(h)) {}

Value HostObjectAdapter::get(Runtime&, const PropNameID& name) {
  return ho->get(rt, name);
}

void HostObjectAdapter::set(
    Runtime&,
    const PropNameID& name,
    const Value& value) {
  ho->set(rt, name, value);
}

std::vector<PropNameID> HostObjectAdapter::getPropertyNames(Runtime&) {
  return ho->getPropertyNames(rt);
}

} // namespace jsi
} // namespace facebook
