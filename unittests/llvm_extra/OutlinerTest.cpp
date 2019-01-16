#include "llvm_extra/Outliner.h"

#include "gtest/gtest.h"

#include <cstring>
#include <iterator>
#include <utility>
#include <vector>

using llvm::outliner::Candidate;
using llvm::outliner::getFunctionsToOutline;
using llvm::outliner::OutlinedFunction;
using llvm::outliner::OutlinerTarget;

namespace {

// Stub implementation of OutlinerTarget that uses predefined values for the
// minimum candidate length, call overhead, and frame overhead.
class Target : public OutlinerTarget {
 private:
  const unsigned minLength;
  const unsigned callOverhead;
  const unsigned frameOverhead;

 public:
  Target(
      unsigned minLength = 1,
      unsigned callOverhead = 0,
      unsigned frameOverhead = 0)
      : minLength(minLength),
        callOverhead(callOverhead),
        frameOverhead(frameOverhead) {}

  unsigned minCandidateLength() override {
    return minLength;
  }

  void createOutlinedFunctions(
      std::vector<OutlinedFunction> &functions,
      llvm::ArrayRef<unsigned> startIndices,
      unsigned len) override {
    std::vector<Candidate> candidates;
    for (unsigned StartIdx : startIndices) {
      candidates.emplace_back(StartIdx, len, callOverhead);
    }
    functions.emplace_back(std::move(candidates), len, frameOverhead);
  }
};

// Remove candidates that were marked deleted. Also remove entire
// OutlinedFunctions if all their candidates were marked deleted.
void filterOutlinedFunctions(std::vector<OutlinedFunction> &functions) {
  std::vector<OutlinedFunction> filtered;
  for (OutlinedFunction &func : functions) {
    std::vector<Candidate> candidates;
    for (Candidate &c : func.Candidates) {
      if (!c.isDeleted()) {
        candidates.push_back(std::move(c));
      }
    }
    if (!candidates.empty()) {
      func.Candidates = std::move(candidates);
      filtered.push_back(std::move(func));
    }
  }
  functions = std::move(filtered);
}

TEST(OutlinerTest, EmptyStringTest) {
  std::vector<unsigned> vec;
  Target target;
  std::vector<OutlinedFunction> functions;
  getFunctionsToOutline(functions, vec, &target);

  EXPECT_TRUE(functions.empty());
}

TEST(OutlinerTest, DuplicateTest) {
  const unsigned size = std::strlen("DUPLICATE");
  const char str[] = "DUPLICATE0DUPLICATE1";
  std::vector<unsigned> vec(std::begin(str), std::end(str));
  Target target;
  std::vector<OutlinedFunction> functions;
  getFunctionsToOutline(functions, vec, &target);
  filterOutlinedFunctions(functions);

  ASSERT_EQ(1, functions.size());
  OutlinedFunction &func = functions.front();
  EXPECT_EQ(size, func.SequenceSize);
  EXPECT_EQ(size, func.getBenefit());

  // The order of candidates doesn't really matter, but determinism does, so
  // check each one explicitly here.
  ASSERT_EQ(2, func.Candidates.size());
  EXPECT_EQ(size + 1, func.Candidates[0].getStartIdx());
  EXPECT_EQ(0, func.Candidates[1].getStartIdx());
  EXPECT_EQ(size, func.Candidates[0].getLength());
  EXPECT_EQ(size, func.Candidates[1].getLength());
}

TEST(OutlinerTest, InternalNodeLimitationTest) {
  const char str[] = "hahahahaha0";
  std::vector<unsigned> vec(std::begin(str), std::end(str));
  Target target;
  std::vector<OutlinedFunction> functions;
  getFunctionsToOutline(functions, vec, &target);
  filterOutlinedFunctions(functions);

  // We can't outline this because we only consider internal nodes with two leaf
  // children. In this case all such nodes are pruned because they overlap.
  // Considering all internal nodes as outlining candidates would be possible,
  // but more expensive, since for any candidate X, we would also have a
  // candidate for every substring of X (rather than just every suffix of X).
  EXPECT_TRUE(functions.empty());
}

TEST(OutlinerTest, NoOverlapTest) {
  const char str[] = "overlapoverlapover";
  std::vector<unsigned> vec(std::begin(str), std::end(str));
  Target target;
  std::vector<OutlinedFunction> functions;
  getFunctionsToOutline(functions, vec, &target);
  filterOutlinedFunctions(functions);

  ASSERT_EQ(1, functions.size());
  OutlinedFunction &func = functions.front();
  ASSERT_EQ(2, func.Candidates.size());
  Candidate &c = func.Candidates.front();
  EXPECT_EQ("lapover", std::string(str, c.getStartIdx(), c.getLength()));
}

TEST(OutlinerTest, MinLengthTest) {
  const char str[] = "short0short1lengthy2lengthy3";
  std::vector<unsigned> vec(std::begin(str), std::end(str));
  const unsigned minLength = std::strlen("lengthy");
  Target target(minLength);
  std::vector<OutlinedFunction> functions;
  getFunctionsToOutline(functions, vec, &target);
  filterOutlinedFunctions(functions);

  ASSERT_EQ(1, functions.size());
  OutlinedFunction &func = functions.front();
  ASSERT_EQ(2, func.Candidates.size());
  Candidate &c = func.Candidates.front();
  EXPECT_EQ("lengthy", std::string(str, c.getStartIdx(), c.getLength()));
}

TEST(OutlinerTest, CallOverheadTest) {
  const char str[] = "aaa0aaa1aaa2zzz3zzz4zzz5zzz6";
  std::vector<unsigned> vec(std::begin(str), std::end(str));
  const unsigned minLength = 1;
  const unsigned callOverhead = 2;
  const unsigned frameOverhead = 0;
  Target target(minLength, callOverhead, frameOverhead);
  std::vector<OutlinedFunction> functions;
  getFunctionsToOutline(functions, vec, &target);
  filterOutlinedFunctions(functions);

  // No outline cost:  3 + 3 + 3 + 3 = 12
  // Outline cost: 3 + 2 + 2 + 2 + 2 = 11
  // Benefit: 1

  ASSERT_EQ(1, functions.size());
  OutlinedFunction &func = functions.front();
  EXPECT_EQ(1, func.getBenefit());
  ASSERT_EQ(4, func.Candidates.size());
  Candidate &c = func.Candidates.front();
  EXPECT_EQ("zzz", std::string(str, c.getStartIdx(), c.getLength()));
}

TEST(OutlinerTest, FrameOverheadTest) {
  const char str[] = "aaa0aaa1zzz2zzz3zzz4";
  std::vector<unsigned> vec(std::begin(str), std::end(str));
  const unsigned minLength = 1;
  const unsigned callOverhead = 1;
  const unsigned frameOverhead = 2;
  Target target(minLength, callOverhead, frameOverhead);
  std::vector<OutlinedFunction> functions;
  getFunctionsToOutline(functions, vec, &target);
  filterOutlinedFunctions(functions);

  // No outline cost:      3 + 3 + 3 = 9
  // Outline cost: (3+2) + 1 + 1 + 1 = 8
  // Benefit: 1

  ASSERT_EQ(1, functions.size());
  OutlinedFunction &func = functions.front();
  EXPECT_EQ(1, func.getBenefit());
  ASSERT_EQ(3, func.Candidates.size());
  Candidate &c = func.Candidates.front();
  EXPECT_EQ("zzz", std::string(str, c.getStartIdx(), c.getLength()));
}

} // namespace
