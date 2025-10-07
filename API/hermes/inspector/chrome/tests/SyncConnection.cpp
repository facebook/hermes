/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "SyncConnection.h"

#include <functional>
#include <stdexcept>

#include <glog/logging.h>

#include <hermes/Parser/JSONParser.h>
#include <hermes/inspector/RuntimeAdapter.h>
#include <hermes/inspector/chrome/JSONValueInterfaces.h>
#include <hermes/inspector/chrome/MessageConverters.h>

#include "TestHelpers.h"

namespace facebook {
namespace hermes {
namespace inspector_modern {
namespace chrome {

using namespace std::placeholders;

void ExecutorRuntimeAdapter::tickleJs() {
  runtime_.tickleJsAsync();
}

SyncConnection::SyncConnection(
    AsyncHermesRuntime &runtime,
    bool waitForDebugger)
    : cdpHandler_(CDPHandler::create(
          std::make_unique<ExecutorRuntimeAdapter>(runtime),
          waitForDebugger)) {
  registerCallbacks();
}

SyncConnection::~SyncConnection() {
  unregisterCallbacks();
}

bool SyncConnection::registerCallbacks() {
  bool registered = cdpHandler_->registerCallbacks(
      std::bind(&SyncConnection::onReply, this, std::placeholders::_1),
      std::bind(&SyncConnection::onUnregister, this));
  if (registered) {
    onUnregisterCalled_ = false;
  }
  return registered;
}

bool SyncConnection::unregisterCallbacks() {
  return cdpHandler_->unregisterCallbacks();
}

bool SyncConnection::onUnregisterWasCalled() {
  return onUnregisterCalled_;
}

void SyncConnection::send(const std::string &str) {
  LOG(INFO) << "SyncConnection::send sending " << str;

  cdpHandler_->handle(str);
}

std::string SyncConnection::waitForMessage(std::chrono::milliseconds timeout) {
  std::unique_lock<std::mutex> lock(mutex_);

  bool success = hasMessage_.wait_for(
      lock, timeout, [this]() -> bool { return !messages_.empty(); });

  if (!success) {
    throw std::runtime_error("timed out waiting for reply");
  }

  std::string message = std::move(messages_.front());
  messages_.pop();
  return message;
}

void SyncConnection::onReply(const std::string &message) {
  LOG(INFO) << "SyncConnection::onReply got message: " << message;
  std::lock_guard<std::mutex> lock(mutex_);
  messages_.push(message);
  hasMessage_.notify_one();
}

void SyncConnection::onUnregister() {
  onUnregisterCalled_ = true;
}

} // namespace chrome
} // namespace inspector_modern
} // namespace hermes
} // namespace facebook
