/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/Support/PageAccessTracker.h"
#ifdef HERMES_HAS_REAL_PAGE_TRACKER

#include <sys/mman.h>
#include <cstdio>

#include <gtest/gtest.h>
#include <hermes/CompileJS.h>
#include <hermes/hermes.h>
#include <jsi/jsi.h>

#include "hermes/Parser/JSONParser.h"
#include "hermes/Support/OSCompat.h"

using namespace facebook::jsi;
using namespace facebook::hermes;
using namespace hermes::parser;

namespace hermes {
namespace {

TEST(TrackIOTest, GetIOTrackingJSONWorks) {
  std::string bytecode;
  std::string js = "var i = 0;";
  for (size_t i = 0; i < hermes::oscompat::page_size(); i++) {
    js.append("i++;");
  }
  ASSERT_TRUE(hermes::compileJS(js, bytecode));
  ASSERT_GT(bytecode.size(), hermes::oscompat::page_size());

  auto rt = makeHermesRuntime(
      hermes::vm::RuntimeConfig::Builder().withTrackIO(true).build());
  const std::string fileUrl = "<in-memory HBC>";
  // This should be tracked
  rt->evaluateJavaScript(std::make_unique<StringBuffer>(bytecode), fileUrl);
  // This won't end up being tracked
  rt->evaluateJavaScript(std::make_unique<StringBuffer>(js), "JS source");
  auto jsonInfo = rt->getIOTrackingInfoJSON();

  // Example output to assert against:
  // [
  //   {
  //      "url": "<in-memory HBC>",
  //      "tracking_info": {
  //         "page_size": 4096,
  //         "total_pages": 30,
  //         "accessed_pages": 22,
  //         "page_ids":
  //         [0,21,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20],
  //         "micros": [0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,0,1]
  //      }
  //    }
  // ]

  auto alloc = std::make_shared<hermes::parser::JSLexer::Allocator>();
  parser::JSONFactory factory(*alloc);
  SourceErrorManager sourceErrorManager;
  parser::JSONParser jsonParser(
      factory,
      llvh::MemoryBufferRef(jsonInfo, "JSON data"),
      sourceErrorManager);

  llvh::Optional<JSONValue *> parsedInfo = jsonParser.parse();
  ASSERT_TRUE(parsedInfo.hasValue());
  auto *runtimeModulesInfo =
      llvh::dyn_cast_or_null<JSONArray>(parsedInfo.getValue());
  ASSERT_NE(runtimeModulesInfo, nullptr);
  ASSERT_EQ(runtimeModulesInfo->size(), 1);
  auto *runtimeModuleInfo =
      llvh::dyn_cast_or_null<JSONObject>(runtimeModulesInfo->at(0));
  ASSERT_NE(runtimeModuleInfo, nullptr);
  ASSERT_EQ(
      llvh::dyn_cast_or_null<JSONString>(runtimeModuleInfo->get("url"))->str(),
      fileUrl);
  auto *trackingInfo = llvh::dyn_cast_or_null<JSONObject>(
      runtimeModuleInfo->get("tracking_info"));
  ASSERT_NE(trackingInfo, nullptr);
  auto *pageSize =
      llvh::dyn_cast_or_null<JSONNumber>(trackingInfo->get("page_size"));
  ASSERT_NE(pageSize, nullptr);
  ASSERT_EQ((size_t)pageSize->getValue(), hermes::oscompat::page_size());
  auto *totalPagesJson =
      llvh::dyn_cast_or_null<JSONNumber>(trackingInfo->get("total_pages"));
  ASSERT_NE(totalPagesJson, nullptr);
  auto totalPages = (int)totalPagesJson->getValue();
  ASSERT_GT(totalPages, 1);
  auto *accessedPagesJson =
      llvh::dyn_cast_or_null<JSONNumber>(trackingInfo->get("accessed_pages"));
  ASSERT_NE(accessedPagesJson, nullptr);
  auto accessedPages = (int)accessedPagesJson->getValue();
  ASSERT_GT(accessedPages, 1);
  ASSERT_LE(accessedPages, totalPages);
  auto *pageIDs =
      llvh::dyn_cast_or_null<JSONArray>(trackingInfo->get("page_ids"));
  ASSERT_NE(pageIDs, nullptr);
  ASSERT_EQ(pageIDs->size(), accessedPages);
  for (size_t i = 0; i < pageIDs->size(); i++) {
    auto *page = llvh::dyn_cast_or_null<JSONNumber>(pageIDs->at(i));
    ASSERT_NE(page, nullptr);
    ASSERT_GE((int)page->getValue(), 0);
    ASSERT_LT((int)page->getValue(), totalPages);
  }
  auto *micros = llvh::dyn_cast_or_null<JSONArray>(trackingInfo->get("micros"));
  ASSERT_NE(micros, nullptr);
  ASSERT_EQ(micros->size(), accessedPages);
  for (size_t i = 0; i < micros->size(); i++) {
    auto *micro = llvh::dyn_cast_or_null<JSONNumber>(micros->at(i));
    ASSERT_NE(micro, nullptr);
    ASSERT_GE((int)micro->getValue(), 0);
  }
}

} // namespace
} // namespace hermes

#endif // HERMES_HAS_REAL_PAGE_TRACKER
