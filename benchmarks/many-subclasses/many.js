/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

class P {
    x;
    y;
    constructor() {
        this.x = 10;
        this.y = 20;
    }
}

class C1 extends P {
  p1;
  constructor() {
    super();
    this.p1 = 1;
  }
}

class C2 extends P {
  p2;
  constructor() {
    super();
    this.p2 = 2;
  }
}

class C3 extends P {
  p3;
  constructor() {
    super();
    this.p3 = 3;
  }
}

class C4 extends P {
  p4;
  constructor() {
    super();
    this.p4 = 4;
  }
}

class C5 extends P {
  p5;
  constructor() {
    super();
    this.p5 = 5;
  }
}

class C6 extends P {
  p6;
  constructor() {
    super();
    this.p6 = 6;
  }
}

class C7 extends P {
  p7;
  constructor() {
    super();
    this.p7 = 7;
  }
}

class C8 extends P {
  p8;
  constructor() {
    super();
    this.p8 = 8;
  }
}

class C9 extends P {
  p9;
  constructor() {
    super();
    this.p9 = 9;
  }
}

class C10 extends P {
  p10;
  constructor() {
    super();
    this.p10 = 10;
  }
}

class C11 extends P {
  p11;
  constructor() {
    super();
    this.p11 = 11;
  }
}

class C12 extends P {
  p12;
  constructor() {
    super();
    this.p12 = 12;
  }
}

class C13 extends P {
  p13;
  constructor() {
    super();
    this.p13 = 13;
  }
}

class C14 extends P {
  p14;
  constructor() {
    super();
    this.p14 = 14;
  }
}

class C15 extends P {
  p15;
  constructor() {
    super();
    this.p15 = 15;
  }
}

class C16 extends P {
  p16;
  constructor() {
    super();
    this.p16 = 16;
  }
}

let children = [
  new C1(),
  new C2(),
  new C3(),
  new C4(),
  new C5(),
  new C6(),
  new C7(),
  new C8(),
  new C9(),
  new C10(),
  new C11(),
  new C12(),
  new C13(),
  new C14(),
  new C15(),
  new C16(),
]


let M = 16;
const N = 300000000;

let arr = [];
for(let i = 0; i < 10000; ++i) {
    let ch = Math.random()*M | 0;
    arr.push(children[ch]);
}


let res = 0;
function sum(o) {
    res += o.x + o.y;
}

var t1 = Date.now();

function bench() {
    var len = arr.length;
    for(var i = 0; i < N; ++i) {
      sum(arr[i % len]);
    }
}
bench();

print(res, `M=${M}, N=${N}, time=${Date.now() - t1}`);
