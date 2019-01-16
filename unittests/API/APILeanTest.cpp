#include <gtest/gtest.h>
#include <hermes/hermes.h>
#include <jsi/jsi.h>

using namespace facebook::jsi;
using namespace facebook::hermes;

namespace {

class HermesLeanRuntimeTest : public ::testing::Test {
 public:
  HermesLeanRuntimeTest() : rt(makeHermesRuntime()) {}

 protected:
  std::unique_ptr<HermesRuntime> rt;
};

TEST_F(HermesLeanRuntimeTest, PropertyTest) {
  rt->global().setProperty(*rt, "answer", Value(42));
  EXPECT_EQ(rt->global().getProperty(*rt, "answer").getNumber(), 42);
}

} // namespace
