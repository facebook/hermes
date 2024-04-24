/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "CDPHandler.h"

#include <chrono>
#include <condition_variable>
#include <cstdlib>
#include <iostream>
#include <mutex>
#include <optional>
#include <queue>
#include <sstream>
#include <string>
#include <thread>
#include <unordered_set>

#include <hermes/DebuggerAPI.h>
#include <hermes/hermes.h>
#include <hermes/inspector/AsyncPauseState.h>
#include <hermes/inspector/Exceptions.h>
#include <hermes/inspector/RuntimeAdapter.h>
#include <hermes/inspector/chrome/CallbackOStream.h>
#include <hermes/inspector/chrome/MessageConverters.h>
#include <hermes/inspector/chrome/RemoteObjectConverters.h>
#include <hermes/inspector/chrome/RemoteObjectsTable.h>
#include <jsi/instrumentation.h>

namespace facebook {
namespace hermes {
namespace inspector {
namespace chrome {

using ::facebook::react::ILocalConnection;
using ::facebook::react::IRemoteConnection;
using namespace ::hermes::parser;

namespace debugger = ::facebook::hermes::debugger;
namespace inspector = ::facebook::hermes::inspector;
namespace m = ::facebook::hermes::inspector::chrome::message;

static const char *const kVirtualBreakpointPrefix = "virtualbreakpoint-";
static const char *const kBeforeScriptWithSourceMapExecution =
    "beforeScriptWithSourceMapExecution";
static const char *const kUserEnteredScriptPrefix = "userScript";
static const char *const kDebuggerEnableMethod = "Debugger.enable";
static const char *const kDebuggerMethodPrefix = "Debugger";

static const int kMaxPendingConsoleMessages = 1000;

struct Script {
  uint32_t fileId{};
  std::string fileName;
  std::string sourceMappingUrl;
  bool notifiedClient;
};

struct ConsoleMessageInfo {
  double timestamp;
  std::string source;
  std::string level;
  std::string url;
  int line;
  int column;

  jsi::Array args;

