/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
 */
#include <gtest/gtest.h>
#include <jsi/jsi.h>
#include <jsi/jsilib.h>
#include <jsi/threadsafe.h>
#include "testlib.h"

#include <fstream>
#include <thread>

using namespace facebook::jsi;

class FBJSITest : public JSITestBase {};

// There's no way to add a special test case on only the ReEntrant instance,
// so we'll just noop if its not the ReEntrant instance.
TEST_P(FBJSITest, IsThreadSafeIfThreadSafe) {
  auto trtPtr = dynamic_cast<ThreadSafeRuntime*>(&rt);
  if (!trtPtr) {
    return;
  }
  Function inc = function("function (o) { o.val += 1; }");
  Object obj(rt);
  obj.setProperty(rt, "val", 0);

  std::function<void(void)> adder = [&]() {
    for (int i = 0; i < 500; i++) {
      inc.call(rt, obj);
    }
  };

  std::thread t1(adder);
  std::thread t2(adder);
  std::thread t3(adder);
  t3.join();
  t2.join();
  t1.join();

  EXPECT_EQ(obj.getProperty(rt, "val").getNumber(), 1500);
}

#ifndef _WINDOWS // skip on Windows because FileBuffer has dummy implementation
TEST_P(FBJSITest, FileBuffer) {
  class TmpFile {
   public:
    TmpFile(const std::string& pathTemplate) {
      // Getting tmp files in pre c++17 world is an awful task.
      // For objc and android tests we set TMPDIR env variable,
      // on host /tmp/ directory should be good enough.
      const char* tmpdir = ::getenv("TMPDIR");
      if (!tmpdir) {
        tmpdir = "/tmp";
      }

      path_ = tmpdir + ("/" + pathTemplate);
      std::vector<char> buffer(path_.begin(), path_.end());
      buffer.push_back('\0');

      ::close(mkstemp(buffer.data()));

      path_ = std::string(buffer.begin(), --buffer.end());
    }

    ~TmpFile() {
      ::unlink(path_.c_str());
    }

    const std::string& path() const {
      return path_;
    }

   private:
    std::string path_;
  };

  TmpFile file("JSITest-FileBuffer-tmp-XXXXXX");
  {
    std::fstream os(file.path());
    os << "function reveal_the_meaning_of_life() {"
       << "  return 42;"
       << "}";
  }

  rt.evaluateJavaScript(std::make_unique<FileBuffer>(file.path()), "");
  EXPECT_EQ(
      rt.global()
          .getPropertyAsFunction(rt, "reveal_the_meaning_of_life")
          .call(rt)
          .getNumber(),
      42);

  std::string exc;
  try {
    FileBuffer buffer("this-is-not-a-valid-path");
  } catch (JSINativeException& e) {
    exc = e.what();
  }
  EXPECT_NE(
      exc.find("Could not open this-is-not-a-valid-path: "), std::string::npos);
}
#endif // !defined(_WINDOWS)

INSTANTIATE_TEST_CASE_P(
    Runtimes,
    FBJSITest,
    ::testing::ValuesIn(runtimeGenerators()));
