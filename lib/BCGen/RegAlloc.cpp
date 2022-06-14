/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/BCGen/RegAlloc.h"
#include "hermes/IR/CFG.h"
#include "hermes/IR/IR.h"
#include "hermes/IR/IRBuilder.h"
#include "hermes/IR/Instrs.h"
#include "hermes/Support/PerfSection.h"
#include "hermes/Utils/Dumper.h"

#include "llvh/Support/Debug.h"
#include "llvh/Support/raw_ostream.h"

#include <queue>

#define DEBUG_TYPE "regalloc"

using namespace hermes;

using llvh::dbgs;

raw_ostream &llvh::operator<<(raw_ostream &OS, const Register &reg) {
  if (!reg.isValid()) {
    OS << "Null";
  } else {
    OS << "Reg" << reg.getIndex();
  }

  return OS;
}

raw_ostream &llvh::operator<<(raw_ostream &OS, const Segment &segment) {
  if (segment.empty()) {
    OS << "[empty]";
    return OS;
  }

  OS << "[" << segment.start_ << "..." << segment.end_ << ") ";
  return OS;
}

raw_ostream &llvh::operator<<(raw_ostream &OS, const Interval &interval) {
  Interval t = interval.compress();
  for (auto &s : t.segments_) {
    OS << s;
  }
  return OS;
}

bool RegisterFile::isUsed(Register r) {
  return !registers.test(r.getIndex());
}

bool RegisterFile::isFree(Register r) {
  return !isUsed(r);
}

void RegisterFile::killRegister(Register reg) {
  LLVM_DEBUG(dbgs() << "-- Releasing the register " << reg << "\n");

  assert(isUsed(reg) && "Killing an unused register!");
  registers.set(reg.getIndex());
  assert(isFree(reg) && "Error freeing register!");
}

void RegisterFile::verify() {}

void RegisterFile::dump() {
  llvh::outs() << "\n";
  for (unsigned i = 0; i < registers.size(); i++) {
    llvh::outs() << (int)!registers.test(i);
  }
  llvh::outs() << "\n";
}

Register RegisterFile::allocateRegister() {
  // We first check for the 'all' case because we usually have a small number of
  // active bits (<64), so this operation is actually faster than the linear
  // scan.
  if (registers.none()) {
    // If all bits are set, create a new register and return it.
    unsigned numRegs = registers.size();
    registers.resize(numRegs + 1, false);
    Register R = Register(numRegs);
    assert(isUsed(R) && "Error allocating a new register.");
    LLVM_DEBUG(dbgs() << "-- Creating the new register " << R << "\n");
    return R;
  }

  // Search for a free register to use.
  int i = registers.find_first();
  assert(i >= 0 && "Unexpected failure to allocate a register");
  Register R(i);
  LLVM_DEBUG(dbgs() << "-- Assigning the free register " << R << "\n");
  assert(isFree(R) && "Error finding a free register");
  registers.reset(i);
  return R;
}

Register RegisterFile::tailAllocateConsecutive(unsigned n) {
  assert(n > 0 && "Can't request zero registers");

  int lastUsed = registers.size() - 1;
  while (lastUsed >= 0) {
    if (!registers.test(lastUsed))
      break;

    lastUsed--;
  }

  int firstClear = lastUsed + 1;

  LLVM_DEBUG(
      dbgs() << "-- Found the last set bit at offset " << lastUsed << "\n");
  registers.resize(std::max(registers.size(), firstClear + n), true);
  registers.reset(firstClear, firstClear + n);

  LLVM_DEBUG(
      dbgs() << "-- Allocated tail consecutive registers of length " << n
             << ", starting at " << Register(firstClear) << "\n");
  return Register(firstClear);
}

