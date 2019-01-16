// RUN: %hermes -O -dump-ir %s

// In this example, the store for 'bar' was removed since 'baz' is never called.
// Having no stores caused an assertion failure, even though all loads were dead
// and would have been removed in later passes.

foo(function() {
  function bar(){
    return true;
  }
  function baz(x){
    if(bar(x)) {
      return 1;
    }
  }
});
