const addon = loadAddon('2_function_arguments');

const { add } = addon;

assert(typeof add === "function");
assert(add(3, 5) === 8);
