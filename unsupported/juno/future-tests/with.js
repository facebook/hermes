function f2(x) {
  with (x) {
    function g() {
      print(typeof g); // function
      print(h); // 2
    }
  }
  g();
  print(x.g); // 1
}
f2({g: 1, h: 2});
