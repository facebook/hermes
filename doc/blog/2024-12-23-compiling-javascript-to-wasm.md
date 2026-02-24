# Compiling Full-Featured JavaScript to Wasm

*December 23, 2024 · tmikov*

Compiling JavaScript to Wasm ahead of time has always been a fascinating topic. However, to my knowledge, there is no close-to-production-quality solution currently capable of statically compiling the full ES6 JavaScript language to Wasm without relying on an interpreter.

I would like to demonstrate how easily Static Hermes enables us to do precisely that.

This article has been a long time coming, since I have posted more than once that Static Hermes can produce Wasm, but I never actually had the time to sit down, write the instructions, and fix a couple of minor problems that appeared.

> **NOTE:** [Porffor](https://porffor.dev/) definitely deserves a mention here: it is an exciting emerging project exploring this space while taking a different approach. While it is not production quality yet, it shows real promise.

## But Why Do It at All?

Compiling JS to Wasm, while perhaps not a game-changer for everyday JavaScript execution, opens up a world of intriguing possibilities. For example:

- It provides sandbox isolation.
- It allows Wasm hosts that lack a JS engine to still run JavaScript.
- It allows standalone applications to embed the runtime, built-ins, and compiled code into a single Wasm binary.

## What is Static Hermes (and Hermes)

Hermes is an optimized JavaScript engine designed to execute JavaScript efficiently on resource-constrained devices, particularly phones. It is the default JS engine of React Native, a cross-platform mobile development framework. Hermes has been deployed to hundreds of millions of devices, including desktop computers and servers.

Static Hermes (which lives in the [static_h branch](https://github.com/facebook/hermes/tree/static_h)) is the code name for the next generation of Hermes, which, among other things, focuses on optional sound typing and optional ahead-of-time (AOT) compilation of JavaScript to native code.

The native compilation aspect of Static Hermes is what makes it particularly suitable to the task at hand - compiling JavaScript to Wasm.

Static Hermes already supports compiling the full JavaScript language to native code. By leveraging Clang and LLVM for code generation, it can treat WebAssembly (Wasm) as just another compiler target. This approach allows Static Hermes to compile the complete ES6 JavaScript language into Wasm while preserving the language's flexibility and dynamic nature.

The resulting Wasm binaries are highly comprehensive and contain:

- **Compiled JavaScript:** The input JavaScript translated directly into Wasm instructions for efficient execution, without the JS interpreter. We will look at a detailed example later in this article.
- **JavaScript Library:** All standard built-ins, such as Object, Array, and Math, are included to ensure compatibility.
- **Embedded Interpreter:** Dynamic features like `eval()` and `new Function()` are supported through an integrated interpreter, activated only when required.

In the next section, we will see how to build Static Hermes and use it to compile JavaScript to Wasm and how to run the resulting Wasm binaries.

## Getting Started with Static Hermes and Wasm

For those interested in exploring Static Hermes for compiling JavaScript to WebAssembly, the process is straightforward and involves a few key steps. Rather than detailing them all here, I have created a guide to walk through:

1. Setting up the development environment with Emscripten and the static_h branch of Hermes.
2. Building the Hermes JSVM for your host platform and for Wasm.
3. Running simple JavaScript applications and benchmarks using the Hermes VM Wasm binary. This involves running the Hermes VM in Wasm mode, which functions as an interpreter.

The final step, compiling JavaScript applications to independent Wasm binaries, highlights Static Hermes's unique capabilities. This process produces a fully self-contained Wasm binary.

The guide includes the necessary commands, configurations, and options to begin experimenting with Static Hermes and Wasm. It can be found here: [doc/Emscripten.md](https://github.com/facebook/hermes/blob/static_h/doc/Emscripten.md).

If you find inaccuracies in the guide, or have ideas how to improve it, PRs and comments are welcome.

Please, keep in mind that while the Static Hermes interpreter has already been shipped in production, the native compilation functionality is still considered experimental (but is not expected to have major bugs).

## Exploring the Generated Wasm Code

To understand the results of Static Hermes's compilation, let's take a closer look at a simple example. The following JavaScript function computes the sum of integers from 0 to a given number `to`:

```javascript
function sum(to) {
    to = +to;
    let sum = 0;
    for(let i = 0; i < to; ++i)
        sum += i;
    return sum;
}
```

The statement `to = +to` is interesting. It ensures that `to` is explicitly converted to a number, enabling us to infer types throughout the function. Type inference allows Static Hermes to optimize the generated Wasm code by treating `to`, `i`, and `sum` as numbers consistently, avoiding the overhead of type checks and conversions during execution.

> **NOTE:** The example would work without that, but would be slower and much more verbose.

After compilation with Static Hermes, this function is transformed into WebAssembly (Wasm) code. The assembly listing is too long to reproduce here, but we can examine the most important part - the loop.

```wasm
loop  ;; label = @2
  local.get 5          ;; Get the current sum
  local.get 6          ;; Get the current value of `i`
  f64.add              ;; Add `i` to the sum
  local.set 6          ;; Store the result back into `sum`
  local.get 5          ;; Increment `i`
  f64.const 0x1p+0     ;; Load the constant 1 (as a floating point)
  f64.add              ;; Add 1 to `i`
  local.tee 5          ;; Store the incremented value into `i` and leave it on the stack
  local.get 4          ;; Get the upper bound `to`
  f64.lt               ;; Compare `i` with `to`
  br_if 0 (;@2;)       ;; If `i >= to`, break
end
```

This Wasm loop corresponds directly to the JavaScript `for` loop. Key operations such as updating the sum, incrementing the loop counter, and checking the loop condition are reflected in the Wasm instructions:

- `local.get` and `local.set` manage local variables like `sum` and `i`.
- `f64.add` handles floating-point addition, used here to increment `i` and update `sum`.
- `f64.lt` compares `i` to the loop limit `to`.
- `br_if` provides the conditional branching mechanism to terminate the loop.

### Observations

- **Explicitness:** The Wasm representation is verbose, reflecting its low-level nature. Each operation (e.g., addition, comparison) is explicitly represented, enabling precise control over execution.
- **Efficiency:** The use of floating-point instructions (`f64.*`) reflects the type inference the compiler is able to perform, turning dynamic typing into static.
- **Direct Mapping:** The loop in the JavaScript code maps fairly cleanly to the Wasm code. This is not always the case with more complex code and optimizations, but is fun to see.

## Advanced JavaScript

I have prepared a small script, `demo.js`, demonstrating some more advanced JavaScript features, including closures, TDZ behavior, and prototype manipulation, which are faithfully supported by Static Hermes when compiled to Wasm.

### Compile and Run demo.js

Assuming you have just successfully completed the "Getting Started" instructions, make sure `demo.js` is in the current directory and compile it.

```bash
${HermesSourcePath?}/utils/wasm-compile.sh build-host build-wasm demo.js
```

The command should output something like:

```
Using shermes to compile demo.js... to demo.c
Using emcc to compile demo.c to demo.o
Using emcc to link demo.o to demo-wasm.js/.wasm
-rw-r--r--  1 user group    73K Dec 23 17:35 demo-wasm.js
-rwxr-xr-x  1 user group   2.6M Dec 23 17:35 demo-wasm.wasm
```

Now we can run the result and enjoy the awesome power of Wasm!

```bash
node ./demo-wasm.js
```

```
Closures
=========
Generating 5 increments with default step size:
1
... and so on
```

The following sections provide a brief tour of `demo.js`, showcasing that Static Hermes handles various JavaScript features.

### Closures and Generators

```javascript
function createCounterWithGenerator() {
    let count = 0; // Shared mutable variable

    // Default parameter.
    const increment = (by = 1) => count += by;

    // Generator captures a peer closure (increment).
    function* doSteps(steps) {
        for (let i = 0; i < steps; i++)
            yield increment();
    }

    // Shorthand object literal.
    return {
        increment,
        doSteps
    };
}

const counter = createCounterWithGenerator();

// Tagged template literal
console.log(`Generating ${steps} increments with default step size:`);
// For-of with a generator
for (const value of counter.doSteps(steps))
    console.log(value);
```

### Temporal Dead Zone (TDZ)

The `show_tdz()` function illustrates TDZ errors arising from improper use of `let`.

```javascript
function show_tdz() {
    function getval() {
        return val;
    }
    // Reading val before it is initialized.
    let val = getval() + 1;
}

console.log("\nTDZ\n=========");
try {
    show_tdz();
} catch (e) {
    console.log("TDZ Error!", e.stack);
}
```

### Prototypical Inheritance

The example uses prototype chains and dynamic getters, showcasing inheritance and dynamic property manipulation.

```javascript
const prototypeObj = {
    first: "I am in the prototype"
};

const obj = {
    get second() {
        // Add and increment the `third` property
        if (!this.third) {
            this.third = 1; // Initialize if it doesn't exist
        } else {
            this.third++;
        }
        return `Getter executed, third is now ${this.third}`;
    },

    __proto__: prototypeObj // Set prototype using object literal syntax
};

console.log("First property:", obj.first); // Inherited from prototype
console.log("Second property:", obj.second); // Triggers the getter
console.log("Third property:", obj.third); // Dynamically added and incremented
console.log("Second property again:", obj.second); // Getter increments third
console.log("Third property now:", obj.third); // Reflects incremented value
```

### ES6 Classes

Demonstrates inheritance and property handling with ES6 classes.

```javascript
class PrototypeClass {
    constructor() {}

    // Define `first` as a getter in the prototype
    get first() {
        return "I am in the prototype";
    }
}

class DerivedClass extends PrototypeClass {
    constructor() {
        super();
    }

    get second() {
        // Add and increment the `third` property
        if (!this.third) {
            this.third = 1; // Initialize if it doesn't exist
        } else {
            this.third++;
        }
        return `Getter executed, third is now ${this.third}`;
    }
}

const clInst = new DerivedClass();

console.log("First property:", clInst.first); // Inherited from PrototypeClass
console.log("Second property:", clInst.second); // Triggers the getter
console.log("Third property:", clInst.third); // Dynamically added and incremented
console.log("Second property again:", clInst.second); // Getter increments third
console.log("Third property now:", clInst.third); // Reflects incremented value
```

## Conclusion

Static Hermes is an ambitious project that pushes the boundaries of what JavaScript can do. The ability to compile the full ES6 standard to Wasm, while preserving JavaScript's flexibility, opens up some fascinating possibilities.

If this kind of thing interests you, check out the [Static Hermes source code](https://github.com/facebook/hermes/tree/static_h). Try it out, poke around, and feel free to ask questions. Contributions and discussions are always welcome.