/// \returns true if the PHI node has an external user (that requires a
/// register read) and a local writer.
static bool phiReadWrite(PhiInst *P) {
  bool localPhiUse = false;
  bool externalUse = false;
  bool terminatorUse = false;

  BasicBlock *parent = P->getParent();

  for (auto *U : P->getUsers()) {
    terminatorUse |= llvh::isa<TerminatorInst>(U);
    localPhiUse |=
        (llvh::isa<PhiInst>(U) && U->getParent() == parent && P != U);
    externalUse |= U->getParent() != parent;
  }

  // TODO: the code used to perform a stricter check which missed some cases.
  // TODO: need to come up with a better condition.
  // bool localWrite = false;
  // for (int i = 0, limit = P->getNumEntries(); !localWrite && i < limit; ++i)
  // {
  //   auto entry = P->getEntry(i);
  //   localWrite |= entry.first != P && entry.second == parent;
  // }
  // return terminatorUse || localPhiUse || (externalUse && localWrite);

  return terminatorUse || localPhiUse || externalUse;
}

void RegisterAllocator::lowerPhis(ArrayRef<BasicBlock *> order) {
  llvh::SmallVector<PhiInst *, 8> PHIs;
  IRBuilder builder(F);

  // Collect all PHIs.
  for (auto &BB : order) {
    for (auto &Inst : *BB) {
      if (auto *P = llvh::dyn_cast<PhiInst>(&Inst)) {
        PHIs.push_back(P);
      }
    }
  }

  // The MOV sequence at the end of the block writes the values that need to go
  // into the PHI node registers in the jump-destination basic blocks. In the
  // case of cycles we may need to read a value from a current PHI node and also
  // prepare the value of the same PHI node for the next iteration. To make sure
  // that we read an up-to-date value we copy the value using a MOV before we
  // emit the MOV sequence and replace all external uses.
  for (PhiInst *P : PHIs) {
    if (!phiReadWrite(P))
      continue;

    // The MOV sequence may clobber the PHI. Insert a copy.
    builder.setInsertionPoint(P->getParent()->getTerminator());
    auto *mov = builder.createMovInst(P);

    Value::UseListTy users = P->getUsers();
    // Update all external users:
    for (auto *U : users) {
      // Local uses of the PHI are allowed.
      if (!llvh::isa<PhiInst>(U) && !llvh::isa<TerminatorInst>(U) &&
          U->getParent() == P->getParent())
        continue;

      U->replaceFirstOperandWith(P, mov);
    }
  }

  /// A list of registers that were copied to prevent clobbering. Maps the
  /// original PHI node to the copied value.
  DenseMap<Value *, MovInst *> copied;

  // Lower all PHI nodes into a sequence of MOVs in the predecessor blocks.
  for (PhiInst *P : PHIs) {
    for (unsigned i = 0, e = P->getNumEntries(); i < e; ++i) {
      auto E = P->getEntry(i);
      auto *term = E.second->getTerminator();
      builder.setInsertionPoint(term);
      auto *mov = builder.createMovInst(E.first);
      P->updateEntry(i, mov, E.second);

      // If the terminator uses the value that we are inserting then we can fix
      // the lifetime by making it use the MOV. We can do this because we know
      // that terminators don't modify values in destination PHI nodes and this
      // allows us to merge the lifetime of the value and save a register.
      copied[E.first] = mov;
    }
  }

  // The terminator comes after the MOV sequence, so make sure it uses the
  // updated registers.
  for (auto &BB : order) {
    auto *term = BB->getTerminator();

    for (int i = 0, e = term->getNumOperands(); i < e; i++) {
      auto *op = term->getOperand(i);
      if (llvh::isa<Literal>(op))
        continue;
      auto it = copied.find(op);
      if (it != copied.end()) {
        if (it->second->getParent() == BB) {
          term->setOperand(it->second, i);
          it->second->moveBefore(term);
        }
      }
    }
  }
}

