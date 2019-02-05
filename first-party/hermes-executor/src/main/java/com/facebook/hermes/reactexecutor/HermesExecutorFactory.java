/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
 */
package com.facebook.hermes.reactexecutor;

import com.facebook.react.bridge.JavaScriptExecutor;
import com.facebook.react.bridge.JavaScriptExecutorFactory;
import java.util.concurrent.ScheduledExecutorService;
import javax.annotation.Nullable;

public class HermesExecutorFactory implements JavaScriptExecutorFactory {
  private static final String TAG = "Hermes";

  private final @Nullable ScheduledExecutorService mTimeoutExecutor;
  private final double mTimeoutMsec;
  private final RuntimeConfig mConfig;

  public HermesExecutorFactory() {
    this(null, 0, null);
  }

  public HermesExecutorFactory(
      @Nullable ScheduledExecutorService timeoutExecutor,
      double timeoutMsec,
      RuntimeConfig config) {

    mTimeoutExecutor = timeoutExecutor;
    mTimeoutMsec = timeoutMsec;
    mConfig = config;
  }

  @Override
  public JavaScriptExecutor create() {
    return new HermesExecutor(mTimeoutExecutor, mTimeoutMsec, mConfig);
  }

  @Override
  public String toString() {
    return "JSIExecutor+HermesRuntime";
  }
}
