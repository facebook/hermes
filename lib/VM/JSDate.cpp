#include "hermes/VM/JSDate.h"

#include "hermes/VM/BuildMetadata.h"

namespace hermes {
namespace vm {

//===----------------------------------------------------------------------===//
// class JSDate

ObjectVTable JSDate::vt{
    VTable(CellKind::DateKind, sizeof(JSDate)),
    JSDate::_getOwnIndexedRangeImpl,
    JSDate::_haveOwnIndexedImpl,
    JSDate::_getOwnIndexedPropertyFlagsImpl,
    JSDate::_getOwnIndexedImpl,
    JSDate::_setOwnIndexedImpl,
    JSDate::_deleteOwnIndexedImpl,
    JSDate::_checkAllOwnIndexedImpl,
};

void DateBuildMeta(const GCCell *cell, Metadata::Builder &mb) {
  ObjectBuildMeta(cell, mb);
}

CallResult<HermesValue>
JSDate::create(Runtime *runtime, double value, Handle<JSObject> protoHandle) {
  auto propStorage =
      JSObject::createPropStorage(runtime, NEEDED_PROPERTY_SLOTS);
  if (LLVM_UNLIKELY(propStorage == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }

  void *mem = runtime->alloc(sizeof(JSDate));
  auto selfHandle = runtime->makeHandle(new (mem) JSDate(
      runtime,
      *protoHandle,
      runtime->getHiddenClassForPrototypeRaw(*protoHandle),
      **propStorage));
  JSObject::addInternalProperties(
      selfHandle,
      runtime,
      1,
      runtime->makeHandle(HermesValue::encodeDoubleValue(value)));
  return selfHandle.getHermesValue();
}

} // namespace vm
} // namespace hermes