void RegisterAllocator::calculateLocalLiveness(
    BlockLifetimeInfo &livenessInfo,
    BasicBlock *BB) {
  // For each instruction in the block:
  for (auto &it : *BB) {
    Instruction *I = &it;

    unsigned idx = getInstructionNumber(I);
    livenessInfo.kill_.set(idx);

    // PHI nodes require special handling because they are flow sensitive. Mask
    // out flow that does not go in the direction of the phi edge.
    if (auto *P = llvh::dyn_cast<PhiInst>(I)) {
      llvh::SmallVector<unsigned, 4> incomingValueNum;

      // Collect all incoming value numbers.
      for (int i = 0, e = P->getNumEntries(); i < e; i++) {
        auto E = P->getEntry(i);
        // Skip unreachable predecessors.
        if (!blockLiveness_.count(E.second))
          continue;
        if (auto *II = llvh::dyn_cast<Instruction>(E.first)) {
          incomingValueNum.push_back(getInstructionNumber(II));
        }
      }

      // Block the incoming values from flowing into the predecessors.
      for (int i = 0, e = P->getNumEntries(); i < e; i++) {
        auto E = P->getEntry(i);
        // Skip unreachable predecessors.
        if (!blockLiveness_.count(E.second))
          continue;
        for (auto num : incomingValueNum) {
          blockLiveness_[E.second].maskIn_.set(num);
        }
      }

      // Allow the flow of incoming values in specific directions:
      for (int i = 0, e = P->getNumEntries(); i < e; i++) {
        auto E = P->getEntry(i);
        // Skip unreachable predecessors.
        if (!blockLiveness_.count(E.second))
          continue;
        if (auto *II = llvh::dyn_cast<Instruction>(E.first)) {
          unsigned idxII = getInstructionNumber(II);
          blockLiveness_[E.second].maskIn_.reset(idxII);
        }
      }
    }

    // For each one of the operands that are also instructions:
    for (unsigned opIdx = 0, e = I->getNumOperands(); opIdx != e; ++opIdx) {
      auto *opInst = llvh::dyn_cast<Instruction>(I->getOperand(opIdx));
      if (!opInst)
        continue;
      // Skip instructions from unreachable blocks.
      if (!blockLiveness_.count(opInst->getParent()))
        continue;

      // Get the index of the operand.
      auto opInstIdx = getInstructionNumber(opInst);
      livenessInfo.gen_.set(opInstIdx);
    }
  }
}

#ifndef NDEBUG
static void dumpVector(
    const BitVector &bv,
    StringRef text,
    llvh::raw_ostream &ost = llvh::errs()) {
  ost << text;
  for (unsigned i = 0; i < bv.size(); i++) {
    ost << bv.test(i);
  }
  ost << "\n";
}
#endif

void RegisterAllocator::calculateGlobalLiveness(ArrayRef<BasicBlock *> order) {
  unsigned iterations = 0;
  bool changed = false;

  // Init the live-in vector to be GEN-KILL.
  for (auto &it : blockLiveness_) {
    BlockLifetimeInfo &livenessInfo = it.second;
    livenessInfo.liveIn_ |= livenessInfo.gen_;
    livenessInfo.liveIn_.reset(livenessInfo.kill_);
    livenessInfo.liveIn_.reset(livenessInfo.maskIn_);
  }

  do {
    iterations++;
    changed = false;

    for (auto it = order.rbegin(), e = order.rend(); it != e; it++) {
      BasicBlock *BB = *it;
      BlockLifetimeInfo &livenessInfo = blockLiveness_[BB];

      // Rule:  OUT = SUCC0_in  + SUCC1_in ...
      for (auto *succ : successors(BB)) {
        BlockLifetimeInfo &succInfo = blockLiveness_[succ];
        // Check if we are about to change bits in the live-out vector.
        if (succInfo.liveIn_.test(livenessInfo.liveOut_)) {
          changed = true;
        }

        livenessInfo.liveOut_ |= succInfo.liveIn_;
      }

      // Rule: In = gen + (OUT - KILL)
      // After initializing 'in' to 'gen' - 'kill', the only way for the result
      // to change is for 'out' to change, and we check if 'out' changes above.
      livenessInfo.liveIn_ = livenessInfo.liveOut_;
      livenessInfo.liveIn_ |= livenessInfo.gen_;
      livenessInfo.liveIn_.reset(livenessInfo.kill_);
      livenessInfo.liveIn_.reset(livenessInfo.maskIn_);
    }
  } while (changed);

#ifndef NDEBUG
  for (auto &it : blockLiveness_) {
    BasicBlock *BB = it.first;
    BlockLifetimeInfo &livenessInfo = it.second;
    LLVM_DEBUG(llvh::dbgs() << "Block " << BB << "\n");
    LLVM_DEBUG(dumpVector(livenessInfo.gen_, "gen     ", llvh::dbgs()));
    LLVM_DEBUG(dumpVector(livenessInfo.kill_, "kill    ", llvh::dbgs()));
    LLVM_DEBUG(dumpVector(livenessInfo.liveIn_, "liveIn  ", llvh::dbgs()));
    LLVM_DEBUG(dumpVector(livenessInfo.liveOut_, "liveOut ", llvh::dbgs()));
    LLVM_DEBUG(dumpVector(livenessInfo.maskIn_, "maskIn  ", llvh::dbgs()));
    LLVM_DEBUG(llvh::dbgs() << "------\n");
  }
#endif

  LLVM_DEBUG(
      dbgs() << "Completed liveness in " << iterations << " iterations\n");
}

