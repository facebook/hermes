/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <sstream>

#include <hermes/cdp/MessageConverters.h>
#include <hermes/cdp/RemoteObjectConverters.h>
#include <hermes/cdp/RemoteObjectsTable.h>
#include <jsi/instrumentation.h>

#include "RuntimeDomainAgent.h"

namespace facebook {
namespace hermes {
namespace cdp {

static const char *const kUserEnteredScriptIdPrefix = "userScript";
static const char *const kEvaluatedCodeUrl = "?eval";

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

} // namespace

RuntimeDomainAgent::RuntimeDomainAgent(
    int32_t executionContextID,
    HermesRuntime &runtime,
    SynchronizedOutboundCallback messageCallback,
    std::shared_ptr<RemoteObjectsTable> objTable,
    ConsoleMessageDispatcher &consoleMessageDispatcher)
    : DomainAgent(
          executionContextID,
          std::move(messageCallback),
          std::move(objTable)),
      runtime_(runtime),
      consoleMessageDispatcher_(consoleMessageDispatcher),
      enabled_(false) {
  consoleMessageRegistration_ = consoleMessageDispatcher_.subscribe(
      [this](const ConsoleMessage &message) {
        this->consoleAPICalled(message);
      });
}

RuntimeDomainAgent::~RuntimeDomainAgent() {
  consoleMessageDispatcher_.unsubscribe(consoleMessageRegistration_);
}

void RuntimeDomainAgent::enable(const m::runtime::EnableRequest &req) {
  if (enabled_) {
    // Can't enable twice without disabling
    sendResponseToClient(m::makeErrorResponse(
        req.id,
        m::ErrorCode::InvalidRequest,
        "Runtime domain already enabled"));
    return;
  }

  // Enable
  enabled_ = true;
  sendResponseToClient(m::makeOkResponse(req.id));
}

void RuntimeDomainAgent::disable(const m::runtime::DisableRequest &req) {
  if (!checkRuntimeEnabled(req)) {
    return;
  }
  enabled_ = false;
  sendResponseToClient(m::makeOkResponse(req.id));
}

void RuntimeDomainAgent::getHeapUsage(
    const m::runtime::GetHeapUsageRequest &req) {
  if (!checkRuntimeEnabled(req)) {
    return;
  }

  auto heapInfo = runtime_.instrumentation().getHeapInfo(false);
  m::runtime::GetHeapUsageResponse resp;
  resp.id = req.id;
  resp.usedSize = heapInfo["hermes_allocatedBytes"];
  resp.totalSize = heapInfo["hermes_heapSize"];
  sendResponseToClient(resp);
}

void RuntimeDomainAgent::globalLexicalScopeNames(
    const m::runtime::GlobalLexicalScopeNamesRequest &req) {
  if (!checkRuntimeEnabled(req)) {
    return;
  }

  const debugger::ProgramState &state =
      runtime_.getDebugger().getProgramState();
  const debugger::LexicalInfo &lexicalInfo = state.getLexicalInfo(0);
  debugger::ScopeDepth scopeCount = lexicalInfo.getScopesCount();
  if (scopeCount == 0) {
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

  m::runtime::CompileScriptResponse resp;
  resp.id = req.id;

  auto source = std::make_shared<jsi::StringBuffer>(req.expression);
  std::shared_ptr<const jsi::PreparedJavaScript> preparedScript;
  try {
    preparedScript = runtime_.prepareJavaScript(source, req.sourceURL);
  } catch (const facebook::jsi::JSIException &err) {
    resp.exceptionDetails = m::runtime::ExceptionDetails();
    resp.exceptionDetails->text = err.what();
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
  if (!checkRuntimeEnabled(req)) {
    return;
  }

  bool generatePreview = req.generatePreview.value_or(false);
  bool ownProperties = req.ownProperties.value_or(true);

  std::string objGroup = objTable_->getObjectGroup(req.objectId);
  auto scopePtr = objTable_->getScope(req.objectId);
  auto valuePtr = objTable_->getValue(req.objectId);

  m::runtime::GetPropertiesResponse resp;
  resp.id = req.id;
  if (scopePtr != nullptr) {
    const debugger::ProgramState &state =
        runtime_.getDebugger().getProgramState();
    resp.result =
        makePropsFromScope(*scopePtr, objGroup, state, generatePreview);
  } else if (valuePtr != nullptr) {
    resp.result =
        makePropsFromValue(*valuePtr, objGroup, ownProperties, generatePreview);
  }
  sendResponseToClient(resp);
}

void RuntimeDomainAgent::evaluate(const m::runtime::EvaluateRequest &req) {
  if (!checkRuntimeEnabled(req)) {
    return;
  }

  m::runtime::EvaluateResponse resp;
  resp.id = req.id;

  std::string objectGroup = req.objectGroup.value_or("");
  try {
    // Evaluate the expression using the runtime's normal script evaluation
    // mechanism. This ensures the expression is evaluated in the global scope,
    // regardless of where the runtime happens to be paused.
    jsi::Value result = runtime_.evaluateJavaScript(
        std::unique_ptr<jsi::StringBuffer>(
            new jsi::StringBuffer(req.expression)),
        kEvaluatedCodeUrl);

    bool byValue = req.returnByValue.value_or(false);
    bool generatePreview = req.generatePreview.value_or(false);
    auto remoteObjPtr = m::runtime::makeRemoteObject(
        runtime_, result, *objTable_, objectGroup, byValue, generatePreview);
    resp.result = std::move(remoteObjPtr);
  } catch (const facebook::jsi::JSError &error) {
    resp.exceptionDetails = m::runtime::ExceptionDetails();
    resp.exceptionDetails->text = error.getMessage() + "\n" + error.getStack();
    resp.exceptionDetails->exception = m::runtime::makeRemoteObject(
        runtime_, error.value(), *objTable_, objectGroup, false, false);
  }

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
    if (*req.executionContextId != executionContextID_) {
      sendResponseToClient(m::makeErrorResponse(
          req.id,
          m::ErrorCode::InvalidRequest,
          "unknown execution context id " +
              std::to_string(*req.executionContextId)));
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
  } catch (const facebook::jsi::JSError &error) {
    sendResponseToClient(m::makeErrorResponse(
        req.id,
        m::ErrorCode::InternalError,
        "Failed to prepare function call"));
    return;
  }

  auto objectGroup = req.objectGroup.value_or("");
  auto byValue = req.returnByValue.value_or(false);
  auto generatePreview = req.generatePreview.value_or(false);

  m::runtime::CallFunctionOnResponse resp;
  resp.id = req.id;
  resp.result = m::runtime::makeRemoteObject(
      runtime_,
      (runner)(runtime_, *objTable_, evalResult),
      *objTable_,
      objectGroup,
      byValue,
      generatePreview);
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

std::vector<m::runtime::PropertyDescriptor>
RuntimeDomainAgent::makePropsFromScope(
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
        runtime_,
        varInfo.value,
        *objTable_,
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
        runtime_,
        varInfo.value,
        *objTable_,
        objectGroup,
        false,
        generatePreview);
    desc.enumerable = true;

    result.emplace_back(std::move(desc));
  }

  return result;
}

std::vector<m::runtime::PropertyDescriptor>
RuntimeDomainAgent::makePropsFromValue(
    const jsi::Value &value,
    const std::string &objectGroup,
    bool onlyOwnProperties,
    bool generatePreview) {
  std::vector<m::runtime::PropertyDescriptor> result;

  if (value.isObject()) {
    jsi::Runtime &runtime = runtime_;
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
            runtime,
            propValue,
            *objTable_,
            objectGroup,
            false,
            generatePreview);
      } catch (const jsi::JSError &err) {
        // We fetched a property with a getter that threw. Show a placeholder.
        // We could have added additional info, but the UI quickly gets messy.
        desc.value = m::runtime::makeRemoteObject(
            runtime,
            jsi::String::createFromUtf8(runtime, "(Exception)"),
            *objTable_,
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
            runtime, proto, *objTable_, objectGroup, false, generatePreview);
        result.emplace_back(std::move(desc));
      }
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

void RuntimeDomainAgent::consoleAPICalled(const ConsoleMessage &message) {
  if (!enabled_) {
    return;
  }

  m::runtime::ConsoleAPICalledNotification note;
  note.type = consoleMessageTypeName(message.type);
  note.timestamp = message.timestamp;
  note.executionContextId = executionContextID_;

  for (auto &arg : message.args) {
    note.args.push_back(m::runtime::makeRemoteObject(
        runtime_, arg, *objTable_, "ConsoleObjectGroup", false, false));
  }

  sendNotificationToClient(note);
}

} // namespace cdp
} // namespace hermes
} // namespace facebook
