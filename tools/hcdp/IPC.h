/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_TOOLS_HCDP_IPC_H
#define HERMES_TOOLS_HCDP_IPC_H

#include <cstdint>
#include <optional>
#include <string>

namespace hermes {

using ClientID = uint32_t;
using IPCType = char;
constexpr IPCType kConnectIPCType = 'C';
constexpr IPCType kMessageIPCType = 'M';
constexpr IPCType kDisconnectIPCType = 'D';

/// A command sent between the hcdp.js WebSockets server and the hcdp C++
/// binary.
struct IPCCommand {
  IPCType type;
  ClientID clientID;
  std::string message;
};

/// Write an IPCCommand to stdout.
void sendIPC(IPCType type, ClientID clientID, const std::string &message);

/// Read an IPCCommand from stdin.
std::optional<IPCCommand> receiveIPC();

} // namespace hermes

#endif // HERMES_TOOLS_HCDP_IPC_H