Interval &RegisterAllocator::getInstructionInterval(Instruction *I) {
  auto idx = getInstructionNumber(I);
  return instructionInterval_[idx];
}

bool RegisterAllocator::isManuallyAllocatedInterval(Instruction *I) {
  if (hasTargetSpecificLowering(I))
    return true;

  for (auto *U : I->getUsers()) {
    if (hasTargetSpecificLowering(U))
      return true;
  }

  return false;
}

void RegisterAllocator::coalesce(
    DenseMap<Instruction *, Instruction *> &map,
    ArrayRef<BasicBlock *> order) {
  // Merge all PHI nodes into a single interval. This part is required for
  // correctness because it bounds the MOV and the PHIs into a single interval.
  for (BasicBlock *BB : order) {
    for (Instruction &I : *BB) {
      auto *P = llvh::dyn_cast<PhiInst>(&I);
      if (!P)
        continue;

      unsigned phiNum = getInstructionNumber(P);
      for (unsigned i = 0, e = P->getNumEntries(); i < e; ++i) {
        auto *mov = cast<MovInst>(P->getEntry(i).first);

        // Bail out if the interval is already mapped, like in the case of self
        // edges.
        if (map.count(mov))
          continue;

        if (!hasInstructionNumber(mov))
          continue;

        unsigned idx = getInstructionNumber(mov);
        instructionInterval_[phiNum].add(instructionInterval_[idx]);

        // Record the fact that the mov should use the same register as the phi.
        map[mov] = P;
      }
    }
  }

  // Given a sequence of MOVs that was generated by the PHI lowering, try to
  // shorten the lifetime of intervals by reusing copies. For example, we
  // shorten the lifetime of %0 by making %2 use %1.
  // %1 = MovInst %0
  // %2 = MovInst %0
  for (BasicBlock *BB : order) {
    DenseMap<Value *, MovInst *> lastCopy;

    for (Instruction &I : *BB) {
      auto *mov = llvh::dyn_cast<MovInst>(&I);
      if (!mov)
        continue;

      Value *op = mov->getSingleOperand();
      if (llvh::isa<Literal>(op))
        continue;

      // If we've made a copy inside this basic block then use the copy.
      auto it = lastCopy.find(op);
      if (it != lastCopy.end()) {
        mov->setOperand(it->second, 0);
      }

      lastCopy[op] = mov;
    }
  }

  // Optimize the program by coalescing multiple live intervals into a single
  // long interval. This phase is optional.
  for (BasicBlock *BB : order) {
    for (Instruction &I : *BB) {
      auto *mov = llvh::dyn_cast<MovInst>(&I);
      if (!mov)
        continue;

      auto *op = llvh::dyn_cast<Instruction>(mov->getSingleOperand());
      if (!op)
        continue;

      // Don't coalesce intervals that are already coalesced to other intervals
      // or that there are other intervals that are coalesced into it, or if
      // the interval is pre-allocated.
      if (map.count(op) || isAllocated(op) || isAllocated(mov))
        continue;

      // If the MOV is already coalesced into some other interval then merge the
      // operand into that interval.
      Instruction *dest = mov;

      // Don't handle instructions with target specific lowering because this
      // means that we won't release them (and call the target specific hook)
      // until the whole register is freed.
      if (isManuallyAllocatedInterval(op))
        continue;

      // If the mov is already merged into another interval then find the
      // destination interval and try to merge the current interval into it.
      while (map.count(dest)) {
        dest = map[dest];
      }

      unsigned destIdx = getInstructionNumber(dest);
      unsigned opIdx = getInstructionNumber(op);
      Interval &destIvl = instructionInterval_[destIdx];
      Interval &opIvl = instructionInterval_[opIdx];

      if (destIvl.intersects(opIvl))
        continue;

      LLVM_DEBUG(
          dbgs() << "Coalescing instruction @" << opIdx << "  " << opIvl
                 << " -> @" << destIdx << "  " << destIvl << "\n");

      for (auto &it : map) {
        if (it.second == op) {
          LLVM_DEBUG(
              dbgs() << "Remapping @" << getInstructionNumber(it.first)
                     << " from @" << opIdx << " to @" << destIdx << "\n");
          it.second = dest;
        }
      }

      instructionInterval_[destIdx].add(opIvl);
      map[op] = dest;
    }
  }
}

