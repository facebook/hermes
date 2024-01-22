/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_CDP_DOMAINAGENT_H
#define HERMES_CDP_DOMAINAGENT_H

#include <string>

#include <hermes/inspector/chrome/MessageTypes.h>

namespace facebook {
namespace hermes {
namespace cdp {

namespace m = ::facebook::hermes::inspector_modern::chrome::message;

using OutboundMessageFunc = std::function<void(const std::string &)>;

class DomainAgent {
 protected:
  DomainAgent(OutboundMessageFunc messageCallback)
      : messageCallback_(messageCallback) {}

  /// Sends the provided string back to the debug client
  void sendToClient(const std::string &str) {
    messageCallback_(str);
  }

  /// Sends the provided \p Response back to the debug client
  void sendResponseToClient(const m::Response &resp) {
    sendToClient(resp.toJsonStr());
  }

  /// Sends the provided \p Notification back to the debug client
  void sendNotificationToClient(const m::Notification &note) {
    sendToClient(note.toJsonStr());
  }

  /// Callback function to send CDP response back to the debug client
  OutboundMessageFunc messageCallback_;
};

} // namespace cdp
} // namespace hermes
} // namespace facebook

#endif // HERMES_CDP_DOMAINAGENT_H