  ConsoleMessageInfo(double timestamp, std::string level, jsi::Array args)
      : timestamp(timestamp),
        source("console-api"),
        level(level),
        url(""),
        line(-1),
        column(-1),
        args(std::move(args)) {}
};

enum Attachment {
  None,
  Enabled,
  Disabled,
};

enum Execution {
  Paused,
  Running,
};

/*
 * CDPHandler::Impl
 */

class CDPHandler::Impl : public message::RequestHandler,
                         public debugger::EventObserver {
 public:
  Impl(
      std::unique_ptr<RuntimeAdapter> adapter,
      const std::string &title,
      bool waitForDebugger);
  ~Impl() override;

  HermesRuntime &getRuntime();
  std::string getTitle() const;

  bool connect(std::unique_ptr<IRemoteConnection> remoteConn);
  bool disconnect();
  void sendMessage(std::string str);

  /* RequestHandler overrides */
  void handle(const m::UnknownRequest &req) override;
  void handle(const m::debugger::DisableRequest &req) override;
  void handle(const m::debugger::EnableRequest &req) override;
  void handle(const m::debugger::EvaluateOnCallFrameRequest &req) override;
  void handle(const m::debugger::PauseRequest &req) override;
  void handle(const m::debugger::RemoveBreakpointRequest &req) override;
  void handle(const m::debugger::ResumeRequest &req) override;
  void handle(const m::debugger::SetBreakpointRequest &req) override;
  void handle(const m::debugger::SetBreakpointByUrlRequest &req) override;
  void handle(const m::debugger::SetBreakpointsActiveRequest &req) override;
  void handle(
      const m::debugger::SetInstrumentationBreakpointRequest &req) override;
  void handle(const m::debugger::SetPauseOnExceptionsRequest &req) override;
  void handle(const m::debugger::StepIntoRequest &req) override;
  void handle(const m::debugger::StepOutRequest &req) override;
  void handle(const m::debugger::StepOverRequest &req) override;
  void handle(const m::heapProfiler::TakeHeapSnapshotRequest &req) override;
  void handle(
      const m::heapProfiler::StartTrackingHeapObjectsRequest &req) override;
  void handle(
      const m::heapProfiler::StopTrackingHeapObjectsRequest &req) override;
  void handle(const m::heapProfiler::StartSamplingRequest &req) override;
  void handle(const m::heapProfiler::StopSamplingRequest &req) override;
  void handle(const m::heapProfiler::CollectGarbageRequest &req) override;
  void handle(
      const m::heapProfiler::GetObjectByHeapObjectIdRequest &req) override;
  void handle(const m::heapProfiler::GetHeapObjectIdRequest &req) override;
  void handle(const m::profiler::StartRequest &req) override;
  void handle(const m::profiler::StopRequest &req) override;
  void handle(const m::runtime::CallFunctionOnRequest &req) override;
  void handle(const m::runtime::CompileScriptRequest &req) override;
  void handle(const m::runtime::DisableRequest &req) override;
  void handle(const m::runtime::EnableRequest &req) override;
  void handle(const m::runtime::EvaluateRequest &req) override;
  void handle(const m::runtime::GetHeapUsageRequest &req) override;
  void handle(const m::runtime::GetPropertiesRequest &req) override;
  void handle(const m::runtime::GlobalLexicalScopeNamesRequest &req) override;
  void handle(const m::runtime::RunIfWaitingForDebuggerRequest &req) override;

  debugger::PauseReason getPauseReason() {
    return getDebugger().getProgramState().getPauseReason();
  }

  debugger::Debugger &getDebugger() {
    return runtimeAdapter_->getRuntime().getDebugger();
  }

  // Whether we started with pauseOnFirstStatement, and have not yet had a
  // debugger attach and ask to resume from that point. This matches the
  // semantics of when CDP Debugger.runIfWaitingForDebugger should resume.
  bool isAwaitingDebuggerOnStart();

  void triggerAsyncPause();

  void sendErrorToClient(int id, const std::string &msg);

  void
  sendErrorCodeToClient(int id, m::ErrorCode errorCode, const std::string &msg);
  void resetScriptsLoaded();

  void sendPausedNotificationToClient();

  void enqueueFunc(
      std::function<void(const debugger::ProgramState &state)> func);
  void enqueueFunc(std::function<void()> func);
  void processPendingFuncs();
  void processPendingDesiredAttachments();
  // If the runtime paused due to an exception being thrown, this method will
  // send the appropriate notification to the client.
  void sendPauseOnExceptionNotification();
  // The following disable/enable methods should only be called from the runtime
  // thread, via the didPause callback.
  void enableDebugger();
  void disableDebugger();
  bool isDebuggerDisabled();
  void processPendingDesiredExecutions(debugger::PauseReason pauseReason);
  // Check what script has just finished parsing, and add it to the appropriate
  // data structures. This does not send any notifications.
  void processCurrentScriptLoaded();
  // Iterate over all the scripts we have already seen as parsed, and
  // potentially notify the client about them.
  void processPendingScriptLoads();
  Script getScriptFromTopCallFrame();
  debugger::Command didPause(debugger::Debugger &debugger) override;

  template <typename T>
  void setHermesLocation(
      debugger::SourceLocation &hermesLoc,
      const T &chromeLoc) {
    hermesLoc.line = chromeLoc.lineNumber + 1;

    if (chromeLoc.columnNumber.has_value()) {
      if (chromeLoc.columnNumber.value() == 0) {
        // TODO: When CDTP sends a column number of 0, we send Hermes a column
        // number of 1. For some reason, this causes Hermes to not be
        // able to resolve breakpoints.
        hermesLoc.column = debugger::kInvalidLocation;
      } else {
        hermesLoc.column = chromeLoc.columnNumber.value() + 1;
      }
    }

    if (chromeLoc.url.has_value()) {
      // TODO: still relevant? RN-ism, maybe should move responsibility
      // elsewhere/out?
      hermesLoc.fileName = m::stripCachePrevention(chromeLoc.url.value());
    } else if (chromeLoc.urlRegex.has_value()) {
      const std::regex regex(
          m::stripCachePrevention(chromeLoc.urlRegex.value()));
      for (const auto &[name, _] : loadedScriptIdByName_) {
        if (std::regex_match(name, regex)) {
          hermesLoc.fileName = name;
          break;
        }
      }
    }
  }

  template <typename R>
  void enqueueDesiredAttachment(const R &req, Attachment attachment) {
    std::lock_guard<std::mutex> lock(mutex_);
    pendingDesiredAttachments_.push({req.id, attachment});
    triggerAsyncPause();
  }

  template <typename R>
  void enqueueDesiredStep(const R &req, debugger::StepMode step) {
    std::lock_guard<std::mutex> lock(mutex_);
    pendingDesiredSteps_.push({req.id, step});
    triggerAsyncPause();
  }

  template <typename R>
  void enqueueDesiredExecution(const R &req, Execution execution) {
    std::lock_guard<std::mutex> lock(mutex_);
    pendingDesiredExecutions_.push({req.id, execution});
  }

 private:
  // The execution context id reported back by the ExecutionContextCreated
  // notification. We only ever expect this execution context id.
  static constexpr int32_t kHermesExecutionContextId = 1;
  std::vector<m::runtime::PropertyDescriptor> makePropsFromScope(
      std::pair<uint32_t, uint32_t> frameAndScopeIndex,
      const std::string &objectGroup,
      const debugger::ProgramState &state,
      bool generatePreview);
  std::vector<m::runtime::PropertyDescriptor> makePropsFromValue(
      const jsi::Value &value,
      const std::string &objectGroup,
      bool onlyOwnProperties,
      bool generatePreview);

  void sendSnapshot(
      int reqId,
      std::string message,
      bool reportProgress,
      bool stopStackTraceCapture);
  void sendToClient(const std::string &str);
  void sendResponseToClient(const m::Response &resp);
  void sendNotificationToClient(const m::Notification &resp);

  bool validateExecutionContext(
      int id,
      std::optional<m::runtime::ExecutionContextId> context);

  // Emit a Runtime.consoleAPICalled event to the debug client.
  void emitConsoleAPICalledEvent(ConsoleMessageInfo info);
  void storePendingConsoleMessage(ConsoleMessageInfo info);
  // Install console log handler. This is to detect console.XXX methods, and
  // then emit the corresponding Runtime.consoleAPICalled event.
  void installLogHandler();
  // Install a host function for a particular console method. This takes a
  // reference to the original console object, so calls can still be forwarded
  // to that object.
  void installConsoleFunction(
      jsi::Object &console,
      std::shared_ptr<jsi::Object> &originalConsole,
      const std::string &name,
      const std::string &chromeTypeDefault = "");
  double currentTimestampMs();

  std::shared_ptr<RuntimeAdapter> runtimeAdapter_;
  std::string title_;

  // connected_ is protected by connectionMutex_.
  std::mutex connectionMutex_;
  bool connected_;

  // preparedScripts_ stores user-entered scripts that have been prepared for
  // execution, and may be invoked by a later command.
  std::vector<std::shared_ptr<const jsi::PreparedJavaScript>> preparedScripts_;

  // Some events are represented as a mode in Hermes but a breakpoint in CDP,
  // e.g. "beforeScriptExecution" and "beforeScriptWithSourceMapExecution".
  // Keep track of these separately. The caller should lock the
  // virtualBreakpointMutex_.
  std::mutex virtualBreakpointMutex_;
  uint32_t nextVirtualBreakpoint_ = 1;
  const std::string &createVirtualBreakpoint(const std::string &category);
  bool isVirtualBreakpointId(const std::string &id);
  bool hasVirtualBreakpoint(const std::string &category);
  bool removeVirtualBreakpoint(const std::string &id);
  std::unordered_map<std::string, std::unordered_set<std::string>>
      virtualBreakpoints_;

  // remoteConn_ is protected by remoteConnMutex_.
  std::mutex remoteConnMutex_;
  std::unique_ptr<IRemoteConnection> remoteConn_;

  // objTable_ is protected by the inspector lock. It should only be accessed
  // when the VM is paused, e.g. in an InspectorObserver callback or in an
  // executeIfEnabled callback.
  RemoteObjectsTable objTable_;

  bool breakpointsActive_ = true;

  // Unguarded
  Attachment currentAttachment_ = Attachment::None;
  Execution currentExecution_ = Execution::Running;
  std::unordered_map<int, Script> loadedScripts_;
  std::unordered_map<std::string, int> loadedScriptIdByName_;
  bool runtimeEnabled_ = false;
  int numPendingConsoleMessagesDiscarded_ = 0;
  std::deque<ConsoleMessageInfo> pendingConsoleMessages_;

  // Guarded by mutex
  std::mutex mutex_;
  std::queue<std::pair<int, Execution>> pendingDesiredExecutions_;
  std::queue<std::pair<int, Attachment>> pendingDesiredAttachments_;
  std::queue<std::pair<int, debugger::StepMode>> pendingDesiredSteps_;
  bool awaitingDebuggerOnStart_;
  std::condition_variable signal_;
  struct PendingEvalReq {
    long long id;
    uint32_t frameIdx;
    std::string expression;
    std::optional<std::string> objectGroup;
    std::optional<bool> returnByValue;
    std::optional<bool> generatePreview;
    std::optional<std::function<void(
        std::shared_ptr<m::runtime::RemoteObject>,
        const facebook::hermes::debugger::EvalResult &)>>
        onEvalCompleteCallback;
  };
  std::queue<PendingEvalReq> pendingEvals_;
  std::queue<std::function<void(const debugger::ProgramState &state)>>
      pendingFuncs_;
};

CDPHandler::Impl::Impl(
    std::unique_ptr<RuntimeAdapter> adapter,
    const std::string &title,
    bool waitForDebugger)
    : runtimeAdapter_(std::move(adapter)),
      title_(title),
      connected_(false),
      remoteConn_(nullptr),
      awaitingDebuggerOnStart_(waitForDebugger) {
  // Install __tickleJs. Do this activity before the call to setEventObserver,
  // so we don't get any didPause callback firings for these.
  std::string src = "function __tickleJs() { return Math.random(); }";
  runtimeAdapter_->getRuntime().evaluateJavaScript(
      std::make_shared<jsi::StringBuffer>(src), "__tickleJsHackUrl");
  installLogHandler();
  runtimeAdapter_->getRuntime().getDebugger().setShouldPauseOnScriptLoad(true);
  runtimeAdapter_->getRuntime().getDebugger().setEventObserver(this);
}

CDPHandler::Impl::~Impl() = default;

HermesRuntime &CDPHandler::Impl::getRuntime() {
  return runtimeAdapter_->getRuntime();
}

std::string CDPHandler::Impl::getTitle() const {
  return title_;
}

bool CDPHandler::Impl::connect(std::unique_ptr<IRemoteConnection> remoteConn) {
  assert(remoteConn);
  std::lock_guard<std::mutex> lock(connectionMutex_);

  if (connected_) {
    return false;
  }

  connected_ = true;
  remoteConn_ = std::move(remoteConn);

  return true;
}

bool CDPHandler::Impl::disconnect() {
  std::lock_guard<std::mutex> lock(connectionMutex_);

  if (!connected_) {
    return false;
  }

  connected_ = false;

  // NOTE: Inspector logic used to be quite convoluted here.

  std::lock_guard<std::mutex> remoteConnLock(remoteConnMutex_);
  auto conn = remoteConn_.release();
  conn->onDisconnect();
  delete conn;

  return true;
}

static bool isDebuggerRequest(const m::Request &req) {
  // If the request method begins with the Debugger method prefix string, then
  // it is a request meant for the debugger.
  return req.method.rfind(kDebuggerMethodPrefix, 0) == 0;
}

void CDPHandler::Impl::sendMessage(std::string str) {
  std::unique_ptr<m::Request> req = m::Request::fromJson(str);
  if (!req) {
    return;
  }

  // If the debugger is currently disabled and the incoming method is for
  // the debugger, then error out to the request immediately here. We make
  // an exception for Debugger.enable though, otherwise we would never be
  // able to turn the debugger back on once it's disabled.
  if (isDebuggerDisabled() && isDebuggerRequest(*req) &&
      (req->method != kDebuggerEnableMethod)) {
    sendResponseToClient(m::makeErrorResponse(
        req->id, m::ErrorCode::ServerError, "Debugger agent is not enabled"));
  } else {
    req->accept(*this);
  }
}

void CDPHandler::Impl::emitConsoleAPICalledEvent(ConsoleMessageInfo info) {
  // buffer console messages before devtool connects
  if (!runtimeEnabled_) {
    storePendingConsoleMessage(std::move(info));
    return;
  }

  m::runtime::ConsoleAPICalledNotification apiCalledNote;
  apiCalledNote.type = info.level;
  apiCalledNote.timestamp = info.timestamp;
  apiCalledNote.executionContextId = kHermesExecutionContextId;

  size_t argsSize = info.args.size(getRuntime());
  for (size_t index = 0; index < argsSize; ++index) {
    apiCalledNote.args.push_back(m::runtime::makeRemoteObject(
        getRuntime(),
        info.args.getValueAtIndex(getRuntime(), index),
        objTable_,
        "ConsoleObjectGroup",
        false,
        false));
  }

  sendNotificationToClient(apiCalledNote);
}

void CDPHandler::Impl::storePendingConsoleMessage(ConsoleMessageInfo info) {
  assert(pendingConsoleMessages_.size() <= kMaxPendingConsoleMessages);

  // Unfortunately we can't have unlimited buffer space, so there will be
  // situations when we run out of storage. We could either discard earlier
  // messages, or discard later messages. What's more useful depends on how the
  // client's app is written, and whether the useful logs happen earlier or
  // later.
  //
  // Chromium's implementation discards the earlier messages.
  //
  // Since this is a new feature for us, opting to match Chromium's behavior for
  // two reasons:
  // 1. Provide a consistent behavior between devtool environments
  // 2. Assuming that Chromium has better insight into which approach is more
  //    desirable
  if (pendingConsoleMessages_.size() == kMaxPendingConsoleMessages) {
    // Increment a counter so we can inform users how many messages were
    // discarded.
    numPendingConsoleMessagesDiscarded_++;
    pendingConsoleMessages_.pop_front();
  }

  pendingConsoleMessages_.push_back(std::move(info));
}

double CDPHandler::Impl::currentTimestampMs() {
  return std::chrono::duration<double, std::milli>(
             std::chrono::system_clock::now().time_since_epoch())
      .count();
}

/*
 * RequestHandler overrides
 */

void CDPHandler::Impl::handle(const m::UnknownRequest &req) {
  sendErrorCodeToClient(
      req.id, m::ErrorCode::MethodNotFound, req.method + " wasn't found");
}

void CDPHandler::Impl::handle(const m::debugger::DisableRequest &req) {
  enqueueDesiredAttachment(req, Attachment::Disabled);
}

void CDPHandler::Impl::handle(const m::debugger::EnableRequest &req) {
  enqueueDesiredAttachment(req, Attachment::Enabled);
}

void CDPHandler::Impl::handle(
    const m::debugger::EvaluateOnCallFrameRequest &req) {
  {
    std::lock_guard<std::mutex> lock(mutex_);
    pendingEvals_.push(
        {req.id,
         (uint32_t)atoi(req.callFrameId.c_str()),
         req.expression,
         req.objectGroup,
         req.returnByValue,
         req.generatePreview,
         {}});
  }
  triggerAsyncPause();
}

void CDPHandler::Impl::sendSnapshot(
    int reqId,
    std::string /*message*/,
    bool reportProgress,
    bool stopStackTraceCapture) {
  enqueueFunc([this, reqId, reportProgress, stopStackTraceCapture]() {
    // Stop taking any new traces before sending out the heap
    // snapshot.
    if (stopStackTraceCapture) {
      getRuntime().instrumentation().stopTrackingHeapObjectStackTraces();
    }

    if (reportProgress) {
      // A progress notification with finished = true indicates the
      // snapshot has been captured and is ready to be sent.  Our
      // implementation streams the snapshot as it is being captured,
      // so we must send this notification first.
      m::heapProfiler::ReportHeapSnapshotProgressNotification note;
      note.done = 1;
      note.total = 1;
      note.finished = true;
      sendNotificationToClient(note);
    }

    // Size picked to conform to Chrome's own implementation, at the
    // time of writing.
    inspector::chrome::CallbackOStream cos(
        /* sz */ 100 << 10, [this](std::string s) {
          m::heapProfiler::AddHeapSnapshotChunkNotification note;
          note.chunk = std::move(s);
          sendNotificationToClient(note);
          return true;
        });

    getRuntime().instrumentation().createSnapshotToStream(cos);
    sendResponseToClient(m::makeOkResponse(reqId));
  });
}

void CDPHandler::Impl::handle(
    const m::heapProfiler::TakeHeapSnapshotRequest &req) {
  sendSnapshot(
      req.id,
      "HeapSnapshot.takeHeapSnapshot",
      req.reportProgress && *req.reportProgress,
      /* stopStackTraceCapture */ false);
}

void CDPHandler::Impl::handle(
    const m::heapProfiler::StartTrackingHeapObjectsRequest &req) {
  // TODO: Convert
  enqueueFunc([this, req]() {
    // const auto id = req.id;
    getRuntime().instrumentation().startTrackingHeapObjectStackTraces(
        [this](
            uint64_t lastSeenObjectId,
            std::chrono::microseconds timestamp,
            std::vector<jsi::Instrumentation::HeapStatsUpdate> stats) {
          // Send the last object ID notification first.
          m::heapProfiler::LastSeenObjectIdNotification note;
          note.lastSeenObjectId = lastSeenObjectId;
          // The protocol uses milliseconds with a fraction for
          // microseconds.
          note.timestamp = static_cast<double>(timestamp.count()) / 1000;
          sendNotificationToClient(note);

          m::heapProfiler::HeapStatsUpdateNotification heapStatsNote;
          // Flatten the HeapStatsUpdate list.
          heapStatsNote.statsUpdate.reserve(stats.size() * 3);
          for (const jsi::Instrumentation::HeapStatsUpdate &fragment : stats) {
            // Each triplet is the fragment number, the total count of
            // objects for the fragment, and the total size of objects
            // for the fragment.
            heapStatsNote.statsUpdate.push_back(
                static_cast<int>(std::get<0>(fragment)));
            heapStatsNote.statsUpdate.push_back(
                static_cast<int>(std::get<1>(fragment)));
            heapStatsNote.statsUpdate.push_back(
                static_cast<int>(std::get<2>(fragment)));
          }
          assert(
              heapStatsNote.statsUpdate.size() == stats.size() * 3 &&
              "Should be exactly 3x the stats vector");
          // TODO: Chunk this if there are too many fragments to
          // update. Unlikely to be a problem in practice unless
          // there's a huge amount of allocation and freeing.
          sendNotificationToClient(heapStatsNote);
        });
    // At this point we need the equivalent of a setInterval, where
    // each interval samples the existing
    sendResponseToClient(m::makeOkResponse(req.id));
  });
}

void CDPHandler::Impl::handle(
    const m::heapProfiler::StopTrackingHeapObjectsRequest &req) {
  sendSnapshot(
      req.id,
      "HeapSnapshot.stopTrackingHeapObjects",
      req.reportProgress && *req.reportProgress,
      /* stopStackTraceCapture */ true);
}

void CDPHandler::Impl::handle(
    const m::heapProfiler::StartSamplingRequest &req) {
  // This is the same default sampling interval that Chrome uses.
  // https://chromedevtools.github.io/devtools-protocol/tot/HeapProfiler/#method-startSampling
  constexpr size_t kDefaultSamplingInterval = 1 << 15;
  const size_t samplingInterval =
      req.samplingInterval.value_or(kDefaultSamplingInterval);

  // TODO: IfEnabled
  enqueueFunc([this, req, samplingInterval]() {
    getRuntime().instrumentation().startHeapSampling(samplingInterval);
    sendResponseToClient(m::makeOkResponse(req.id));
  });
}

void CDPHandler::Impl::handle(const m::heapProfiler::StopSamplingRequest &req) {
  // TODO: IfEnabled
  enqueueFunc([this, req]() {
    std::ostringstream stream;
    getRuntime().instrumentation().stopHeapSampling(stream);

    m::heapProfiler::StopSamplingResponse resp;
    auto profile = m::heapProfiler::makeSamplingHeapProfile(stream.str());
    if (profile == nullptr) {
      throw std::runtime_error("Failed to make SamplingHeapProfile");
    }
    resp.id = req.id;
    resp.profile = std::move(*profile);
    sendResponseToClient(resp);
  });
}

void CDPHandler::Impl::handle(
    const m::heapProfiler::CollectGarbageRequest &req) {
  enqueueFunc([this, req]() {
    getRuntime().instrumentation().collectGarbage("inspector");
    sendResponseToClient(m::makeOkResponse(req.id));
  });
}

void CDPHandler::Impl::handle(
    const m::heapProfiler::GetObjectByHeapObjectIdRequest &req) {
  enqueueFunc([this, req]() {
    uint64_t objID = atoi(req.objectId.c_str());
    std::optional<std::string> group = req.objectGroup;
    auto remoteObjPtr = std::make_shared<m::runtime::RemoteObject>();
    jsi::Runtime *rt = &getRuntime();
    if (auto *hermesRT = dynamic_cast<HermesRuntime *>(rt)) {
      jsi::Value val = hermesRT->getObjectForID(objID);
      if (val.isNull()) {
        return;
      }
      *remoteObjPtr = m::runtime::makeRemoteObject(
          getRuntime(), val, objTable_, group.value_or(""), false, false);
    }
    if (!remoteObjPtr->type.empty()) {
      m::heapProfiler::GetObjectByHeapObjectIdResponse resp;
      resp.id = req.id;
      resp.result = std::move(*remoteObjPtr);
      sendResponseToClient(resp);
    } else {
      sendResponseToClient(m::makeErrorResponse(
          req.id, m::ErrorCode::ServerError, "Object is not available"));
    }
  });
}

void CDPHandler::Impl::handle(
    const m::heapProfiler::GetHeapObjectIdRequest &req) {
  enqueueFunc([this, req]() {
    // TODO: de-shared_ptr this
    std::shared_ptr<uint64_t> snapshotID = std::make_shared<uint64_t>(0);
    if (const jsi::Value *valuePtr = objTable_.getValue(req.objectId)) {
      jsi::Runtime *rt = &getRuntime();
      if (auto *hermesRT = dynamic_cast<HermesRuntime *>(rt)) {
        *snapshotID = hermesRT->getUniqueID(*valuePtr);
      }
    }
    if (*snapshotID) {
      m::heapProfiler::GetHeapObjectIdResponse resp;
      resp.id = req.id;
      // std::to_string is not available on Android, use a std::ostream
      // instead.
      std::ostringstream stream;
      stream << *snapshotID;
      resp.heapSnapshotObjectId = stream.str();
      sendResponseToClient(resp);
    } else {
      sendResponseToClient(m::makeErrorResponse(
          req.id, m::ErrorCode::ServerError, "Object is not available"));
    }
  });
}

void CDPHandler::Impl::handle(const m::profiler::StartRequest &req) {
  enqueueFunc([this, req]() {
    HermesRuntime::enableSamplingProfiler();
    sendResponseToClient(m::makeOkResponse(req.id));
  });
}

void CDPHandler::Impl::handle(const m::profiler::StopRequest &req) {
  enqueueFunc([this, req]() {
    HermesRuntime *hermesRT = &getRuntime();

    HermesRuntime::disableSamplingProfiler();

    std::ostringstream profileStream;
    hermesRT->sampledTraceToStreamInDevToolsFormat(profileStream);

    // Hermes can emit the proper format directly, but it still needs to
    // be parsed into a dynamic.
    try {
      m::profiler::StopResponse resp;
      resp.id = req.id;
      auto profile = m::profiler::makeProfile(std::move(profileStream).str());
      if (profile == nullptr) {
        throw std::runtime_error("Failed to make Profile");
      }
      resp.profile = std::move(*profile);
      sendResponseToClient(resp);
    } catch (const std::exception &) {
      sendResponseToClient(m::makeErrorResponse(
          req.id,
          m::ErrorCode::InternalError,
          "Hermes profile output could not be parsed."));
    }
  });
}

namespace {
/// Runtime.CallArguments can have their values specified "inline", or they
/// can have remote references. The inline values are eval'd together with the
/// Runtime.CallFunctionOn.functionDeclaration (see CallFunctionOnBuilder
/// below), while remote object Ids need to be resolved outside of the VM.
class CallFunctionOnArgument {
 public:
  explicit CallFunctionOnArgument(
      std::optional<m::runtime::RemoteObjectId> maybeObjectId)
      : maybeObjectId_(std::move(maybeObjectId)) {}