namespace {
/// Determines whether the Instruction is ever used outside its BasicBlock.
bool isBlockLocal(Instruction *inst) {
  BasicBlock *parent = inst->getParent();
  for (auto user : inst->getUsers()) {
    if (parent != user->getParent()) {
      return false;
    }
  }
  return true;
}
} // namespace

void RegisterAllocator::allocateFastPass(ArrayRef<BasicBlock *> order) {
  // Make sure Phis and related Movs get the same register
  for (auto *bb : order) {
    for (auto &inst : *bb) {
      handleInstruction(&inst);
      if (auto *phi = llvh::dyn_cast<PhiInst>(&inst)) {
        auto reg = file.allocateRegister();
        updateRegister(phi, reg);
        for (int i = 0, e = phi->getNumEntries(); i < e; i++) {
          updateRegister(phi->getEntry(i).first, reg);
        }
      }
    }
  }

  llvh::SmallVector<Register, 16> blockLocals;

  // Then just allocate the rest sequentially, while optimizing the case
  // where an inst is only ever used in its own block.
  for (auto *bb : order) {
    for (auto &inst : *bb) {
      if (!isAllocated(&inst)) {
        Register R = file.allocateRegister();
        updateRegister(&inst, R);
        if (inst.getNumUsers() == 0) {
          file.killRegister(R);
        } else if (isBlockLocal(&inst)) {
          blockLocals.push_back(R);
        }
      }
    }
    for (auto &reg : blockLocals) {
      file.killRegister(reg);
    }
    blockLocals.clear();
  }
}

