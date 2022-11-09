// RUN: (! %llvm-tblgen -I %p %s 2>&1) | %FileCheck %s

// CHECK: error: #endif without #ifdef
#endif