  /// Computes the real value for this argument, which can be an object
  /// referenced by maybeObjectId_, or the given evaldValue. Throws if
  /// maybeObjectId_ is not empty but references an unknown object.
  jsi::Value value(
      jsi::Runtime &rt,
      RemoteObjectsTable &objTable,
      jsi::Value evaldValue) const {
    if (maybeObjectId_) {
      assert(evaldValue.isUndefined() && "expected undefined placeholder");
      return getValueFromId(rt, objTable, *maybeObjectId_);
    }

    return evaldValue;
  }

 private:
  /// Returns the jsi::Object for the given objId. Throws if such object can't
  /// be found.
  static jsi::Value getValueFromId(
      jsi::Runtime &rt,
      RemoteObjectsTable &objTable,
      m::runtime::RemoteObjectId objId) {
    if (const jsi::Value *ptr = objTable.getValue(objId)) {
      return jsi::Value(rt, *ptr);
    }

    throw std::runtime_error("unknown object id " + objId);
  }

  std::optional<m::runtime::RemoteObjectId> maybeObjectId_;
};

/// Functor that should be used to run the result of eval-ing a CallFunctionOn
/// request.
class CallFunctionOnRunner {
 public:
  static constexpr size_t kJsThisIndex = 0;
  static constexpr size_t kFirstArgIndex = 1;