void RegisterAllocator::allocate(ArrayRef<BasicBlock *> order) {
  PerfSection regAlloc("Register Allocation");

  // Lower PHI nodes into a sequence of MOVs.
  lowerPhis(order);

  {
    // We have two forms of register allocation: classic and fast pass.
    // Classic allocation calculates and merges liveness intervals, fast pass
    // just assigns sequentually. The fast pass can be enabled for small
    // functions where runtime memory savings will be small, and for large
    // functions where degenerate behavior can inflate compile time memory
    // usage.

    unsigned int instructionCount = 0;
    for (const auto &BB : order) {
      instructionCount += BB->getInstList().size();
    }
    // We allocate five bits per instruction per basicblock in our liveness
    // sets.
    uint64_t estimatedMemoryUsage =
        (uint64_t)order.size() * instructionCount * 5 / 8;
    if (instructionCount < fastPassThreshold ||
        estimatedMemoryUsage > memoryLimit) {
      allocateFastPass(order);
      return;
    }
  }

  // Number instructions:
  for (auto *BB : order) {
    for (auto &it : *BB) {
      Instruction *I = &it;
      auto idx = getInstructionNumber(I);
      (void)idx;
      assert(idx == getInstructionNumber(I) && "Invalid numbering");
    }
  }

  // Init the basic block liveness data structure and calculate the local
  // liveness for each basic block.
  unsigned maxIdx = getMaxInstrIndex();
  for (auto *BB : order) {
    blockLiveness_[BB].init(maxIdx);
  }
  for (auto *BB : order) {
    calculateLocalLiveness(blockLiveness_[BB], BB);
  }

  // Propagate the local liveness information across the whole function.
  calculateGlobalLiveness(order);

  // Calculate the live intervals for each instruction.
  calculateLiveIntervals(order);

  // Free the memory used for liveness.
  blockLiveness_.clear();

  // Maps coalesced instructions. First uses the register allocated for Second.
  DenseMap<Instruction *, Instruction *> coalesced;

  coalesce(coalesced, order);

  // Compare two intervals and return the one that starts first.
  auto startsFirst = [&](unsigned a, unsigned b) {
    Interval &IA = instructionInterval_[a];
    Interval &IB = instructionInterval_[b];
    return IA.start() < IB.start() || (IA.start() == IB.start() && a < b);
  };

  // Compare two intervals and return the one that starts first. If two
  // intervals end at the same place, schedule the instruction before the
  // operands.

  auto endsFirst = [&](unsigned a, unsigned b) {
    auto &aInterval = instructionInterval_[a];
    auto &bInterval = instructionInterval_[b];
    if (bInterval.end() == aInterval.end()) {
      return bInterval.start() > aInterval.start() ||
          (bInterval.start() == aInterval.start() && b > a);
    }
    return bInterval.end() > aInterval.end();
  };

  using InstList = llvh::SmallVector<unsigned, 32>;

  std::priority_queue<unsigned, InstList, decltype(endsFirst)> intervals(
      endsFirst);

  for (int i = 0, e = getMaxInstrIndex(); i < e; i++) {
    intervals.push(i);
  }

  std::priority_queue<unsigned, InstList, decltype(startsFirst)>
      liveIntervalsQueue(startsFirst);

  // Perform the register allocation:
  while (!intervals.empty()) {
    unsigned instIdx = intervals.top();
    intervals.pop();
    Instruction *inst = instructionsByNumbers_[instIdx];
    Interval &instInterval = instructionInterval_[instIdx];
    unsigned currentIndex = instInterval.end();

    LLVM_DEBUG(
        dbgs() << "Looking at index " << currentIndex << ": " << instInterval
               << " " << inst->getName() << "\n");

    // Free all of the intervals that start after the current index.
    while (!liveIntervalsQueue.empty()) {
      LLVM_DEBUG(
          dbgs() << "\t Cleaning up for index " << currentIndex << " PQ("
                 << liveIntervalsQueue.size() << ")\n");

      unsigned topIdx = liveIntervalsQueue.top();
      Interval &range = instructionInterval_[topIdx];
      LLVM_DEBUG(dbgs() << "\t Earliest interval: " << range << "\n");

      // Flush empty intervals and intervals that finished after our index.
      bool nonEmptyInterval = range.size();
      if (range.start() < currentIndex && nonEmptyInterval) {
        break;
      }

      liveIntervalsQueue.pop();

      Instruction *I = instructionsByNumbers_[topIdx];
      Register R = getRegister(I);
      LLVM_DEBUG(
          dbgs() << "\t Reached idx #" << currentIndex << " deleting inverval "
                 << range << " that's allocated to register " << R
                 << " used by instruction " << I->getName() << "\n");
      file.killRegister(R);

      handleInstruction(I);
    }

    // Don't try to allocate registers that were merged into other live
    // intervals.
    if (coalesced.count(inst)) {
      continue;
    }

    // Allocate a register for the live interval that we are currently handling.
    if (!isAllocated(inst)) {
      Register R = file.allocateRegister();
      updateRegister(inst, R);
    }

    // Mark the current instruction as live and remember to perform target
    // specific calls when we are done with the bundle.
    liveIntervalsQueue.push(instIdx);
  } // For each instruction in the function.

  // Free the remaining intervals.
  while (!liveIntervalsQueue.empty()) {
    Instruction *I = instructionsByNumbers_[liveIntervalsQueue.top()];
    LLVM_DEBUG(
        dbgs() << "Free register used by instruction " << I->getName() << "\n");
    file.killRegister(getRegister(I));
    handleInstruction(I);
    liveIntervalsQueue.pop();
  }

  // Allocate registers for the coalesced registers.
  for (auto &RP : coalesced) {
    assert(!isAllocated(RP.first) && "Register should not be allocated");
    Instruction *dest = RP.second;
    updateRegister(RP.first, getRegister(dest));
  }
}

