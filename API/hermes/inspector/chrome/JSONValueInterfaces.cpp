/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "JSONValueInterfaces.h"
#include "hermes/include/hermes/Parser/JSONParser.h"

namespace facebook {
namespace hermes {
namespace inspector {
namespace chrome {

JSONValue *parseStr(const std::string &str, JSONFactory &factory) {
  ::hermes::SourceErrorManager sm;
  ::hermes::SourceErrorManager::SaveAndSuppressMessages suppress(&sm);
  JSONParser jsonParser(factory, str, sm);
  auto jsonRes = jsonParser.parse();
  if (!jsonRes) {
    throw std::runtime_error("Invalid JSON");
  }
  return *jsonRes;
}

JSONObject *parseStrAsJsonObj(const std::string &str, JSONFactory &factory) {
  auto *jsonVal = parseStr(str, factory);
  auto *jsonObj = llvh::dyn_cast_or_null<JSONObject>(jsonVal);
  if (!jsonObj) {
    throw std::runtime_error("JSON value not an object");
  }
  return jsonObj;
}

std::string jsonValToStr(const JSONValue *v) {
  std::string storage;
  llvh::raw_string_ostream OS(storage);
  ::hermes::JSONEmitter emitter(OS);
  v->emitInto(emitter);
  return std::move(OS.str());
}

JSONValue *get(const JSONObject *obj, const std::string &key) {
  JSONValue *v = obj->get(key);
  if (v == nullptr) {
    throw std::runtime_error("key not found: " + key);
  }
  return v;
}

JSONValue *safeGet(const JSONObject *obj, const std::string &key) {
  return obj->get(key);
}

bool jsonValsEQ(const JSONValue *A, const JSONValue *B) {
  if (A == B) {
    return true;
  }
  if (A->getKind() != B->getKind()) {
    return false;
  }
  switch (A->getKind()) {
    case JSONKind::Object: {
      auto *Aobj = llvh::cast<JSONObject>(A);
      auto *Bobj = llvh::cast<JSONObject>(B);
      if (Aobj->size() != Bobj->size()) {
        return false;
      }
      for (const JSONString *keyInA : *Aobj->getHiddenClass()) {
        llvh::StringRef keyStr = keyInA->str();
        JSONValue *valInB = Bobj->get(keyStr);
        if (valInB == nullptr) {
          return false;
        }
        if (!jsonValsEQ(valInB, Aobj->at(keyStr))) {
          return false;
        }
      }
      return true;
    }
    case JSONKind::Array: {
      auto *Aarr = llvh::cast<JSONArray>(A);
      auto *Barr = llvh::cast<JSONArray>(B);
      if (Aarr->size() != Barr->size()) {
        return false;
      }
      for (size_t i = 0, e = Aarr->size(); i < e; i++) {
        if (!jsonValsEQ(Aarr->at(i), Barr->at(i))) {
          return false;
        }
      }
      return true;
    }
    case JSONKind::String: {
      llvh::StringRef Astr = llvh::cast<JSONString>(A)->str();
      llvh::StringRef Bstr = llvh::cast<JSONString>(B)->str();
      return Astr == Bstr;
    }
    case JSONKind::Number: {
      double Anum = llvh::cast<JSONNumber>(A)->getValue();
      double Bnum = llvh::cast<JSONNumber>(B)->getValue();
      assert(
          (!std::isnan(Anum) && !std::isnan(Bnum)) &&
          "NaNs not supported in JSON");
      return Anum == Bnum;
    }
    case JSONKind::Boolean: {
      bool Abool = llvh::cast<JSONBoolean>(A)->getValue();
      bool Bbool = llvh::cast<JSONBoolean>(B)->getValue();
      return Abool == Bbool;
    }
    case JSONKind::Null: {
      // two nulls are always equal to each other.
      return true;
    }
  }
  return false;
}

} // namespace chrome
} // namespace inspector
} // namespace hermes
} // namespace facebook
