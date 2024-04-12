/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <iostream>
#include <sstream>

#include "IPC.h"

namespace hermes {

void sendIPC(IPCType type, ClientID clientID, const std::string &message) {
  std::cout << type << clientID << message << std::endl;
}

std::optional<IPCCommand> receiveIPC() {
  // Read a line of input
  std::string line;
  if (!std::getline(std::cin, line)) {
    return std::nullopt;
  }

  // Parse the type and client ID
  IPCType type;
  ClientID clientID;
  std::istringstream iss(line);
  if (!(iss >> type >> clientID)) {
    throw std::runtime_error("Malformed IPC command");
  }

  // The remainder of the line is the message
  std::string message;
  std::getline(iss >> std::ws, message);

  return IPCCommand{type, clientID, message};
}

} // namespace hermes