void RegisterAllocator::calculateLiveIntervals(ArrayRef<BasicBlock *> order) {
  /// Calculate the live intervals for each instruction. Start with a list of
  /// intervals that only contain the instruction itself.
  for (int i = 0, e = instructionsByNumbers_.size(); i < e; ++i) {
    // The instructions are ordered consecutively. The start offset of the
    // instruction is the index in the array plus one because the value starts
    // to live on the next instruction.
    instructionInterval_[i] = Interval(i + 1, i + 1);
  }

  // For each basic block in the liveness map:
  for (BasicBlock *BB : order) {
    BlockLifetimeInfo &liveness = blockLiveness_[BB];

    auto startOffset = getInstructionNumber(&*BB->begin());
    auto endOffset = getInstructionNumber(BB->getTerminator());

    // Register fly-through basic blocks (basic blocks where the value enters)
    // and leavs without doing anything to any of the operands.
    for (int i = 0, e = liveness.liveOut_.size(); i < e; i++) {
      bool leavs = liveness.liveOut_.test(i);
      bool enters = liveness.liveIn_.test(i);
      if (leavs && enters) {
        instructionInterval_[i].add(Segment(startOffset, endOffset + 1));
      }
    }

    // For each instruction in the block:
    for (auto &it : *BB) {
      auto instOffset = getInstructionNumber(&it);
      // The instruction is defined in this basic block. Check if it is leaving
      // the basic block extend the interval until the end of the block.
      if (liveness.liveOut_.test(instOffset)) {
        instructionInterval_[instOffset].add(
            Segment(instOffset + 1, endOffset + 1));
        assert(
            !liveness.liveIn_.test(instOffset) &&
            "Livein but also killed in this block?");
      }

      // Extend the lifetime of the operands.
      for (int i = 0, e = it.getNumOperands(); i < e; i++) {
        auto instOp = llvh::dyn_cast<Instruction>(it.getOperand(i));
        if (!instOp)
          continue;

        if (!hasInstructionNumber(instOp)) {
          assert(
              llvh::isa<PhiInst>(&it) &&
              "Only PhiInst should reference values from dead code");
          continue;
        }

        auto operandIdx = getInstructionNumber(instOp);
        // Extend the lifetime of the interval to reach this instruction.
        // Include this instruction in the interval in order to make sure that
        // the register is not freed before the use.

        auto start = operandIdx + 1;
        auto end = instOffset + 1;
        if (start < end) {
          auto seg = Segment(operandIdx + 1, instOffset + 1);
          instructionInterval_[operandIdx].add(seg);
        }
      }

      // Extend the lifetime of the PHI to include the source basic blocks.
      if (auto *P = llvh::dyn_cast<PhiInst>(&it)) {
        for (int i = 0, e = P->getNumEntries(); i < e; i++) {
          auto E = P->getEntry(i);
          // PhiInsts may reference instructions from dead code blocks
          // (which will be unnumbered and unallocated). Since the edge
          // is necessarily also dead, we can just skip it.
          if (!hasInstructionNumber(E.second->getTerminator()))
            continue;

          unsigned termIdx = getInstructionNumber(E.second->getTerminator());
          Segment S(termIdx, termIdx + 1);
          instructionInterval_[instOffset].add(S);

          // Extend the lifetime of the predecessor to the end of the BB.
          if (auto *instOp = llvh::dyn_cast<Instruction>(E.first)) {
            auto predIdx = getInstructionNumber(instOp);
            auto S2 = Segment(predIdx + 1, termIdx);
            instructionInterval_[predIdx].add(S2);
          }
        } // each pred.
      }

    } // for each instruction in the block.
  } // for each block.
}

