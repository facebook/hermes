#ifndef HERMES_VM_ITERATIONKIND_H
#define HERMES_VM_ITERATIONKIND_H

namespace hermes {
namespace vm {

/// Different Iteration Kinds. Entry means Key + Value.
enum class IterationKind {
  Key,
  Value,
  Entry,
};

} // namespace vm
} // namespace hermes

#endif
