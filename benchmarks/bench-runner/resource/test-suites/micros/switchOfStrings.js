var start = Date.now();
(function () {
  function stringSwitch(s) {
    switch (s) {
    case "a":
      return 1;
    case "b":
      return 2;
    case "c":
      return 3;
    case "d":
      return 4;
    case "e":
      return 5;
    case "f":
      return 6;
    case "g":
      return 7;
    case "h":
      return 8;
    case "i":
      return 9;
    case "j":
      return 10;
    case "k":
      return 11;
    case "l":
      return 12;
    case "m":
      return 13;
    case "n":
      return 14;
    case "o":
      return 15;
    case "p":
      return 16;
    case "q":
      return 17;
    case "r":
      return 18;
    case "s":
      return 19;
    case "t":
      return 20;
    case "u":
      return 21;
    case "v":
      return 22;
    case "w":
      return 23;
    case "x":
      return 24;
    case "y":
      return 25;
    case "z":
      return 26;
    }
    return 0;
  }

  var numIter = 500000;
  var cases = ["a", "b", "c", "d", "e", "f", "g", "h", "i", "j",
               "k", "l", "m", "n", "o", "p", "q", "r", "s", "t",
               "u", "v", "w", "x", "y", "z"];

  var sum = 0;
  for (var i = 0; i < numIter; i++) {
    for (var j = 0; j < 26; j++) {
      sum += stringSwitch(cases[j]);
    }
  }
  print(sum);
})();
var end = Date.now();
print("Time: " + (end - start));