struct LivenessRegAllocIRPrinter : IRPrinter {
  RegisterAllocator &allocator;

  explicit LivenessRegAllocIRPrinter(
      RegisterAllocator &RA,
      llvh::raw_ostream &ost,
      bool escape = false)
      : IRPrinter(RA.getContext(), ost, escape), allocator(RA) {}

  void printInstructionDestination(Instruction *I) override {
    if (!allocator.isAllocated(I)) {
      os << "$??? ";
    } else {
      os << "$" << allocator.getRegister(I) << " ";
    }

    if (allocator.hasInstructionNumber(I)) {
      auto idx = allocator.getInstructionNumber(I);
      Interval &ivl = allocator.getInstructionInterval(I);
      os << "@" << idx << " " << ivl << "\t";
    } else {
      os << "          \t";
    }

    IRPrinter::printInstructionDestination(I);
  }

  void printValueLabel(Instruction *I, Value *V, unsigned opIndex) override {
    IRPrinter::printValueLabel(I, V, opIndex);
    auto codeGenOpts = I->getContext().getCodeGenerationSettings();
    if (codeGenOpts.dumpOperandRegisters && allocator.isAllocated(V)) {
      os << " @ $" << allocator.getRegister(V);
    }
  }
};

void RegisterAllocator::dump() {
  LivenessRegAllocIRPrinter Printer(*this, llvh::outs());
  Printer.visitFunction(*F);
}

Register RegisterAllocator::getRegister(Value *I) {
  assert(isAllocated(I) && "Instruction is not allocated!");
  return allocated[I];
}

void RegisterAllocator::updateRegister(Value *I, Register R) {
  allocated[I] = R;
}

bool RegisterAllocator::isAllocated(Value *I) {
  return allocated.count(I);
}

Register RegisterAllocator::reserve(unsigned count) {
  return file.tailAllocateConsecutive(count);
}

Register RegisterAllocator::reserve(ArrayRef<Value *> values) {
  assert(!values.empty() && "Can't reserve zero registers");
  Register first = file.tailAllocateConsecutive(values.size());

  Register T = first;
  for (auto *v : values) {
    if (v)
      allocated[v] = T;
    T = T.getConsecutive();
  }

  return first;
}

bool RegisterAllocator::hasInstructionNumber(Instruction *I) {
  return instructionNumbers_.count(I);
}

unsigned RegisterAllocator::getInstructionNumber(Instruction *I) {
  auto it = instructionNumbers_.find(I);
  if (it != instructionNumbers_.end()) {
    return it->second;
  }

  instructionsByNumbers_.push_back(I);
  instructionInterval_.push_back(Interval());

  unsigned newIdx = instructionsByNumbers_.size() - 1;
  instructionNumbers_[I] = newIdx;
  return newIdx;
}

unsigned llvh::DenseMapInfo<Register>::getHashValue(Register Val) {
  return Val.getIndex();
}

bool llvh::DenseMapInfo<Register>::isEqual(Register LHS, Register RHS) {
  return LHS.getIndex() == RHS.getIndex();
}

#undef DEBUG_TYPE
