// Scoped function declarations assign at point of execution.
function f1() {
  var g;
  g = 10;
  { function g() {} }
  print(typeof g);
}
f1(); // function

// Unscoped function declarations are hoisted to before the function begins.
function f2() {
  var g;
  g = 10;
  function g() {}
  print(typeof g);
}
f2(); // number