  // N.B.: constexpr char[] broke react-native-oss-android.
  static const char *kJsThisArgPlaceholder;

  CallFunctionOnRunner() = default;
  CallFunctionOnRunner(CallFunctionOnRunner &&) = default;
  CallFunctionOnRunner &operator=(CallFunctionOnRunner &&) = default;

  /// Performs the actual Runtime.CallFunctionOn request. It assumes.
  /// \p evalResult is the result of invoking the Inspector's evaluate()
  /// method on the expression built by the CallFunctionOnBuilder below.
  jsi::Value operator()(
      jsi::Runtime &rt,
      RemoteObjectsTable &objTable,
      const facebook::hermes::debugger::EvalResult &evalResult) {
    // The eval result is an array [a0, a1, ..., an, func] (see
    // CallFunctionOnBuilder below).
    auto argsAndFunc = evalResult.value.getObject(rt).getArray(rt);
    assert(
        argsAndFunc.length(rt) == thisAndArguments_.size() + 1 &&
        "Unexpected result size");

    // now resolve the arguments to the call, including "this".
    std::vector<jsi::Value> arguments(thisAndArguments_.size() - 1);

    jsi::Object jsThis =
        getJsThis(rt, objTable, argsAndFunc.getValueAtIndex(rt, kJsThisIndex));

    size_t i = kFirstArgIndex;
    for (/*i points to the first param*/; i < thisAndArguments_.size(); ++i) {
      arguments[i - kFirstArgIndex] = thisAndArguments_[i].value(
          rt, objTable, argsAndFunc.getValueAtIndex(rt, i));
    }

    // i is now func's index.
    jsi::Function func =
        argsAndFunc.getValueAtIndex(rt, i).getObject(rt).getFunction(rt);

    return func.callWithThis(
        rt,
        std::move(jsThis),
        static_cast<const jsi::Value *>(arguments.data()),
        arguments.size());
  }

 private:
  friend class CallFunctionOnBuilder;

  CallFunctionOnRunner(const CallFunctionOnRunner &) = delete;
  CallFunctionOnRunner &operator=(const CallFunctionOnRunner &) = delete;

  CallFunctionOnRunner(
      std::vector<CallFunctionOnArgument> thisAndArguments,
      std::optional<m::runtime::ExecutionContextId> executionContextId)
      : thisAndArguments_(std::move(thisAndArguments)),
        executionContextId_(std::move(executionContextId)) {}

  /// Resolves the js "this" for the request, which lives in
  /// thisAndArguments_[kJsThisIndex]. \p evaldThis should either be
  /// undefined, or the placeholder indicating that globalThis should be used.
  jsi::Object getJsThis(
      jsi::Runtime &rt,
      RemoteObjectsTable &objTable,
      jsi::Value evaldThis) const {
    // In the future we may support multiple execution context ids; for now,
    // there's only one.
    (void)executionContextId_;

    // Either evaldThis is undefined (because the request had an object id
    // specifying "this"), or it should be a string (i.e., the placeholder
    // kJsThisArgPlaceholder).
    assert(
        (evaldThis.isUndefined() ||
         (evaldThis.isString() &&
          evaldThis.getString(rt).utf8(rt) == kJsThisArgPlaceholder)) &&
        "unexpected value for jsThis argument placeholder");

    // Need to save this information because of the std::move() below.
    const bool useGlobalThis = evaldThis.isString();
    jsi::Value value = thisAndArguments_[kJsThisIndex].value(
        rt, objTable, std::move(evaldThis));

    return useGlobalThis ? rt.global() : value.getObject(rt);
  }

  std::vector<CallFunctionOnArgument> thisAndArguments_;
  std::optional<m::runtime::ExecutionContextId> executionContextId_;
};

/*static*/ const char *CallFunctionOnRunner::kJsThisArgPlaceholder =
    "jsThis is Execution Context";

/// Returns true if \p str is a number-like string value (e.g., Infinity),
/// and false otherwise.
bool unserializableValueLooksLikeNumber(const std::string &str) {
  return str == "Infinity" || str == "-Infinity" || str == "NaN";
}

/// Helper class that processes a Runtime.CallFunctionOn request, and
/// builds an expression string that, once eval()d, yields an Array with the
/// CallArguments as well as the function to run. The generated array is
///
/// [JsThis, P0, P1, P2, P3, Pn, F]
///
/// where:
///   * F is the functionDeclaration in the request
///   * JsThis is either:
///      * undefined (if the request has an object ID); or
///      * the placeholder kJsThisArgPlaceholder
///   * Pi is either:
///      * the string in CallArgument[i].unserializableValue; or
///      * the string in CallArgument[i].value; or
///      * arguments[j] (i.e., the j-th argument passed to the newly built
///        function), j being the j-th CallArgument with an ObjectId. This is
///        needed because there's no easy way to express the objects referred
///        to by object ids by name.
class CallFunctionOnBuilder {
 public:
  explicit CallFunctionOnBuilder(const m::runtime::CallFunctionOnRequest &req)
      : executionContextId_(req.executionContextId) {
    out_ << "[";
    thisAndArguments_.emplace_back(CallFunctionOnArgument(req.objectId));
    if (req.objectId) {
      out_ << "undefined, ";
    } else {
      out_ << '\'' << CallFunctionOnRunner::kJsThisArgPlaceholder << "', ";
    }

    addParams(req.arguments);
    out_ << req.functionDeclaration;
    out_ << "]";
  };

  /// Extracts the functions that handles the CallFunctionOn requests, as well
  /// as the list of object ids that must be passed when calling it.
  std::pair<std::string, CallFunctionOnRunner> expressionAndRunner() && {
    return std::make_pair(
        std::move(out_).str(),
        CallFunctionOnRunner(
            std::move(thisAndArguments_), std::move(executionContextId_)));
  }

 private:
  void addParams(const std::optional<std::vector<m::runtime::CallArgument>>
                     &maybeArguments) {
    if (maybeArguments) {
      for (const auto &ca : *maybeArguments) {
        addParam(ca);
        thisAndArguments_.emplace_back(CallFunctionOnArgument(ca.objectId));
        out_ << ", ";
      }
    }
  }

  void addParam(const m::runtime::CallArgument &ca) {
    if (ca.objectId) {
      out_ << "undefined";
    } else if (ca.value) {
      // TODO: this may throw if ca.value is a CBOR (see RFC 8949), but the
      // chrome debugger doesn't seem to send those.
      out_ << "(" << *ca.value << ")";
    } else if (ca.unserializableValue) {
      if (unserializableValueLooksLikeNumber(*ca.unserializableValue)) {
        out_ << "+(" << *ca.unserializableValue << ")";
      } else {
        out_ << *ca.unserializableValue;
      }
    } else {
      throw std::runtime_error("unknown payload for CallParam");
    }
  }

  std::ostringstream out_;

