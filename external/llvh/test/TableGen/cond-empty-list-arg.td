// RUN: %llvm-tblgen %s
// XFAIL: vg_leak

class C<bit cond> {
  bit True = 1;
  list<int> X = !cond(cond: [1, 2, 3], True : []);
  list<int> Y = !cond(cond: [], True : [4, 5, 6]);
}
