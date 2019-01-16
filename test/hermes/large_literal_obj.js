// RUN: echo "var obj ={0" ":0,"{1..65535} ":0};" | %hermes -target=HBC -O
// The above echo generates JavaScript code like this:
// var obj = {
//   0:0,
//   1:0,
//   2:0,
//   3:0,
//   ...
//   65535:0
// };
// It's a literal object that contains 65536 literal entries. This test is used
// to exercise literal object optimizations and make sure it handles size of literal
// objects that does not fit in 2 bytes.
