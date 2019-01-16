#ifndef HERMES_VM_MARKUNUSED_H
#define HERMES_VM_MARKUNUSED_H

namespace hermes {
namespace vm {

/// Indicates whether or not operations that free up pages should return them to
/// the OS.
enum class AdviseUnused { No = 0, Yes };

} // namespace vm
} // namespace hermes

#endif // HERMES_VM_MARKUNUSED_H
