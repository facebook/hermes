/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <sstream>
#include <unordered_set>

#include <hermes/cdp/MessageConverters.h>
#include <hermes/cdp/RemoteObjectConverters.h>
#include <hermes/cdp/RemoteObjectsTable.h>
#include <jsi/instrumentation.h>

#include "RuntimeDomainAgent.h"

namespace facebook {
namespace hermes {
namespace cdp {

static const char *const kUserEnteredScriptIdPrefix = "userScript";

// Chrome does not assign a URL to evaluated scripts
static const char *const kEvaluatedCodeUrl = "";

namespace {
/// Runtime.CallArguments can have their values specified "inline", or they
/// can have remote object references. The inline values are eval'd together
/// with the Runtime.CallFunctionOn.functionDeclaration (see
/// CallFunctionOnBuilder below), while remote object IDs need to be resolved
/// first.
class CallFunctionOnArgument {
 public:
  explicit CallFunctionOnArgument(
      std::optional<m::runtime::RemoteObjectId> maybeObjectId)
      : maybeObjectId_(std::move(maybeObjectId)) {}

  /// Computes the real value for this argument, which can be an object
  /// referenced by maybeObjectId_, or the given evaldValue. Throws if
  /// maybeObjectId_ is not empty but references an unknown object.
  std::optional<jsi::Value> value(
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
  static std::optional<jsi::Value> getValueFromId(
      jsi::Runtime &rt,
      RemoteObjectsTable &objTable,
      m::runtime::RemoteObjectId objId) {
    if (objTable.isScopeId(objId)) {
      if (objTable.getScope(objId) != nullptr) {
        // Scope is found, but since scope IDs are not actual objects of the
        // running program, there isn't an actual value associated with it.
        return std::nullopt;
      }
    } else {
      if (const jsi::Value *ptr = objTable.getValue(objId)) {
        return jsi::Value(rt, *ptr);
      }
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
  /// \p evalResult is the result of evaluating the expression built by the
  /// CallFunctionOnBuilder below.
  jsi::Value operator()(
      jsi::Runtime &rt,
      RemoteObjectsTable &objTable,
      const jsi::Value &evalResult) {
    // The eval result is an array [a0, a1, ..., an, func] (see
    // CallFunctionOnBuilder below).
    auto argsAndFunc = evalResult.getObject(rt).getArray(rt);
    assert(
        argsAndFunc.length(rt) == thisAndArguments_.size() + 1 &&
        "Unexpected result size");

    // now resolve the arguments to the call, including "this".
    std::vector<jsi::Value> arguments(thisAndArguments_.size() - 1);

    std::optional<jsi::Object> jsThis =
        getJsThis(rt, objTable, argsAndFunc.getValueAtIndex(rt, kJsThisIndex));

    size_t i = kFirstArgIndex;
    for (/*i points to the first param*/; i < thisAndArguments_.size(); ++i) {
      std::optional<jsi::Value> value = thisAndArguments_[i].value(
          rt, objTable, argsAndFunc.getValueAtIndex(rt, i));
      assert(
          value.has_value() &&
          "Expect RemoteObjectId to be referencing real objects and are not scope IDs");
      arguments[i - kFirstArgIndex] = std::move(value.value());
    }

    // i is now func's index.
    jsi::Function func =
        argsAndFunc.getValueAtIndex(rt, i).getObject(rt).getFunction(rt);

    if (jsThis) {
      return func.callWithThis(
          rt,
          std::move(*jsThis),
          static_cast<const jsi::Value *>(arguments.data()),
          arguments.size());
    } else {
      return func.call(
          rt,
          static_cast<const jsi::Value *>(arguments.data()),
          arguments.size());
    }
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
  std::optional<jsi::Object> getJsThis(
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

    const bool useGlobalThis = evaldThis.isString();
    if (useGlobalThis)
      return rt.global();

    std::optional<jsi::Value> value = thisAndArguments_[kJsThisIndex].value(
        rt, objTable, std::move(evaldThis));
    if (!value.has_value()) {
      // VS Code's node.js debugger could pass a scope ID as 'this'. It doesn't
      // make any sense because those RemoteObjects are not real. Still need to
      // handle it so things don't crash.
      return std::nullopt;
    }

    // TODO: Support any type of RemoteObject. Currently we're limited by what's
    // available through jsi::Function.
    assert(
        value.value().isObject() &&
        "jsi::Function only supports callWithThis with a jsi::Object");

    return value.value().getObject(rt);
  }

  std::vector<CallFunctionOnArgument> thisAndArguments_;
  std::optional<m::runtime::ExecutionContextId> executionContextId_;
};

const char *CallFunctionOnRunner::kJsThisArgPlaceholder =
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

template <typename Fn>
std::pair<m::runtime::RemoteObject, std::optional<m::runtime::ExceptionDetails>>
evaluateAndWrapResult(
    jsi::Runtime &runtime,
    RemoteObjectsTable &objTable,
    const std::string &objectGroup,
    const ObjectSerializationOptions &serializationOptions,
    Fn &&fn) {
  m::runtime::RemoteObject remoteObj;
  std::optional<m::runtime::ExceptionDetails> exceptionDetails;
  try {
    jsi::Value result = fn(runtime);
    remoteObj = m::runtime::makeRemoteObject(
        runtime, result, objTable, objectGroup, serializationOptions);
  } catch (const jsi::JSError &error) {
    exceptionDetails =
        m::runtime::makeExceptionDetails(runtime, error, objTable, objectGroup);
    // In V8, @cdp Runtime.evaluate and @cdp Runtime.callFunctionOn populate the
    // `result` field with the exception value, and some code paths in Chrome
    // DevTools depend on this.
    // See
    // https://github.com/facebookexperimental/rn-chrome-devtools-frontend/blob/35aa264a622e853bb28b4c83a7b5cc3b2a9747bc/front_end/core/sdk/RemoteObject.ts#L574-L592
    remoteObj = m::runtime::makeRemoteObjectForError(
        runtime, error.value(), objTable, objectGroup);
  } catch (const jsi::JSIException &err) {
    exceptionDetails = m::runtime::makeExceptionDetails(err);
  }

  return std::make_pair(std::move(remoteObj), std::move(exceptionDetails));
}

/// A grow-only unordered set of jsi::PropNameID (String / Symbol).
/// Must not outlive the \c jsi::Runtime reference provided to the constructor.
class JSIPropNameIDSet {
 public:
  explicit JSIPropNameIDSet(jsi::Runtime &rt);

  /// Inserts a PropNameID into the set.
  /// \return true if the name was not already in the set.
  bool insert(jsi::PropNameID &&name);

 private:
  class Hash {
   public:
    explicit Hash(jsi::Runtime &rt) : runtime_(rt) {}
    size_t operator()(const jsi::PropNameID &name) const {
      return std::hash<std::string>{}(name.utf8(runtime_));
    }

   private:
    jsi::Runtime &runtime_;
  };

  class IsEqual {
   public:
    explicit IsEqual(jsi::Runtime &rt) : runtime_(rt) {}
    bool operator()(const jsi::PropNameID &lhs, const jsi::PropNameID &rhs)
        const {
      return jsi::PropNameID::compare(runtime_, lhs, rhs);
    }

   private:
    jsi::Runtime &runtime_;
  };

  jsi::Runtime &runtime_;
  std::unordered_set<jsi::PropNameID, Hash, IsEqual> names_;
};

JSIPropNameIDSet::JSIPropNameIDSet(jsi::Runtime &rt)
    : runtime_(rt),
      names_(
          /* bucket_count */ 32,
          Hash(runtime_),
          IsEqual(runtime_)) {}

bool JSIPropNameIDSet::insert(jsi::PropNameID &&name) {
  return names_.insert(std::move(name)).second;
}

} // namespace

RuntimeDomainAgent::RuntimeDomainAgent(
    int32_t executionContextID,
    HermesRuntime &runtime,
    debugger::AsyncDebuggerAPI &asyncDebuggerAPI,
    SynchronizedOutboundCallback messageCallback,
    std::shared_ptr<RemoteObjectsTable> objTable,
    ConsoleMessageStorage &consoleMessageStorage,
    ConsoleMessageDispatcher &consoleMessageDispatcher)
    : DomainAgent(
          executionContextID,
          std::move(messageCallback),
          std::move(objTable)),
      runtime_(runtime),
      asyncDebuggerAPI_(asyncDebuggerAPI),
      consoleMessageStorage_(consoleMessageStorage),
      consoleMessageDispatcher_(consoleMessageDispatcher),
      enabled_(false),
      helpers_(runtime_) {
  consoleMessageRegistration_ = consoleMessageDispatcher_.subscribe(
      [this](const ConsoleMessage &message) {
        this->consoleAPICalled(message, /* isBuffered */ false);
      });
}

RuntimeDomainAgent::~RuntimeDomainAgent() {
  consoleMessageDispatcher_.unsubscribe(consoleMessageRegistration_);
}

void RuntimeDomainAgent::enable() {
  if (enabled_) {
    return;
  }

  enabled_ = true;

  // Send any buffered console messages.
  size_t numConsoleMessagesDiscardedFromCache =
      consoleMessageStorage_.discarded();

  if (numConsoleMessagesDiscardedFromCache != 0) {
    std::ostringstream oss;
    oss << "Only limited number of console messages can be cached. "
        << numConsoleMessagesDiscardedFromCache
        << (numConsoleMessagesDiscardedFromCache == 1 ? " message was"
                                                      : " messages were")

        << " discarded at the beginning.";
    jsi::Value arg = jsi::String::createFromAscii(runtime_, oss.str());
    std::vector<jsi::Value> args;
    args.push_back(std::move(arg));

    consoleAPICalled(
        ConsoleMessage(
            *consoleMessageStorage_.oldestTimestamp() - 0.1,
            ConsoleAPIType::kWarning,
            std::move(args)),
        /* isBuffered */ true);
  }

  for (auto &message : consoleMessageStorage_.messages()) {
    consoleAPICalled(message, /* isBuffered */ true);
  }
}

void RuntimeDomainAgent::enable(const m::runtime::EnableRequest &req) {
  // Match V8 behavior of returning success even if domain is already enabled
  enable();
  sendResponseToClient(m::makeOkResponse(req.id));
}

void RuntimeDomainAgent::disable(const m::runtime::DisableRequest &req) {
  // Match V8 behavior of returning success even if domain is already disabled
  enabled_ = false;
  sendResponseToClient(m::makeOkResponse(req.id));
}

void RuntimeDomainAgent::discardConsoleEntries(
    const m::runtime::DiscardConsoleEntriesRequest &req) {
  // Allow this message even if domain is not enabled to match V8 behavior.

  consoleMessageStorage_.clear();
  sendResponseToClient(m::makeOkResponse(req.id));
}

void RuntimeDomainAgent::getHeapUsage(
    const m::runtime::GetHeapUsageRequest &req) {
  // Allow this message even if domain is not enabled to match V8 behavior.

  auto heapInfo = runtime_.instrumentation().getHeapInfo(false);
  m::runtime::GetHeapUsageResponse resp;
  resp.id = req.id;
  resp.usedSize = heapInfo["hermes_allocatedBytes"];
  resp.totalSize = heapInfo["hermes_heapSize"];
  sendResponseToClient(resp);
}

void RuntimeDomainAgent::globalLexicalScopeNames(
    const m::runtime::GlobalLexicalScopeNamesRequest &req) {
  // Allow this message even if domain is not enabled to match V8 behavior.

  if (req.executionContextId.has_value() &&
      !validateExecutionContextId(*req.executionContextId, req.id)) {
    return;
  }

  if (!asyncDebuggerAPI_.isPaused()) {
    sendResponseToClient(m::makeErrorResponse(
        req.id,
        m::ErrorCode::InvalidRequest,
        "Cannot get global scope names unless execution is paused"));
    return;
  }

  const debugger::ProgramState &state =
      runtime_.getDebugger().getProgramState();
  assert(
      state.getStackTrace().callFrameCount() > 0 &&
      "Paused with no call frames");
  const debugger::LexicalInfo &lexicalInfo = state.getLexicalInfo(0);
  debugger::ScopeDepth scopeCount = lexicalInfo.getScopesCount();
  if (scopeCount == 0) {
    sendResponseToClient(m::makeErrorResponse(
        req.id, m::ErrorCode::InvalidRequest, "No scope descriptor"));
    return;
  }
  const debugger::ScopeDepth globalScopeIndex = scopeCount - 1;
  uint32_t variableCount =
      lexicalInfo.getVariablesCountInScope(globalScopeIndex);

  m::runtime::GlobalLexicalScopeNamesResponse resp;
  resp.id = req.id;
  resp.names.reserve(variableCount);
  for (uint32_t i = 0; i < variableCount; i++) {
    debugger::String name = state.getVariableInfo(0, globalScopeIndex, i).name;
    // The global scope has some entries prefixed with '?', which
    // are not valid identifiers.
    if (!name.empty() && name.front() != '?') {
      resp.names.push_back(name);
    }
  }
  sendResponseToClient(resp);
}

void RuntimeDomainAgent::compileScript(
    const m::runtime::CompileScriptRequest &req) {
  if (!checkRuntimeEnabled(req)) {
    return;
  }
  if (req.executionContextId.has_value() &&
      !validateExecutionContextId(*req.executionContextId, req.id)) {
    return;
  }

  m::runtime::CompileScriptResponse resp;
  resp.id = req.id;

  auto source = std::make_shared<jsi::StringBuffer>(req.expression);
  std::shared_ptr<const jsi::PreparedJavaScript> preparedScript;
  try {
    preparedScript = runtime_.prepareJavaScript(source, req.sourceURL);
  } catch (const jsi::JSIException &err) {
    resp.exceptionDetails = m::runtime::makeExceptionDetails(err);
    sendResponseToClient(resp);
    return;
  }

  if (req.persistScript) {
    auto scriptId =
        kUserEnteredScriptIdPrefix + std::to_string(preparedScripts_.size());
    preparedScripts_.push_back(std::move(preparedScript));
    resp.scriptId = scriptId;
  }
  sendResponseToClient(resp);
}

void RuntimeDomainAgent::getProperties(
    const m::runtime::GetPropertiesRequest &req) {
  // Allow this to be used when domain is not enabled to match V8 behavior.

  ObjectSerializationOptions serializationOptions;
  serializationOptions.generatePreview = req.generatePreview.value_or(false);
  bool ownProperties = req.ownProperties.value_or(false);
  bool accessorPropertiesOnly = req.accessorPropertiesOnly.value_or(false);

  std::string objGroup = objTable_->getObjectGroup(req.objectId);
  auto scopePtr = objTable_->getScope(req.objectId);
  auto valuePtr = objTable_->getValue(req.objectId);

  m::runtime::GetPropertiesResponse resp;
  resp.id = req.id;
  try {
    if (scopePtr != nullptr) {
      if (!asyncDebuggerAPI_.isPaused()) {
        sendResponseToClient(m::makeErrorResponse(
            req.id,
            m::ErrorCode::InvalidRequest,
            "Cannot get scope properties unless execution is paused"));
        return;
      }
      const debugger::ProgramState &state =
          runtime_.getDebugger().getProgramState();
      auto result =
          makePropsFromScope(*scopePtr, objGroup, state, serializationOptions);
      if (!result) {
        sendResponseToClient(m::makeErrorResponse(
            req.id,
            m::ErrorCode::InvalidRequest,
            "Could not inspect specified scope"));
        return;
      }
      resp.result = std::move(*result);

    } else if (valuePtr != nullptr) {
      resp.result = makePropsFromValue(
          *valuePtr,
          objGroup,
          ownProperties,
          accessorPropertiesOnly,
          serializationOptions);
      if (!accessorPropertiesOnly) {
        auto internalProps = makeInternalPropsFromValue(
            *valuePtr, objGroup, serializationOptions);
        if (internalProps.size()) {
          resp.internalProperties = std::move(internalProps);
        }
      }
    }
  } catch (const jsi::JSError &error) {
    resp.exceptionDetails =
        m::runtime::makeExceptionDetails(runtime_, error, *objTable_, objGroup);
  } catch (const jsi::JSIException &err) {
    resp.exceptionDetails = m::runtime::makeExceptionDetails(err);
  }
  sendResponseToClient(resp);
}

void RuntimeDomainAgent::evaluate(const m::runtime::EvaluateRequest &req) {
  // Allow this to be used when domain is not enabled to match V8 behavior.

  if (req.contextId.has_value() &&
      !validateExecutionContextId(*req.contextId, req.id)) {
    return;
  }

  m::runtime::EvaluateResponse resp;
  resp.id = req.id;

  ObjectSerializationOptions serializationOptions;
  serializationOptions.returnByValue = req.returnByValue.value_or(false);
  serializationOptions.generatePreview = req.generatePreview.value_or(false);

  std::string objectGroup = req.objectGroup.value_or("");

  std::tie(resp.result, resp.exceptionDetails) = evaluateAndWrapResult(
      runtime_,
      *objTable_,
      objectGroup,
      serializationOptions,
      [&req](jsi::Runtime &runtime) {
        return runtime.evaluateJavaScript(
            std::unique_ptr<jsi::StringBuffer>(
                new jsi::StringBuffer(req.expression)),
            kEvaluatedCodeUrl);
      });

  sendResponseToClient(resp);
}

void RuntimeDomainAgent::callFunctionOn(
    const m::runtime::CallFunctionOnRequest &req) {
  if (!checkRuntimeEnabled(req)) {
    return;
  }

  std::string expression;
  CallFunctionOnRunner runner;

  if (req.objectId.has_value() == req.executionContextId.has_value()) {
    sendResponseToClient(m::makeErrorResponse(
        req.id,
        m::ErrorCode::InvalidRequest,
        "The request must specify either object id or execution context id."));
    return;
  }

  if (!req.objectId) {
    assert(
        req.executionContextId &&
        "should not be here if both object id and execution context id are missing");
    if (!validateExecutionContextId(*req.executionContextId, req.id)) {
      return;
    }
  }

  try {
    std::tie(expression, runner) =
        CallFunctionOnBuilder(req).expressionAndRunner();
  } catch (const std::exception &e) {
    sendResponseToClient(m::makeErrorResponse(
        req.id, m::ErrorCode::InvalidRequest, std::string(e.what())));
    return;
  }

  jsi::Value evalResult;
  try {
    evalResult = runtime_.evaluateJavaScript(
        std::unique_ptr<jsi::StringBuffer>(new jsi::StringBuffer(expression)),
        kEvaluatedCodeUrl);
  } catch (const jsi::JSIException &) {
    sendResponseToClient(m::makeErrorResponse(
        req.id,
        m::ErrorCode::InternalError,
        "Failed to prepare function call"));
    return;
  }

  auto objectGroup = req.objectGroup.value_or("");
  ObjectSerializationOptions serializationOptions;
  serializationOptions.returnByValue = req.returnByValue.value_or(false);
  serializationOptions.generatePreview = req.generatePreview.value_or(false);

  m::runtime::CallFunctionOnResponse resp;
  resp.id = req.id;
  std::tie(resp.result, resp.exceptionDetails) = evaluateAndWrapResult(
      runtime_,
      *objTable_,
      objectGroup,
      serializationOptions,
      [&](jsi::Runtime &runtime) {
        return runner(runtime, *objTable_, evalResult);
      });

  sendResponseToClient(resp);
}

bool RuntimeDomainAgent::checkRuntimeEnabled(const m::Request &req) {
  if (!enabled_) {
    sendResponseToClient(m::makeErrorResponse(
        req.id, m::ErrorCode::InvalidRequest, "Runtime domain not enabled"));
    return false;
  }
  return true;
}

bool RuntimeDomainAgent::validateExecutionContextId(
    m::runtime::ExecutionContextId executionContextId,
    long long commandId) {
  if (executionContextId == executionContextID_) {
    return true;
  }

  sendResponseToClient(m::makeErrorResponse(
      commandId,
      m::ErrorCode::InvalidRequest,
      "Unknown execution context id: " + std::to_string(executionContextId)));
  return false;
}

std::optional<std::vector<m::runtime::PropertyDescriptor>>
RuntimeDomainAgent::makePropsFromScope(
    std::pair<uint32_t, uint32_t> frameAndScopeIndex,
    const std::string &objectGroup,
    const debugger::ProgramState &state,
    const ObjectSerializationOptions &serializationOptions) {
  // Chrome represents variables in a scope as properties on a dummy object.
  // We don't instantiate such dummy objects, we just pretended to have one.
  // Chrome has now asked for its properties, so it's time to synthesize
  // descriptions of the properties that the dummy object would have had.
  std::vector<m::runtime::PropertyDescriptor> result;

  uint32_t frameIndex = frameAndScopeIndex.first;
  uint32_t scopeIndex = frameAndScopeIndex.second;
  if (frameIndex >= state.getStackTrace().callFrameCount()) {
    return std::nullopt;
  }
  debugger::LexicalInfo lexicalInfo = state.getLexicalInfo(frameIndex);
  if (scopeIndex >= lexicalInfo.getScopesCount()) {
    return std::nullopt;
  }
  uint32_t varCount = lexicalInfo.getVariablesCountInScope(scopeIndex);

  // Then add each of the variables in this lexical scope.
  for (uint32_t varIndex = 0; varIndex < varCount; varIndex++) {
    debugger::VariableInfo varInfo =
        state.getVariableInfo(frameIndex, scopeIndex, varIndex);

    m::runtime::PropertyDescriptor desc;
    desc.name = varInfo.name;
    desc.value = m::runtime::makeRemoteObject(
        runtime_, varInfo.value, *objTable_, objectGroup, serializationOptions);
    desc.enumerable = true;
    desc.configurable = true;
    desc.writable = true;
    desc.isOwn = true;

    result.emplace_back(std::move(desc));
  }

  return result;
}

std::vector<m::runtime::PropertyDescriptor>
RuntimeDomainAgent::makePropsFromValue(
    const jsi::Value &value,
    const std::string &objectGroup,
    bool onlyOwnProperties,
    bool accessorPropertiesOnly,
    const ObjectSerializationOptions &serializationOptions) {
  std::vector<m::runtime::PropertyDescriptor> result;

  if (value.isObject()) {
    jsi::Runtime &runtime = runtime_;

    jsi::Value current{runtime, value};
    bool isOwn = true;
    // Keep track of the properties we've emitted so far to avoid duplicates
    // in the case of prototype shadowing.
    JSIPropNameIDSet emittedPropIDs(runtime);
    // Walk up the prototype chain.
    do {
      jsi::Object obj = current.getObject(runtime);
      std::array<jsi::Array, 2> propArrays{
          helpers_.objectGetOwnPropertyNames.call(runtime, obj)
              .asObject(runtime)
              .asArray(runtime),
          helpers_.objectGetOwnPropertySymbols.call(runtime, obj)
              .asObject(runtime)
              .asArray(runtime)};
      // Loop through all own property descriptors and convert them to CDP
      // format.
      for (auto &propArray : propArrays) {
        size_t propCount = propArray.length(runtime);
        for (size_t i = 0; i < propCount; i++) {
          m::runtime::PropertyDescriptor desc;
          desc.isOwn = isOwn;
          jsi::Value propNameOrSymbol = propArray.getValueAtIndex(runtime, i);
          jsi::Object descriptor = helpers_.objectGetOwnPropertyDescriptor
                                       .call(runtime, obj, propNameOrSymbol)
                                       .asObject(runtime);
          if (accessorPropertiesOnly &&
              !descriptor.hasProperty(runtime, "get") &&
              !descriptor.hasProperty(runtime, "set")) {
            continue;
          }
          if (propNameOrSymbol.isString()) {
            auto propName = propNameOrSymbol.getString(runtime);
            if (
                // Short-circuit the shadowing check if we're emitting only
                // own properties.
                !onlyOwnProperties &&
                !emittedPropIDs.insert(
                    jsi::PropNameID::forString(runtime, propName))) {
              // This property has already been emitted somewhere down the
              // prototype chain.
              continue;
            }
            desc.name = propName.utf8(runtime);
          } else if (propNameOrSymbol.isSymbol()) {
            auto propSymbol = propNameOrSymbol.getSymbol(runtime);
            if (
                // Short-circuit the shadowing check if we're emitting only
                // own properties.
                !onlyOwnProperties &&
                !emittedPropIDs.insert(
                    jsi::PropNameID::forSymbol(runtime, propSymbol))) {
              // This property has already been emitted somewhere down the
              // prototype chain.
              continue;
            }
            desc.name = propSymbol.toString(runtime);
            desc.symbol = m::runtime::makeRemoteObject(
                runtime,
                propNameOrSymbol,
                *objTable_,
                objectGroup,
                serializationOptions);
          } else {
            assert(false && "unexpected non-string non-symbol property key");
          }
          try {
            desc.enumerable =
                descriptor.getProperty(runtime, "enumerable").asBool();
            desc.configurable =
                descriptor.getProperty(runtime, "configurable").asBool();
            if (descriptor.hasProperty(runtime, "value")) {
              desc.value = m::runtime::makeRemoteObject(
                  runtime,
                  descriptor.getProperty(runtime, "value"),
                  *objTable_,
                  objectGroup,
                  serializationOptions);
            }
            if (descriptor.hasProperty(runtime, "get")) {
              desc.get = m::runtime::makeRemoteObject(
                  runtime,
                  descriptor.getProperty(runtime, "get"),
                  *objTable_,
                  objectGroup,
                  serializationOptions);
            }
            if (descriptor.hasProperty(runtime, "set")) {
              desc.set = m::runtime::makeRemoteObject(
                  runtime,
                  descriptor.getProperty(runtime, "set"),
                  *objTable_,
                  objectGroup,
                  serializationOptions);
            }
            if (descriptor.hasProperty(runtime, "writable")) {
              desc.writable =
                  descriptor.getProperty(runtime, "writable").asBool();
            }
          } catch (const jsi::JSError &err) {
            desc.wasThrown = true;
            desc.value = m::runtime::makeRemoteObjectForError(
                runtime, err.value(), *objTable_, objectGroup);
          } catch (const jsi::JSIException &) {
            desc.wasThrown = true;
            desc.value = m::runtime::makeRemoteObject(
                runtime,
                jsi::String::createFromUtf8(runtime, "(Exception)"),
                *objTable_,
                objectGroup,
                ObjectSerializationOptions{});
          }
          result.emplace_back(std::move(desc));
        }
      }
      if (onlyOwnProperties) {
        // The client requested own properties only, so stop walking up the
        // prototype chain.
        break;
      }
      current = helpers_.objectGetPrototypeOf.call(runtime, obj);
      isOwn = false;
    } while (current.isObject());
  }

  return result;
}

std::vector<m::runtime::InternalPropertyDescriptor>
RuntimeDomainAgent::makeInternalPropsFromValue(
    const jsi::Value &value,
    const std::string &objectGroup,
    const ObjectSerializationOptions &serializationOptions) {
  std::vector<m::runtime::InternalPropertyDescriptor> result;
  if (value.isObject()) {
    jsi::Runtime &runtime = runtime_;

    jsi::Object obj = value.getObject(runtime);

    jsi::Value proto = helpers_.objectGetPrototypeOf.call(runtime, obj);
    if (!proto.isNull()) {
      m::runtime::InternalPropertyDescriptor desc;
      desc.name = "[[Prototype]]";
      desc.value = m::runtime::makeRemoteObject(
          runtime, proto, *objTable_, objectGroup, serializationOptions);
      result.emplace_back(std::move(desc));
    }
  }
  return result;
}

static std::string consoleMessageTypeName(ConsoleAPIType type) {
  switch (type) {
    case ConsoleAPIType::kLog:
      return "log";
    case ConsoleAPIType::kDebug:
      return "debug";
    case ConsoleAPIType::kInfo:
      return "info";
    case ConsoleAPIType::kError:
      return "error";
    case ConsoleAPIType::kWarning:
      return "warning";
    case ConsoleAPIType::kDir:
      return "dir";
    case ConsoleAPIType::kDirXML:
      return "dirxml";
    case ConsoleAPIType::kTable:
      return "table";
    case ConsoleAPIType::kTrace:
      return "trace";
    case ConsoleAPIType::kStartGroup:
      return "startGroup";
    case ConsoleAPIType::kStartGroupCollapsed:
      return "startGroupCollapsed";
    case ConsoleAPIType::kEndGroup:
      return "endGroup";
    case ConsoleAPIType::kClear:
      return "clear";
    case ConsoleAPIType::kAssert:
      return "assert";
    case ConsoleAPIType::kTimeEnd:
      return "timeEnd";
    case ConsoleAPIType::kCount:
      return "count";
    default:
      assert(false && "unknown console API type");
      return "error";
  }
}

void RuntimeDomainAgent::consoleAPICalled(
    const ConsoleMessage &message,
    bool isBuffered) {
  if (!enabled_) {
    return;
  }

  m::runtime::ConsoleAPICalledNotification note;
  note.type = consoleMessageTypeName(message.type);
  note.timestamp = message.timestamp;
  note.executionContextId = executionContextID_;
  if (message.stackTrace.callFrameCount() > 0) {
    note.stackTrace = m::runtime::StackTrace{};
    note.stackTrace->callFrames =
        m::runtime::makeCallFrames(message.stackTrace);
  }
  ObjectSerializationOptions serializationOptions;
  serializationOptions.generatePreview = !isBuffered;
  for (auto &arg : message.args) {
    note.args.push_back(m::runtime::makeRemoteObject(
        runtime_, arg, *objTable_, "ConsoleObjectGroup", serializationOptions));
  }

  sendNotificationToClient(note);
}

RuntimeDomainAgent::Helpers::Helpers(jsi::Runtime &runtime)
    : // TODO(moti): The best place to read and cache these helpers is in
      // CDPDebugAPI, before user code ever runs.
      objectGetOwnPropertySymbols(
          runtime.global()
              .getPropertyAsObject(runtime, "Object")
              .getPropertyAsFunction(runtime, "getOwnPropertySymbols")),
      objectGetOwnPropertyNames(
          runtime.global()
              .getPropertyAsObject(runtime, "Object")
              .getPropertyAsFunction(runtime, "getOwnPropertyNames")),
      objectGetOwnPropertyDescriptor(
          runtime.global()
              .getPropertyAsObject(runtime, "Object")
              .getPropertyAsFunction(runtime, "getOwnPropertyDescriptor")),
      objectGetPrototypeOf(
          runtime.global()
              .getPropertyAsObject(runtime, "Object")
              .getPropertyAsFunction(runtime, "getPrototypeOf")) {}

} // namespace cdp
} // namespace hermes
} // namespace facebook