  std::vector<CallFunctionOnArgument> thisAndArguments_;
  std::optional<m::runtime::ExecutionContextId> executionContextId_;
};

} // namespace

void CDPHandler::Impl::handle(const m::runtime::CallFunctionOnRequest &req) {
  std::string expression;
  CallFunctionOnRunner runner;

  auto validateAndParseRequest =
      [&expression, &runner](const m::runtime::CallFunctionOnRequest &req)
      -> std::optional<std::string> {
    if (req.objectId.has_value() == req.executionContextId.has_value()) {
      return std::string(
          "The request must specify either object id or execution context id.");
    }

    if (!req.objectId) {
      assert(
          req.executionContextId &&
          "should not be here if both object id and execution context id are missing");
      if (*req.executionContextId != kHermesExecutionContextId) {
        return "unknown execution context id " +
            std::to_string(*req.executionContextId);
      }
    }

    try {
      std::tie(expression, runner) =
          CallFunctionOnBuilder(req).expressionAndRunner();
    } catch (const std::exception &e) {
      return std::string(e.what());
    }

    return {};
  };

  if (auto errMsg = validateAndParseRequest(req)) {
    sendErrorToClient(req.id, *errMsg);
    return;
  }

  // std::function needs a copy-able type.
  auto sharedRunner = std::make_shared<CallFunctionOnRunner>(std::move(runner));
  {
    std::lock_guard<std::mutex> lock(mutex_);
    pendingEvals_.push(
        {req.id,
         0, // top of the stackframe
         expression,
         req.objectGroup,
         req.returnByValue,
         req.generatePreview,
         [this,
          objectGroup = req.objectGroup.value_or("ConsoleObjectGroup"),
          byValue = req.returnByValue.value_or(false),
          generatePreview = req.generatePreview.value_or(false),
          sharedRunner](
             std::shared_ptr<m::runtime::RemoteObject> remoteObjPtr,
             const facebook::hermes::debugger::EvalResult &evalResult) mutable {
           if (evalResult.isException) {
             return;
           }

           *remoteObjPtr = m::runtime::makeRemoteObject(
               getRuntime(),
               (*sharedRunner)(getRuntime(), objTable_, evalResult),
               objTable_,
               objectGroup,
               byValue,
               generatePreview);
         }});
  }
  triggerAsyncPause();
}

void CDPHandler::Impl::handle(const m::runtime::CompileScriptRequest &req) {
  // TODO: formerly IfEnabled
  enqueueFunc([this, req]() {
    if (!validateExecutionContext(req.id, req.executionContextId)) {
      return;
    }

    m::runtime::CompileScriptResponse resp;
    resp.id = req.id;

    auto source = std::make_shared<jsi::StringBuffer>(req.expression);
    std::shared_ptr<const jsi::PreparedJavaScript> preparedScript;
    try {
      preparedScript = getRuntime().prepareJavaScript(source, req.sourceURL);
    } catch (const facebook::jsi::JSIException &err) {
      resp.exceptionDetails = m::runtime::ExceptionDetails();
      resp.exceptionDetails->text = err.what();
      return;
    }

    if (req.persistScript) {
      auto scriptId =
          kUserEnteredScriptPrefix + std::to_string(preparedScripts_.size());
      preparedScripts_.push_back(std::move(preparedScript));
      resp.scriptId = scriptId;
    }
    sendResponseToClient(resp);
  });
}

void CDPHandler::Impl::handle(const m::runtime::DisableRequest &req) {
  enqueueFunc([this, req]() {
    runtimeEnabled_ = false;
    sendResponseToClient(m::makeOkResponse(req.id));
  });
}

void CDPHandler::Impl::handle(const m::runtime::EnableRequest &req) {
  enqueueFunc([this, req]() {
    runtimeEnabled_ = true;
    sendResponseToClient(m::makeOkResponse(req.id));

    // Per CDP spec, when reporting is enabled, immediately send an
    // executionContextCreated event for each existing execution context.
    m::runtime::ExecutionContextCreatedNotification note;
    note.context.id = CDPHandler::Impl::kHermesExecutionContextId;
    note.context.name = "hermes";
    sendNotificationToClient(note);

    if (numPendingConsoleMessagesDiscarded_ != 0) {
      jsi::Runtime &rt = getRuntime();
      std::ostringstream oss;
      oss << "Too many console messages were logged before devtool connected. "
          << numPendingConsoleMessagesDiscarded_
          << (numPendingConsoleMessagesDiscarded_ == 1 ? " message was"
                                                       : " messages were")
          << " discarded at the beginning.";
      jsi::Array argsArray(rt, 1);
      argsArray.setValueAtIndex(rt, 0, oss.str());
      emitConsoleAPICalledEvent(ConsoleMessageInfo{
          pendingConsoleMessages_.front().timestamp - 0.1,
          "warning",
          std::move(argsArray)});

      numPendingConsoleMessagesDiscarded_ = 0;
    }

    for (auto &msg : pendingConsoleMessages_) {
      emitConsoleAPICalledEvent(std::move(msg));
    }
    pendingConsoleMessages_.clear();
  });
}

void CDPHandler::Impl::handle(const m::runtime::EvaluateRequest &req) {
  {
    std::lock_guard<std::mutex> lock(mutex_);
    pendingEvals_.push(
        {req.id,
         0, // Top of the stackframe
         req.expression,
         req.objectGroup,
         req.returnByValue,
         req.generatePreview,
         {}});
  }
  triggerAsyncPause();
}

void CDPHandler::Impl::handle(const m::debugger::PauseRequest &req) {
  enqueueDesiredExecution(req, Execution::Paused);
  triggerAsyncPause();
}

void CDPHandler::Impl::handle(const m::debugger::RemoveBreakpointRequest &req) {
  enqueueFunc([this, req]() {
    if (isVirtualBreakpointId(req.breakpointId)) {
      std::lock_guard<std::mutex> lock(virtualBreakpointMutex_);
      if (!removeVirtualBreakpoint(req.breakpointId)) {
        sendErrorToClient(req.id, "Unknown breakpoint ID: " + req.breakpointId);
      }
    } else {
      getDebugger().deleteBreakpoint(std::stoull(req.breakpointId));
    }
    sendResponseToClient(m::makeOkResponse(req.id));
  });
}

void CDPHandler::Impl::handle(const m::debugger::ResumeRequest &req) {
  enqueueDesiredExecution(req, Execution::Running);
  triggerAsyncPause();
}

void CDPHandler::Impl::handle(const m::debugger::SetBreakpointRequest &req) {
  debugger::SourceLocation loc;
  // TODO: failure to parse
  auto scriptId = std::stoull(req.location.scriptId);
  loc.fileId = scriptId;
  // CDP Locations are 0-based, Hermes lines/columns are 1-based
  loc.line = req.location.lineNumber + 1;
  if (req.location.columnNumber) {
    loc.column = req.location.columnNumber.value() + 1;
  }
  enqueueFunc([this, id = req.id, condition = req.condition, loc]() {
    debugger::BreakpointID bpID = getDebugger().setBreakpoint(loc);
    debugger::BreakpointInfo info{debugger::kInvalidBreakpoint, {}, {}, {}, {}};
    if (bpID != debugger::kInvalidBreakpoint) {
      info = getDebugger().getBreakpointInfo(bpID);
      if (condition) {
        getDebugger().setBreakpointCondition(bpID, condition.value());
      }
    }

    m::debugger::SetBreakpointResponse resp;
    resp.id = id;
    resp.breakpointId = std::to_string(info.id);
    if (info.resolved) {
      resp.actualLocation = m::debugger::makeLocation(info.resolvedLocation);
    }

    // Automatically re-enable breakpoints since the user presumably wants this
    // to start triggering.
    breakpointsActive_ = true;

    sendResponseToClient(resp);
  });
}

void CDPHandler::Impl::handle(
    const m::debugger::SetBreakpointByUrlRequest &req) {
  enqueueFunc([this, req]() {
    debugger::SourceLocation loc;
    // TODO: getLocationByBreakpointRequest(req);
    // TODO: failure to parse
    setHermesLocation(loc, req);
    debugger::BreakpointID id = getDebugger().setBreakpoint(loc);
    debugger::BreakpointInfo info{debugger::kInvalidBreakpoint, {}, {}, {}, {}};
    if (id != debugger::kInvalidBreakpoint) {
      info = getDebugger().getBreakpointInfo(id);
      if (req.condition) {
        getDebugger().setBreakpointCondition(id, req.condition.value());
      }
    }
    m::debugger::SetBreakpointByUrlResponse resp;
    resp.id = req.id;
    resp.breakpointId = std::to_string(info.id);
    if (info.resolved) {
      resp.locations.emplace_back(
          m::debugger::makeLocation(info.resolvedLocation));
    }
    sendResponseToClient(resp);
  });
}

void CDPHandler::Impl::handle(
    const m::debugger::SetBreakpointsActiveRequest &req) {
  breakpointsActive_ = req.active;
  sendResponseToClient(m::makeOkResponse(req.id));
}

bool CDPHandler::Impl::isVirtualBreakpointId(const std::string &id) {
  return id.rfind(kVirtualBreakpointPrefix, 0) == 0;
}

const std::string &CDPHandler::Impl::createVirtualBreakpoint(
    const std::string &category) {
  auto ret = virtualBreakpoints_[category].insert(
      kVirtualBreakpointPrefix + std::to_string(nextVirtualBreakpoint_++));
  return *ret.first;
}

bool CDPHandler::Impl::hasVirtualBreakpoint(const std::string &category) {
  auto pos = virtualBreakpoints_.find(category);
  if (pos == virtualBreakpoints_.end())
    return false;
  return !pos->second.empty();
}

bool CDPHandler::Impl::removeVirtualBreakpoint(const std::string &id) {
  // We expect roughly 1 category, so just iterate over all the sets
  for (auto &kv : virtualBreakpoints_) {
    if (kv.second.erase(id) > 0) {
      return true;
    }
  }
  return false;
}

void CDPHandler::Impl::handle(
    const m::debugger::SetInstrumentationBreakpointRequest &req) {
  if (req.instrumentation != kBeforeScriptWithSourceMapExecution) {
    sendErrorToClient(
        req.id, "Unknown instrumentation breakpoint: " + req.instrumentation);
    return;
  }

  // The act of creating and registering the breakpoint ID is enough
  // to "set" it. We merely check for the existence of them later.
  std::lock_guard<std::mutex> lock(virtualBreakpointMutex_);
  m::debugger::SetInstrumentationBreakpointResponse resp;
  resp.id = req.id;
  resp.breakpointId = createVirtualBreakpoint(req.instrumentation);
  sendResponseToClient(resp);
}

void CDPHandler::Impl::handle(
    const m::debugger::SetPauseOnExceptionsRequest &req) {
  debugger::PauseOnThrowMode mode = debugger::PauseOnThrowMode::None;

  if (req.state == "none") {
    mode = debugger::PauseOnThrowMode::None;
  } else if (req.state == "all") {
    mode = debugger::PauseOnThrowMode::All;
  } else if (req.state == "uncaught") {
    mode = debugger::PauseOnThrowMode::Uncaught;
  } else {
    sendErrorToClient(req.id, "Unknown pause-on-exception state: " + req.state);
    return;
  }

  enqueueFunc([this, req, mode]() {
    getDebugger().setPauseOnThrowMode(mode);
    sendResponseToClient(m::makeOkResponse(req.id));
  });
}

void CDPHandler::Impl::handle(const m::debugger::StepIntoRequest &req) {
  enqueueDesiredStep(req, debugger::StepMode::Into);
}

void CDPHandler::Impl::handle(const m::debugger::StepOutRequest &req) {
  enqueueDesiredStep(req, debugger::StepMode::Out);
}

void CDPHandler::Impl::handle(const m::debugger::StepOverRequest &req) {
  enqueueDesiredStep(req, debugger::StepMode::Over);
}

std::vector<m::runtime::PropertyDescriptor>
CDPHandler::Impl::makePropsFromScope(
    std::pair<uint32_t, uint32_t> frameAndScopeIndex,
    const std::string &objectGroup,
    const debugger::ProgramState &state,
    bool generatePreview) {
  // Chrome represents variables in a scope as properties on a dummy object.
  // We don't instantiate such dummy objects, we just pretended to have one.
  // Chrome has now asked for its properties, so it's time to synthesize
  // descriptions of the properties that the dummy object would have had.
  std::vector<m::runtime::PropertyDescriptor> result;

  uint32_t frameIndex = frameAndScopeIndex.first;
  uint32_t scopeIndex = frameAndScopeIndex.second;
  debugger::LexicalInfo lexicalInfo = state.getLexicalInfo(frameIndex);
  uint32_t varCount = lexicalInfo.getVariablesCountInScope(scopeIndex);

  // If this is the frame's local scope, include 'this'.
  if (scopeIndex == 0) {
    auto varInfo = state.getVariableInfoForThis(frameIndex);
    m::runtime::PropertyDescriptor desc;
    desc.name = varInfo.name;
    desc.value = m::runtime::makeRemoteObject(
        getRuntime(),
        varInfo.value,
        objTable_,
        objectGroup,
        false,
        generatePreview);
    // Chrome only shows enumerable properties.
    desc.enumerable = true;
    result.emplace_back(std::move(desc));
  }

  // Then add each of the variables in this lexical scope.
  for (uint32_t varIndex = 0; varIndex < varCount; varIndex++) {
    debugger::VariableInfo varInfo =
        state.getVariableInfo(frameIndex, scopeIndex, varIndex);

    m::runtime::PropertyDescriptor desc;
    desc.name = varInfo.name;
    desc.value = m::runtime::makeRemoteObject(
        getRuntime(),
        varInfo.value,
        objTable_,
        objectGroup,
        false,
        generatePreview);
    desc.enumerable = true;

    result.emplace_back(std::move(desc));
  }

  return result;
}

std::vector<m::runtime::PropertyDescriptor>
CDPHandler::Impl::makePropsFromValue(
    const jsi::Value &value,
    const std::string &objectGroup,
    bool onlyOwnProperties,
    bool generatePreview) {
  std::vector<m::runtime::PropertyDescriptor> result;

  if (value.isObject()) {
    jsi::Runtime &runtime = getRuntime();
    jsi::Object obj = value.getObject(runtime);

    // TODO(hypuk): obj.getPropertyNames only returns enumerable properties.
    jsi::Array propNames = onlyOwnProperties
        ? runtime.global()
              .getPropertyAsObject(runtime, "Object")
              .getPropertyAsFunction(runtime, "getOwnPropertyNames")
              .call(runtime, obj)
              .getObject(runtime)
              .getArray(runtime)
        : obj.getPropertyNames(runtime);

    size_t propCount = propNames.length(runtime);
    for (size_t i = 0; i < propCount; i++) {
      jsi::String propName =
          propNames.getValueAtIndex(runtime, i).getString(runtime);

      m::runtime::PropertyDescriptor desc;
      desc.name = propName.utf8(runtime);

      try {
        // Currently, we fetch the property even if it runs code.
        // Chrome instead detects getters and makes you click to invoke.
        jsi::Value propValue = obj.getProperty(runtime, propName);
        desc.value = m::runtime::makeRemoteObject(
            runtime, propValue, objTable_, objectGroup, false, generatePreview);
      } catch (const jsi::JSError &err) {
        // We fetched a property with a getter that threw. Show a placeholder.
        // We could have added additional info, but the UI quickly gets messy.
        desc.value = m::runtime::makeRemoteObject(
            runtime,
            jsi::String::createFromUtf8(runtime, "(Exception)"),
            objTable_,
            objectGroup,
            false,
            generatePreview);
      }

      result.emplace_back(std::move(desc));
    }

    if (onlyOwnProperties) {
      jsi::Value proto = runtime.global()
                             .getPropertyAsObject(runtime, "Object")
                             .getPropertyAsFunction(runtime, "getPrototypeOf")
                             .call(runtime, obj);
      if (!proto.isNull()) {
        m::runtime::PropertyDescriptor desc;
        desc.name = "__proto__";
        desc.value = m::runtime::makeRemoteObject(
            runtime, proto, objTable_, objectGroup, false, generatePreview);
        result.emplace_back(std::move(desc));
      }
    }
  }

  return result;
}

void CDPHandler::Impl::handle(const m::runtime::GetHeapUsageRequest &req) {
  enqueueFunc([this, req]() {
    // getHeapInfo must be called from the runtime thread.
    auto heapInfo = getRuntime().instrumentation().getHeapInfo(false);
    auto resp = std::make_shared<m::runtime::GetHeapUsageResponse>();
    resp->id = req.id;
    resp->usedSize = heapInfo["hermes_allocatedBytes"];
    resp->totalSize = heapInfo["hermes_heapSize"];
    sendResponseToClient(*resp);
  });
}

void CDPHandler::Impl::handle(const m::runtime::GetPropertiesRequest &req) {
  // TODO: formerly "IfEnabled"
  enqueueFunc(
      [this, req, generatePreview = req.generatePreview.value_or(false)](
          const debugger::ProgramState &state) {
        std::string objGroup = objTable_.getObjectGroup(req.objectId);
        auto scopePtr = objTable_.getScope(req.objectId);
        auto valuePtr = objTable_.getValue(req.objectId);

        m::runtime::GetPropertiesResponse resp;
        resp.id = req.id;
        if (scopePtr != nullptr) {
          resp.result =
              makePropsFromScope(*scopePtr, objGroup, state, generatePreview);
        } else if (valuePtr != nullptr) {
          resp.result = makePropsFromValue(
              *valuePtr,
              objGroup,
              req.ownProperties.value_or(true),
              generatePreview);
        }
        sendResponseToClient(resp);
      });
}

void CDPHandler::Impl::handle(
    const m::runtime::GlobalLexicalScopeNamesRequest &req) {
  // TODO: formerly "IfEnabled"
  enqueueFunc([this, req](const debugger::ProgramState &state) {
    m::runtime::GlobalLexicalScopeNamesResponse resp;
    resp.id = req.id;

    if (!validateExecutionContext(req.id, req.executionContextId)) {
      return;
    }

    const debugger::LexicalInfo &lexicalInfo = state.getLexicalInfo(0);
    debugger::ScopeDepth scopeCount = lexicalInfo.getScopesCount();
    if (scopeCount == 0) {
      return;
    }

    const debugger::ScopeDepth globalScopeIndex = scopeCount - 1;
    uint32_t variableCount =
        lexicalInfo.getVariablesCountInScope(globalScopeIndex);
    resp.names.reserve(variableCount);
    for (uint32_t i = 0; i < variableCount; i++) {
      debugger::String name =
          state.getVariableInfo(0, globalScopeIndex, i).name;
      // The global scope has some entries prefixed with '?', which
      // are not valid identifiers.
      if (!name.empty() && name.front() != '?') {
        resp.names.push_back(name);
      }
    }

    sendResponseToClient(resp);
  });
}

void CDPHandler::Impl::handle(
    const m::runtime::RunIfWaitingForDebuggerRequest &req) {
  if (isAwaitingDebuggerOnStart()) {
    enqueueDesiredExecution(req, Execution::Running);
    triggerAsyncPause();
  } else {
    // We weren't awaiting a debugger. Just send an 'ok'.
    sendResponseToClient(m::makeOkResponse(req.id));
  }
}

/*
 * Send-to-client methods
 */

void CDPHandler::Impl::sendToClient(const std::string &str) {
  if (remoteConn_) {
    std::lock_guard<std::mutex> lock(remoteConnMutex_);
    remoteConn_->onMessage(str);
  }
}

void CDPHandler::Impl::sendResponseToClient(const m::Response &resp) {
  sendToClient(resp.toJsonStr());
}

void CDPHandler::Impl::sendNotificationToClient(const m::Notification &note) {
  sendToClient(note.toJsonStr());
}

bool CDPHandler::Impl::validateExecutionContext(
    int id,
    std::optional<m::runtime::ExecutionContextId> context) {
  if (context.has_value() && context.value() != kHermesExecutionContextId) {
    sendErrorToClient(id, "Invalid execution context");
    return false;
  }

  return true;
}

/*
 * CDPHandler
 */
CDPHandler::CDPHandler(
    std::unique_ptr<RuntimeAdapter> adapter,
    const std::string &title,
    bool waitForDebugger)
    : impl_(
          std::make_unique<Impl>(std::move(adapter), title, waitForDebugger)) {}

CDPHandler::~CDPHandler() = default;

HermesRuntime &CDPHandler::getRuntime() {
  return impl_->getRuntime();
}

std::string CDPHandler::getTitle() const {
  return impl_->getTitle();
}

bool CDPHandler::connect(std::unique_ptr<IRemoteConnection> remoteConn) {
  return impl_->connect(std::move(remoteConn));
}

bool CDPHandler::disconnect() {
  return impl_->disconnect();
}

void CDPHandler::sendMessage(std::string str) {
  impl_->sendMessage(std::move(str));
}

bool CDPHandler::Impl::isAwaitingDebuggerOnStart() {
  return awaitingDebuggerOnStart_;
}

void CDPHandler::Impl::triggerAsyncPause() {
  signal_.notify_one();
  getDebugger().triggerAsyncPause(debugger::AsyncPauseKind::Implicit);
  runtimeAdapter_->tickleJs();
}

void CDPHandler::Impl::sendErrorToClient(int id, const std::string &msg) {
  sendResponseToClient(makeErrorResponse(id, m::ErrorCode::ServerError, msg));
}
void CDPHandler::Impl::sendErrorCodeToClient(
    int id,
    m::ErrorCode errorCode,
    const std::string &msg) {
  sendResponseToClient(m::makeErrorResponse(id, errorCode, msg));
}

void CDPHandler::Impl::resetScriptsLoaded() {
  for (auto &[_, script] : loadedScripts_) {
    script.notifiedClient = false;
  }
}

void CDPHandler::Impl::sendPausedNotificationToClient() {
  processPendingScriptLoads();
  m::debugger::PausedNotification note;
  note.reason = "other";
  note.callFrames = m::debugger::makeCallFrames(
      getDebugger().getProgramState(), objTable_, getRuntime());
  sendNotificationToClient(note);
}

void CDPHandler::Impl::enqueueFunc(
    std::function<void(const debugger::ProgramState &state)> func) {
  std::lock_guard<std::mutex> lock(mutex_);
  pendingFuncs_.push(func);
  triggerAsyncPause();
}

// Convenience wrapper for funcs that don't need program state
void CDPHandler::Impl::enqueueFunc(std::function<void()> func) {
  std::lock_guard<std::mutex> lock(mutex_);
  pendingFuncs_.push([func](const debugger::ProgramState &) { func(); });
  triggerAsyncPause();
}

void CDPHandler::Impl::sendPauseOnExceptionNotification() {
  m::debugger::PausedNotification note;
  note.reason = "exception";
  note.callFrames = m::debugger::makeCallFrames(
      getDebugger().getProgramState(), objTable_, getRuntime());
  sendNotificationToClient(note);
}

void CDPHandler::Impl::processPendingFuncs() {
  std::lock_guard<std::mutex> lock(mutex_); // TODO: narrow
  while (!pendingFuncs_.empty()) {
    if (true) {
      auto func = pendingFuncs_.front();
      pendingFuncs_.pop();
      func(getDebugger().getProgramState());
    } else {
      sendResponseToClient(m::makeErrorResponse(
          -1, // TODO: real id
          m::ErrorCode::InvalidRequest,
          "Already in desired attachment state"));
    }
  }
}

void CDPHandler::Impl::processPendingDesiredAttachments() {
  std::lock_guard<std::mutex> lock(mutex_); // TODO: narrow
  while (!pendingDesiredAttachments_.empty()) {
    int requestId;
    Attachment desiredAttachment;
    std::tie(requestId, desiredAttachment) = pendingDesiredAttachments_.front();
    pendingDesiredAttachments_.pop();
    currentAttachment_ = desiredAttachment;
    sendResponseToClient(m::makeOkResponse(requestId));
    if (desiredAttachment == Attachment::Enabled) {
      enableDebugger();
    } else {
      disableDebugger();
    }
  }
}

void CDPHandler::Impl::enableDebugger() {
  getDebugger().setIsDebuggerAttached(true);
  if (currentExecution_ == Execution::Paused) {
    sendPausedNotificationToClient();
  }
}

void CDPHandler::Impl::disableDebugger() {
  getDebugger().setIsDebuggerAttached(false);
  resetScriptsLoaded();
  getDebugger().deleteAllBreakpoints();
  // If a user sends a Pause request right after they send a Disable request,
  // and they are handled within the same didPause callback, then the Pause
  // request will 'win' and the currentExecution_ will be set to paused.
  currentExecution_ = Execution::Running;
}

bool CDPHandler::Impl::isDebuggerDisabled() {
  return currentAttachment_ == Attachment::Disabled;
}

void CDPHandler::Impl::processPendingDesiredExecutions(
    debugger::PauseReason pauseReason) {
  std::lock_guard<std::mutex> lock(mutex_); // TODO: narrow
  Execution previousExecution = currentExecution_;
  while (!pendingDesiredExecutions_.empty()) {
    int requestId;
    Execution desiredExecution;
    std::tie(requestId, desiredExecution) = pendingDesiredExecutions_.front();
    pendingDesiredExecutions_.pop();
    sendResponseToClient(m::makeOkResponse(requestId));
    currentExecution_ = desiredExecution;
  }
  if (debugger::PauseReason::DebuggerStatement == pauseReason ||
      debugger::PauseReason::Breakpoint == pauseReason ||
      debugger::PauseReason::StepFinish == pauseReason) {
    currentExecution_ = Execution::Paused;
  }

  // If we paused because the debugger hit a breakpoint, but breakpoints are
  // currently disabled or the entire debugger is disabled, then just continue
  // on like normal.
  if (pauseReason == debugger::PauseReason::Breakpoint &&
      (!breakpointsActive_ || isDebuggerDisabled())) {
    currentExecution_ = Execution::Running;
    return;
  }

  // If the runtime paused on a script load, and we are awaiting a debugger on
  // start command, then we must pause.
  // We also pause on script loads when the appropriate virtual breakpoint has
  // been set.
  if (pauseReason == debugger::PauseReason::ScriptLoaded) {
    Script info = getScriptFromTopCallFrame();
    // We don't want to pause on files that we should be ignoring.
    if (isAwaitingDebuggerOnStart() ||
        (hasVirtualBreakpoint(kBeforeScriptWithSourceMapExecution) &&
         !info.sourceMappingUrl.empty())) {
      currentExecution_ = Execution::Paused;
      // We should only be sending notifications if a debugger is attached,
      // else there's no one there to receive them. This notification will be
      // sent correctly when the debugger is enabled.
      if (currentAttachment_ == Attachment::Enabled) {
        m::debugger::PausedNotification note;
        note.reason = "other";
        note.callFrames = m::debugger::makeCallFrames(
            getDebugger().getProgramState(), objTable_, getRuntime());
        {
          std::lock_guard<std::mutex> lock(virtualBreakpointMutex_);
          note.hitBreakpoints = std::vector<m::debugger::BreakpointId>();
          for (auto &bp :
               virtualBreakpoints_[kBeforeScriptWithSourceMapExecution]) {
            note.hitBreakpoints->emplace_back(bp);
          }
        }
        sendNotificationToClient(note);
      }
      return;
    }
  }

  if (pauseReason == debugger::PauseReason::Exception) {
    currentExecution_ = Execution::Paused;
    sendPauseOnExceptionNotification();
    return;
  }

  if (previousExecution != currentExecution_ &&
      currentAttachment_ == Attachment::Enabled) {
    if (currentExecution_ == Execution::Paused) {
      // n.b. gross. fix?
      sendPausedNotificationToClient();
    } else /* running */ {
      awaitingDebuggerOnStart_ = false;
      objTable_.releaseObjectGroup(BacktraceObjectGroup);
      sendNotificationToClient(m::debugger::ResumedNotification{});
    }
  }
}

void CDPHandler::Impl::processCurrentScriptLoaded() {
  Script info = getScriptFromTopCallFrame();
  auto loadedScript = loadedScripts_.find(info.fileId);
  if (loadedScript == loadedScripts_.end()) {
    loadedScripts_[info.fileId] = info;
    loadedScriptIdByName_[info.fileName] = info.fileId;
  }
}

void CDPHandler::Impl::processPendingScriptLoads() {
  if (Attachment::Enabled != currentAttachment_)
    return;
  for (auto &[_, info] : loadedScripts_) {
    if (!info.notifiedClient) {
      m::debugger::ScriptParsedNotification note;
      note.scriptId = std::to_string(info.fileId);
      note.url = info.fileName;
      note.executionContextId = kHermesExecutionContextId;
      if (!info.sourceMappingUrl.empty()) {
        note.sourceMapURL = info.sourceMappingUrl;
      }
      sendNotificationToClient(note);
      info.notifiedClient = true;
    }
  }
}

Script CDPHandler::Impl::getScriptFromTopCallFrame() {
  Script info{};
  auto stackTrace = getDebugger().getProgramState().getStackTrace();

  if (stackTrace.callFrameCount() > 0) {
    debugger::SourceLocation loc = stackTrace.callFrameForIndex(0).location;

    info.fileId = loc.fileId;
    info.fileName = loc.fileName;
    info.sourceMappingUrl = getDebugger().getSourceMappingUrl(info.fileId);
    info.notifiedClient = false;
  }

  return info;
}

debugger::Command CDPHandler::Impl::didPause(debugger::Debugger &debugger) {
  processPendingDesiredAttachments();

  if (getPauseReason() == debugger::PauseReason::ScriptLoaded) {
    processCurrentScriptLoaded();
  }

  processPendingScriptLoads();

  processPendingDesiredExecutions(getPauseReason());

  processPendingFuncs();

  if (getPauseReason() == debugger::PauseReason::EvalComplete) {
    auto evalReq = pendingEvals_.front();
    pendingEvals_.pop();
    auto remoteObjPtr = std::make_shared<m::runtime::RemoteObject>();
    auto evalResult = getDebugger().getProgramState().getEvalResult();

    if (evalReq.onEvalCompleteCallback) {
      (*(evalReq.onEvalCompleteCallback))(remoteObjPtr, evalResult);
    } else {
      std::string objectGroup = evalReq.objectGroup.value_or("");
      bool byValue = evalReq.returnByValue.value_or(false);
      bool generatePreview = evalReq.generatePreview.value_or(false);
      *remoteObjPtr = m::runtime::makeRemoteObject(
          getRuntime(),
          evalResult.value,
          objTable_,
          objectGroup,
          byValue,
          generatePreview);
    }

    m::debugger::EvaluateOnCallFrameResponse resp;
    resp.id = evalReq.id;
    if (evalResult.isException) {
      resp.exceptionDetails =
          m::runtime::makeExceptionDetails(evalResult.exceptionDetails);
    } else {
      resp.result = std::move(*remoteObjPtr);
    }
    sendResponseToClient(resp);
  }

  // Sometimes an eval request may come in while execution is still running. In
  // this case, we should respect the pending eval over giving the debugger a
  // continue command, otherwise this eval would never be serviced.
  {
    std::unique_lock<std::mutex> lock(mutex_);
    // There is currently a known issue with returning an eval command when the
    // runtime paused because of a script load. Therefore, we simply don't
    // process any evals if the pause reason is a script load.
    if (getPauseReason() != debugger::PauseReason::ScriptLoaded &&
        !pendingEvals_.empty()) {
      auto evalReq = pendingEvals_.front();
      return debugger::Command::eval(evalReq.expression, evalReq.frameIdx);
    }
  }

  if (Execution::Running == currentExecution_) {
    return debugger::Command::continueExecution();
  }

  while (true) {
    {
      std::unique_lock<std::mutex> lock(mutex_);
      while (pendingEvals_.empty() && pendingDesiredExecutions_.empty() &&
             pendingDesiredSteps_.empty() && pendingFuncs_.empty() &&
             pendingDesiredAttachments_.empty()) {
        signal_.wait(lock);
      }
    }
    processPendingDesiredAttachments();
    processPendingDesiredExecutions(
        (debugger::PauseReason)-1); // TOOD: no pause reason here?
    processPendingFuncs();
    {
      std::unique_lock<std::mutex> lock(mutex_);
      if (!pendingDesiredSteps_.empty()) {
        auto pair = pendingDesiredSteps_.front();
        pendingDesiredSteps_.pop();
        sendResponseToClient(m::makeOkResponse(pair.first));
        currentExecution_ = Execution::Running;
        sendNotificationToClient(m::debugger::ResumedNotification{});
        return debugger::Command::step(pair.second);
      }
      if (Execution::Running == currentExecution_) {
        decltype(pendingEvals_) empty;
        std::swap(pendingEvals_, empty);
        return debugger::Command::continueExecution();
      } else if (!pendingEvals_.empty()) {
        auto evalReq = pendingEvals_.front();
        // n.b. NOT popped, eval complete pops it
        return debugger::Command::eval(evalReq.expression, evalReq.frameIdx);
      }
    }
  }
}

static bool toBoolean(jsi::Runtime &runtime, const jsi::Value &val) {
  // Based on Operations.cpp:toBoolean in the Hermes VM.
  if (val.isUndefined() || val.isNull()) {
    return false;
  }
  if (val.isBool()) {
    return val.getBool();
  }
  if (val.isNumber()) {
    double m = val.getNumber();
    return m != 0 && !std::isnan(m);
  }
  if (val.isSymbol() || val.isObject()) {
    return true;
  }
  if (val.isString()) {
    std::string s = val.getString(runtime).utf8(runtime);
    return !s.empty();
  }
  assert(false && "All cases should be covered");
  return false;
}

void CDPHandler::Impl::installLogHandler() {
  jsi::Runtime &rt = getRuntime();
  auto console = jsi::Object(rt);
  auto val = rt.global().getProperty(rt, "console");
  std::shared_ptr<jsi::Object> originalConsole;
  if (val.isObject()) {
    originalConsole = std::make_shared<jsi::Object>(val.getObject(rt));
  }
  installConsoleFunction(console, originalConsole, "assert");
  installConsoleFunction(console, originalConsole, "clear");
  installConsoleFunction(console, originalConsole, "debug");
  installConsoleFunction(console, originalConsole, "dir");
  installConsoleFunction(console, originalConsole, "dirxml");
  installConsoleFunction(console, originalConsole, "error");
  installConsoleFunction(console, originalConsole, "group", "startGroup");
  installConsoleFunction(
      console, originalConsole, "groupCollapsed", "startGroupCollapsed");
  installConsoleFunction(console, originalConsole, "groupEnd", "endGroup");
  installConsoleFunction(console, originalConsole, "info");
  installConsoleFunction(console, originalConsole, "log");
  installConsoleFunction(console, originalConsole, "profile");
  installConsoleFunction(console, originalConsole, "profileEnd");
  installConsoleFunction(console, originalConsole, "table");
  installConsoleFunction(console, originalConsole, "trace");
  installConsoleFunction(console, originalConsole, "warn", "warning");
  rt.global().setProperty(rt, "console", console);
}

void CDPHandler::Impl::installConsoleFunction(
    jsi::Object &console,
    std::shared_ptr<jsi::Object> &originalConsole,
    const std::string &name,
    const std::string &chromeTypeDefault) {
  jsi::Runtime &rt = getRuntime();
  auto chromeType = chromeTypeDefault == "" ? name : chromeTypeDefault;
  auto nameID = jsi::PropNameID::forUtf8(rt, name);
  console.setProperty(
      rt,
      nameID,
      jsi::Function::createFromHostFunction(
          rt,
          nameID,
          1,
          [this, originalConsole, name, chromeType](
              jsi::Runtime &runtime,
              const jsi::Value &thisVal,
              const jsi::Value *args,
              size_t count) {
            if (originalConsole) {
              auto val = originalConsole->getProperty(runtime, name.c_str());
              if (val.isObject()) {
                auto obj = val.getObject(runtime);
                if (obj.isFunction(runtime)) {
                  auto func = obj.getFunction(runtime);
                  func.callWithThis(runtime, *originalConsole, args, count);
                }
              }
            }

            if (name != "assert") {
              // All cases other than assert just log a simple message.
              jsi::Array argsArray(runtime, count);
              for (size_t index = 0; index < count; ++index)
                argsArray.setValueAtIndex(runtime, index, args[index]);
              emitConsoleAPICalledEvent(ConsoleMessageInfo{
                  currentTimestampMs(), chromeType, std::move(argsArray)});
              return jsi::Value::undefined();
            }
            // console.assert needs to check the first parameter before
            // logging.
            if (count == 0) {
              // No parameters, throw a blank assertion failed message.
              emitConsoleAPICalledEvent(ConsoleMessageInfo{
                  currentTimestampMs(), chromeType, jsi::Array(runtime, 0)});
            } else if (!toBoolean(runtime, args[0])) {
              // Shift the message array down by one to not include the
              // condition.
              jsi::Array argsArray(runtime, count - 1);
              for (size_t index = 1; index < count; ++index)
                argsArray.setValueAtIndex(runtime, index, args[index]);
              emitConsoleAPICalledEvent(ConsoleMessageInfo{
                  currentTimestampMs(), chromeType, std::move(argsArray)});
            }

            return jsi::Value::undefined();
          }));
}

} // namespace chrome
} // namespace inspector
} // namespace hermes
} // namespace facebook
