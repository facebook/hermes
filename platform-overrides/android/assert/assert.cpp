#include <assert.h>

// Provide a prototype to silence missing prototype warning in release
// mode.
extern "C" void
hermes_assert_fail(const char *file, int line, const char *expr);

extern "C" void
hermes_assert_fail(const char *file, int line, const char *expr) {
  // Save the arguments to memory to make it possible to recover them from a
  // stack dump.
  const char *volatile saveFile = file;
  int volatile saveLine = line;
  const char *volatile saveExpr = expr;
  (void)saveFile;
  (void)saveLine;
  (void)saveExpr;
  __builtin_trap();
}
