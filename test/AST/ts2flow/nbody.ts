/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -typed -parse-ts -dump-ast -pretty -script %s | %FileCheck --match-full-lines %s

"use strict";
(function main() {
  /* The Computer Language Benchmarks Game
   https://salsa.debian.org/benchmarksgame-team/benchmarksgame/
   contributed by Isaac Gouy
   modified by Andrey Filatkin */

  const PI: number = Math.PI;
  const SOLAR_MASS: number = 4 * PI * PI;
  const DAYS_PER_YEAR = 365.24;

  class Body {
    x: number;
    y: number;
    z: number;
    vx: number;
    vy: number;
    vz: number;
    mass: number;

    constructor(
      x: number,
      y: number,
      z: number,
      vx: number,
      vy: number,
      vz: number,
      mass: number
    ) {
      this.x = x;
      this.y = y;
      this.z = z;
      this.vx = vx;
      this.vy = vy;
      this.vz = vz;
      this.mass = mass;
    }
  }

  function Jupiter(): Body {
    return new Body(
      4.8414314424647209,
      -1.16032004402742839,
      -1.03622044471123109e-1,
      1.66007664274403694e-3 * DAYS_PER_YEAR,
      7.69901118419740425e-3 * DAYS_PER_YEAR,
      -6.90460016972063023e-5 * DAYS_PER_YEAR,
      9.54791938424326609e-4 * SOLAR_MASS
    );
  }

  function Saturn(): Body {
    return new Body(
      8.34336671824457987,
      4.12479856412430479,
      -4.03523417114321381e-1,
      -2.76742510726862411e-3 * DAYS_PER_YEAR,
      4.99852801234917238e-3 * DAYS_PER_YEAR,
      2.30417297573763929e-5 * DAYS_PER_YEAR,
      2.85885980666130812e-4 * SOLAR_MASS
    );
  }

  function Uranus(): Body {
    return new Body(
      1.2894369562139131e1,
      -1.51111514016986312e1,
      -2.23307578892655734e-1,
      2.96460137564761618e-3 * DAYS_PER_YEAR,
      2.3784717395948095e-3 * DAYS_PER_YEAR,
      -2.96589568540237556e-5 * DAYS_PER_YEAR,
      4.36624404335156298e-5 * SOLAR_MASS
    );
  }

  function Neptune(): Body {
    return new Body(
      1.53796971148509165e1,
      -2.59193146099879641e1,
      1.79258772950371181e-1,
      2.68067772490389322e-3 * DAYS_PER_YEAR,
      1.62824170038242295e-3 * DAYS_PER_YEAR,
      -9.5159225451971587e-5 * DAYS_PER_YEAR,
      5.15138902046611451e-5 * SOLAR_MASS
    );
  }

  function Sun(): Body {
    return new Body(0.0, 0.0, 0.0, 0.0, 0.0, 0.0, SOLAR_MASS);
  }

  const bodies: Body[] = [Sun(), Jupiter(), Saturn(), Uranus(), Neptune()];

  function offsetMomentum(): void {
    let px = 0;
    let py = 0;
    let pz = 0;
    const size: number = bodies.length;
    for (let i = 0; i < size; i++) {
      const body: Body = bodies[i];
      const mass: number = body.mass;
      px += body.vx * mass;
      py += body.vy * mass;
      pz += body.vz * mass;
    }

    const body: Body = bodies[0];
    body.vx = -px / SOLAR_MASS;
    body.vy = -py / SOLAR_MASS;
    body.vz = -pz / SOLAR_MASS;
  }

  function advance(dt: number) {
    const size: number = bodies.length;

    for (let i: number = 0; i < size; i++) {
      const bodyi: Body = bodies[i];
      let vxi: number = bodyi.vx;
      let vyi: number = bodyi.vy;
      let vzi: number = bodyi.vz;
      for (let j: number = i + 1; j < size; j++) {
        const bodyj: Body = bodies[j];
        const dx: number = bodyi.x - bodyj.x;
        const dy: number = bodyi.y - bodyj.y;
        const dz: number = bodyi.z - bodyj.z;

        const d2: number = dx * dx + dy * dy + dz * dz;
        const mag: number = dt / (d2 * Math.sqrt(d2));

        const massj: number = bodyj.mass;
        vxi -= dx * massj * mag;
        vyi -= dy * massj * mag;
        vzi -= dz * massj * mag;

        const massi: number = bodyi.mass;
        bodyj.vx += dx * massi * mag;
        bodyj.vy += dy * massi * mag;
        bodyj.vz += dz * massi * mag;
      }
      bodyi.vx = vxi;
      bodyi.vy = vyi;
      bodyi.vz = vzi;
    }

    for (let i: number = 0; i < size; i++) {
      const body: Body = bodies[i];
      body.x += dt * body.vx;
      body.y += dt * body.vy;
      body.z += dt * body.vz;
    }
  }

  function energy(): number {
    let e = 0;
    const size: number = bodies.length;

    for (let i = 0; i < size; i++) {
      const bodyi: Body = bodies[i];

      e +=
        0.5 *
        bodyi.mass *
        (bodyi.vx * bodyi.vx + bodyi.vy * bodyi.vy + bodyi.vz * bodyi.vz);

      for (let j = i + 1; j < size; j++) {
        const bodyj: Body = bodies[j];
        const dx: number = bodyi.x - bodyj.x;
        const dy: number = bodyi.y - bodyj.y;
        const dz: number = bodyi.z - bodyj.z;

        const distance: number = Math.sqrt(dx * dx + dy * dy + dz * dz);
        e -= (bodyi.mass * bodyj.mass) / distance;
      }
    }
    return e;
  }

  function nbody(): number {
    const n = 400_000;
    //const n = 100;

    offsetMomentum();
    for (let i = 0; i < n; i++) {
      advance(0.01);
    }

    return energy();
  }
  // @ts-ignore TODO: add the type declaration for print()
  print(nbody());
})();

// CHECK:{
// CHECK-NEXT:  "type": "Program",
// CHECK-NEXT:  "body": [
// CHECK-NEXT:    {
// CHECK-NEXT:      "type": "ExpressionStatement",
// CHECK-NEXT:      "expression": {
// CHECK-NEXT:        "type": "StringLiteral",
// CHECK-NEXT:        "value": "use strict"
// CHECK-NEXT:      },
// CHECK-NEXT:      "directive": "use strict"
// CHECK-NEXT:    },
// CHECK-NEXT:    {
// CHECK-NEXT:      "type": "ExpressionStatement",
// CHECK-NEXT:      "expression": {
// CHECK-NEXT:        "type": "CallExpression",
// CHECK-NEXT:        "callee": {
// CHECK-NEXT:          "type": "FunctionExpression",
// CHECK-NEXT:          "id": {
// CHECK-NEXT:            "type": "Identifier",
// CHECK-NEXT:            "name": "main"
// CHECK-NEXT:          },
// CHECK-NEXT:          "params": [],
// CHECK-NEXT:          "body": {
// CHECK-NEXT:            "type": "BlockStatement",
// CHECK-NEXT:            "body": [
// CHECK-NEXT:              {
// CHECK-NEXT:                "type": "VariableDeclaration",
// CHECK-NEXT:                "kind": "const",
// CHECK-NEXT:                "declarations": [
// CHECK-NEXT:                  {
// CHECK-NEXT:                    "type": "VariableDeclarator",
// CHECK-NEXT:                    "init": {
// CHECK-NEXT:                      "type": "MemberExpression",
// CHECK-NEXT:                      "object": {
// CHECK-NEXT:                        "type": "Identifier",
// CHECK-NEXT:                        "name": "Math"
// CHECK-NEXT:                      },
// CHECK-NEXT:                      "property": {
// CHECK-NEXT:                        "type": "Identifier",
// CHECK-NEXT:                        "name": "PI"
// CHECK-NEXT:                      },
// CHECK-NEXT:                      "computed": false
// CHECK-NEXT:                    },
// CHECK-NEXT:                    "id": {
// CHECK-NEXT:                      "type": "Identifier",
// CHECK-NEXT:                      "name": "PI",
// CHECK-NEXT:                      "typeAnnotation": {
// CHECK-NEXT:                        "type": "TypeAnnotation",
// CHECK-NEXT:                        "typeAnnotation": {
// CHECK-NEXT:                          "type": "NumberTypeAnnotation"
// CHECK-NEXT:                        }
// CHECK-NEXT:                      }
// CHECK-NEXT:                    }
// CHECK-NEXT:                  }
// CHECK-NEXT:                ]
// CHECK-NEXT:              },
// CHECK-NEXT:              {
// CHECK-NEXT:                "type": "VariableDeclaration",
// CHECK-NEXT:                "kind": "const",
// CHECK-NEXT:                "declarations": [
// CHECK-NEXT:                  {
// CHECK-NEXT:                    "type": "VariableDeclarator",
// CHECK-NEXT:                    "init": {
// CHECK-NEXT:                      "type": "BinaryExpression",
// CHECK-NEXT:                      "left": {
// CHECK-NEXT:                        "type": "BinaryExpression",
// CHECK-NEXT:                        "left": {
// CHECK-NEXT:                          "type": "NumericLiteral",
// CHECK-NEXT:                          "value": 4,
// CHECK-NEXT:                          "raw": "4"
// CHECK-NEXT:                        },
// CHECK-NEXT:                        "right": {
// CHECK-NEXT:                          "type": "Identifier",
// CHECK-NEXT:                          "name": "PI"
// CHECK-NEXT:                        },
// CHECK-NEXT:                        "operator": "*"
// CHECK-NEXT:                      },
// CHECK-NEXT:                      "right": {
// CHECK-NEXT:                        "type": "Identifier",
// CHECK-NEXT:                        "name": "PI"
// CHECK-NEXT:                      },
// CHECK-NEXT:                      "operator": "*"
// CHECK-NEXT:                    },
// CHECK-NEXT:                    "id": {
// CHECK-NEXT:                      "type": "Identifier",
// CHECK-NEXT:                      "name": "SOLAR_MASS",
// CHECK-NEXT:                      "typeAnnotation": {
// CHECK-NEXT:                        "type": "TypeAnnotation",
// CHECK-NEXT:                        "typeAnnotation": {
// CHECK-NEXT:                          "type": "NumberTypeAnnotation"
// CHECK-NEXT:                        }
// CHECK-NEXT:                      }
// CHECK-NEXT:                    }
// CHECK-NEXT:                  }
// CHECK-NEXT:                ]
// CHECK-NEXT:              },
// CHECK-NEXT:              {
// CHECK-NEXT:                "type": "VariableDeclaration",
// CHECK-NEXT:                "kind": "const",
// CHECK-NEXT:                "declarations": [
// CHECK-NEXT:                  {
// CHECK-NEXT:                    "type": "VariableDeclarator",
// CHECK-NEXT:                    "init": {
// CHECK-NEXT:                      "type": "NumericLiteral",
// CHECK-NEXT:                      "value": 365.24,
// CHECK-NEXT:                      "raw": "365.24"
// CHECK-NEXT:                    },
// CHECK-NEXT:                    "id": {
// CHECK-NEXT:                      "type": "Identifier",
// CHECK-NEXT:                      "name": "DAYS_PER_YEAR"
// CHECK-NEXT:                    }
// CHECK-NEXT:                  }
// CHECK-NEXT:                ]
// CHECK-NEXT:              },
// CHECK-NEXT:              {
// CHECK-NEXT:                "type": "ClassDeclaration",
// CHECK-NEXT:                "id": {
// CHECK-NEXT:                  "type": "Identifier",
// CHECK-NEXT:                  "name": "Body"
// CHECK-NEXT:                },
// CHECK-NEXT:                "superClass": null,
// CHECK-NEXT:                "body": {
// CHECK-NEXT:                  "type": "ClassBody",
// CHECK-NEXT:                  "body": [
// CHECK-NEXT:                    {
// CHECK-NEXT:                      "type": "ClassProperty",
// CHECK-NEXT:                      "key": {
// CHECK-NEXT:                        "type": "Identifier",
// CHECK-NEXT:                        "name": "x"
// CHECK-NEXT:                      },
// CHECK-NEXT:                      "value": null,
// CHECK-NEXT:                      "computed": false,
// CHECK-NEXT:                      "static": false,
// CHECK-NEXT:                      "declare": false,
// CHECK-NEXT:                      "typeAnnotation": {
// CHECK-NEXT:                        "type": "TypeAnnotation",
// CHECK-NEXT:                        "typeAnnotation": {
// CHECK-NEXT:                          "type": "NumberTypeAnnotation"
// CHECK-NEXT:                        }
// CHECK-NEXT:                      }
// CHECK-NEXT:                    },
// CHECK-NEXT:                    {
// CHECK-NEXT:                      "type": "ClassProperty",
// CHECK-NEXT:                      "key": {
// CHECK-NEXT:                        "type": "Identifier",
// CHECK-NEXT:                        "name": "y"
// CHECK-NEXT:                      },
// CHECK-NEXT:                      "value": null,
// CHECK-NEXT:                      "computed": false,
// CHECK-NEXT:                      "static": false,
// CHECK-NEXT:                      "declare": false,
// CHECK-NEXT:                      "typeAnnotation": {
// CHECK-NEXT:                        "type": "TypeAnnotation",
// CHECK-NEXT:                        "typeAnnotation": {
// CHECK-NEXT:                          "type": "NumberTypeAnnotation"
// CHECK-NEXT:                        }
// CHECK-NEXT:                      }
// CHECK-NEXT:                    },
// CHECK-NEXT:                    {
// CHECK-NEXT:                      "type": "ClassProperty",
// CHECK-NEXT:                      "key": {
// CHECK-NEXT:                        "type": "Identifier",
// CHECK-NEXT:                        "name": "z"
// CHECK-NEXT:                      },
// CHECK-NEXT:                      "value": null,
// CHECK-NEXT:                      "computed": false,
// CHECK-NEXT:                      "static": false,
// CHECK-NEXT:                      "declare": false,
// CHECK-NEXT:                      "typeAnnotation": {
// CHECK-NEXT:                        "type": "TypeAnnotation",
// CHECK-NEXT:                        "typeAnnotation": {
// CHECK-NEXT:                          "type": "NumberTypeAnnotation"
// CHECK-NEXT:                        }
// CHECK-NEXT:                      }
// CHECK-NEXT:                    },
// CHECK-NEXT:                    {
// CHECK-NEXT:                      "type": "ClassProperty",
// CHECK-NEXT:                      "key": {
// CHECK-NEXT:                        "type": "Identifier",
// CHECK-NEXT:                        "name": "vx"
// CHECK-NEXT:                      },
// CHECK-NEXT:                      "value": null,
// CHECK-NEXT:                      "computed": false,
// CHECK-NEXT:                      "static": false,
// CHECK-NEXT:                      "declare": false,
// CHECK-NEXT:                      "typeAnnotation": {
// CHECK-NEXT:                        "type": "TypeAnnotation",
// CHECK-NEXT:                        "typeAnnotation": {
// CHECK-NEXT:                          "type": "NumberTypeAnnotation"
// CHECK-NEXT:                        }
// CHECK-NEXT:                      }
// CHECK-NEXT:                    },
// CHECK-NEXT:                    {
// CHECK-NEXT:                      "type": "ClassProperty",
// CHECK-NEXT:                      "key": {
// CHECK-NEXT:                        "type": "Identifier",
// CHECK-NEXT:                        "name": "vy"
// CHECK-NEXT:                      },
// CHECK-NEXT:                      "value": null,
// CHECK-NEXT:                      "computed": false,
// CHECK-NEXT:                      "static": false,
// CHECK-NEXT:                      "declare": false,
// CHECK-NEXT:                      "typeAnnotation": {
// CHECK-NEXT:                        "type": "TypeAnnotation",
// CHECK-NEXT:                        "typeAnnotation": {
// CHECK-NEXT:                          "type": "NumberTypeAnnotation"
// CHECK-NEXT:                        }
// CHECK-NEXT:                      }
// CHECK-NEXT:                    },
// CHECK-NEXT:                    {
// CHECK-NEXT:                      "type": "ClassProperty",
// CHECK-NEXT:                      "key": {
// CHECK-NEXT:                        "type": "Identifier",
// CHECK-NEXT:                        "name": "vz"
// CHECK-NEXT:                      },
// CHECK-NEXT:                      "value": null,
// CHECK-NEXT:                      "computed": false,
// CHECK-NEXT:                      "static": false,
// CHECK-NEXT:                      "declare": false,
// CHECK-NEXT:                      "typeAnnotation": {
// CHECK-NEXT:                        "type": "TypeAnnotation",
// CHECK-NEXT:                        "typeAnnotation": {
// CHECK-NEXT:                          "type": "NumberTypeAnnotation"
// CHECK-NEXT:                        }
// CHECK-NEXT:                      }
// CHECK-NEXT:                    },
// CHECK-NEXT:                    {
// CHECK-NEXT:                      "type": "ClassProperty",
// CHECK-NEXT:                      "key": {
// CHECK-NEXT:                        "type": "Identifier",
// CHECK-NEXT:                        "name": "mass"
// CHECK-NEXT:                      },
// CHECK-NEXT:                      "value": null,
// CHECK-NEXT:                      "computed": false,
// CHECK-NEXT:                      "static": false,
// CHECK-NEXT:                      "declare": false,
// CHECK-NEXT:                      "typeAnnotation": {
// CHECK-NEXT:                        "type": "TypeAnnotation",
// CHECK-NEXT:                        "typeAnnotation": {
// CHECK-NEXT:                          "type": "NumberTypeAnnotation"
// CHECK-NEXT:                        }
// CHECK-NEXT:                      }
// CHECK-NEXT:                    },
// CHECK-NEXT:                    {
// CHECK-NEXT:                      "type": "MethodDefinition",
// CHECK-NEXT:                      "key": {
// CHECK-NEXT:                        "type": "Identifier",
// CHECK-NEXT:                        "name": "constructor"
// CHECK-NEXT:                      },
// CHECK-NEXT:                      "value": {
// CHECK-NEXT:                        "type": "FunctionExpression",
// CHECK-NEXT:                        "id": null,
// CHECK-NEXT:                        "params": [
// CHECK-NEXT:                          {
// CHECK-NEXT:                            "type": "Identifier",
// CHECK-NEXT:                            "name": "x",
// CHECK-NEXT:                            "typeAnnotation": {
// CHECK-NEXT:                              "type": "TypeAnnotation",
// CHECK-NEXT:                              "typeAnnotation": {
// CHECK-NEXT:                                "type": "NumberTypeAnnotation"
// CHECK-NEXT:                              }
// CHECK-NEXT:                            }
// CHECK-NEXT:                          },
// CHECK-NEXT:                          {
// CHECK-NEXT:                            "type": "Identifier",
// CHECK-NEXT:                            "name": "y",
// CHECK-NEXT:                            "typeAnnotation": {
// CHECK-NEXT:                              "type": "TypeAnnotation",
// CHECK-NEXT:                              "typeAnnotation": {
// CHECK-NEXT:                                "type": "NumberTypeAnnotation"
// CHECK-NEXT:                              }
// CHECK-NEXT:                            }
// CHECK-NEXT:                          },
// CHECK-NEXT:                          {
// CHECK-NEXT:                            "type": "Identifier",
// CHECK-NEXT:                            "name": "z",
// CHECK-NEXT:                            "typeAnnotation": {
// CHECK-NEXT:                              "type": "TypeAnnotation",
// CHECK-NEXT:                              "typeAnnotation": {
// CHECK-NEXT:                                "type": "NumberTypeAnnotation"
// CHECK-NEXT:                              }
// CHECK-NEXT:                            }
// CHECK-NEXT:                          },
// CHECK-NEXT:                          {
// CHECK-NEXT:                            "type": "Identifier",
// CHECK-NEXT:                            "name": "vx",
// CHECK-NEXT:                            "typeAnnotation": {
// CHECK-NEXT:                              "type": "TypeAnnotation",
// CHECK-NEXT:                              "typeAnnotation": {
// CHECK-NEXT:                                "type": "NumberTypeAnnotation"
// CHECK-NEXT:                              }
// CHECK-NEXT:                            }
// CHECK-NEXT:                          },
// CHECK-NEXT:                          {
// CHECK-NEXT:                            "type": "Identifier",
// CHECK-NEXT:                            "name": "vy",
// CHECK-NEXT:                            "typeAnnotation": {
// CHECK-NEXT:                              "type": "TypeAnnotation",
// CHECK-NEXT:                              "typeAnnotation": {
// CHECK-NEXT:                                "type": "NumberTypeAnnotation"
// CHECK-NEXT:                              }
// CHECK-NEXT:                            }
// CHECK-NEXT:                          },
// CHECK-NEXT:                          {
// CHECK-NEXT:                            "type": "Identifier",
// CHECK-NEXT:                            "name": "vz",
// CHECK-NEXT:                            "typeAnnotation": {
// CHECK-NEXT:                              "type": "TypeAnnotation",
// CHECK-NEXT:                              "typeAnnotation": {
// CHECK-NEXT:                                "type": "NumberTypeAnnotation"
// CHECK-NEXT:                              }
// CHECK-NEXT:                            }
// CHECK-NEXT:                          },
// CHECK-NEXT:                          {
// CHECK-NEXT:                            "type": "Identifier",
// CHECK-NEXT:                            "name": "mass",
// CHECK-NEXT:                            "typeAnnotation": {
// CHECK-NEXT:                              "type": "TypeAnnotation",
// CHECK-NEXT:                              "typeAnnotation": {
// CHECK-NEXT:                                "type": "NumberTypeAnnotation"
// CHECK-NEXT:                              }
// CHECK-NEXT:                            }
// CHECK-NEXT:                          }
// CHECK-NEXT:                        ],
// CHECK-NEXT:                        "body": {
// CHECK-NEXT:                          "type": "BlockStatement",
// CHECK-NEXT:                          "body": [
// CHECK-NEXT:                            {
// CHECK-NEXT:                              "type": "ExpressionStatement",
// CHECK-NEXT:                              "expression": {
// CHECK-NEXT:                                "type": "AssignmentExpression",
// CHECK-NEXT:                                "operator": "=",
// CHECK-NEXT:                                "left": {
// CHECK-NEXT:                                  "type": "MemberExpression",
// CHECK-NEXT:                                  "object": {
// CHECK-NEXT:                                    "type": "ThisExpression"
// CHECK-NEXT:                                  },
// CHECK-NEXT:                                  "property": {
// CHECK-NEXT:                                    "type": "Identifier",
// CHECK-NEXT:                                    "name": "x"
// CHECK-NEXT:                                  },
// CHECK-NEXT:                                  "computed": false
// CHECK-NEXT:                                },
// CHECK-NEXT:                                "right": {
// CHECK-NEXT:                                  "type": "Identifier",
// CHECK-NEXT:                                  "name": "x"
// CHECK-NEXT:                                }
// CHECK-NEXT:                              },
// CHECK-NEXT:                              "directive": null
// CHECK-NEXT:                            },
// CHECK-NEXT:                            {
// CHECK-NEXT:                              "type": "ExpressionStatement",
// CHECK-NEXT:                              "expression": {
// CHECK-NEXT:                                "type": "AssignmentExpression",
// CHECK-NEXT:                                "operator": "=",
// CHECK-NEXT:                                "left": {
// CHECK-NEXT:                                  "type": "MemberExpression",
// CHECK-NEXT:                                  "object": {
// CHECK-NEXT:                                    "type": "ThisExpression"
// CHECK-NEXT:                                  },
// CHECK-NEXT:                                  "property": {
// CHECK-NEXT:                                    "type": "Identifier",
// CHECK-NEXT:                                    "name": "y"
// CHECK-NEXT:                                  },
// CHECK-NEXT:                                  "computed": false
// CHECK-NEXT:                                },
// CHECK-NEXT:                                "right": {
// CHECK-NEXT:                                  "type": "Identifier",
// CHECK-NEXT:                                  "name": "y"
// CHECK-NEXT:                                }
// CHECK-NEXT:                              },
// CHECK-NEXT:                              "directive": null
// CHECK-NEXT:                            },
// CHECK-NEXT:                            {
// CHECK-NEXT:                              "type": "ExpressionStatement",
// CHECK-NEXT:                              "expression": {
// CHECK-NEXT:                                "type": "AssignmentExpression",
// CHECK-NEXT:                                "operator": "=",
// CHECK-NEXT:                                "left": {
// CHECK-NEXT:                                  "type": "MemberExpression",
// CHECK-NEXT:                                  "object": {
// CHECK-NEXT:                                    "type": "ThisExpression"
// CHECK-NEXT:                                  },
// CHECK-NEXT:                                  "property": {
// CHECK-NEXT:                                    "type": "Identifier",
// CHECK-NEXT:                                    "name": "z"
// CHECK-NEXT:                                  },
// CHECK-NEXT:                                  "computed": false
// CHECK-NEXT:                                },
// CHECK-NEXT:                                "right": {
// CHECK-NEXT:                                  "type": "Identifier",
// CHECK-NEXT:                                  "name": "z"
// CHECK-NEXT:                                }
// CHECK-NEXT:                              },
// CHECK-NEXT:                              "directive": null
// CHECK-NEXT:                            },
// CHECK-NEXT:                            {
// CHECK-NEXT:                              "type": "ExpressionStatement",
// CHECK-NEXT:                              "expression": {
// CHECK-NEXT:                                "type": "AssignmentExpression",
// CHECK-NEXT:                                "operator": "=",
// CHECK-NEXT:                                "left": {
// CHECK-NEXT:                                  "type": "MemberExpression",
// CHECK-NEXT:                                  "object": {
// CHECK-NEXT:                                    "type": "ThisExpression"
// CHECK-NEXT:                                  },
// CHECK-NEXT:                                  "property": {
// CHECK-NEXT:                                    "type": "Identifier",
// CHECK-NEXT:                                    "name": "vx"
// CHECK-NEXT:                                  },
// CHECK-NEXT:                                  "computed": false
// CHECK-NEXT:                                },
// CHECK-NEXT:                                "right": {
// CHECK-NEXT:                                  "type": "Identifier",
// CHECK-NEXT:                                  "name": "vx"
// CHECK-NEXT:                                }
// CHECK-NEXT:                              },
// CHECK-NEXT:                              "directive": null
// CHECK-NEXT:                            },
// CHECK-NEXT:                            {
// CHECK-NEXT:                              "type": "ExpressionStatement",
// CHECK-NEXT:                              "expression": {
// CHECK-NEXT:                                "type": "AssignmentExpression",
// CHECK-NEXT:                                "operator": "=",
// CHECK-NEXT:                                "left": {
// CHECK-NEXT:                                  "type": "MemberExpression",
// CHECK-NEXT:                                  "object": {
// CHECK-NEXT:                                    "type": "ThisExpression"
// CHECK-NEXT:                                  },
// CHECK-NEXT:                                  "property": {
// CHECK-NEXT:                                    "type": "Identifier",
// CHECK-NEXT:                                    "name": "vy"
// CHECK-NEXT:                                  },
// CHECK-NEXT:                                  "computed": false
// CHECK-NEXT:                                },
// CHECK-NEXT:                                "right": {
// CHECK-NEXT:                                  "type": "Identifier",
// CHECK-NEXT:                                  "name": "vy"
// CHECK-NEXT:                                }
// CHECK-NEXT:                              },
// CHECK-NEXT:                              "directive": null
// CHECK-NEXT:                            },
// CHECK-NEXT:                            {
// CHECK-NEXT:                              "type": "ExpressionStatement",
// CHECK-NEXT:                              "expression": {
// CHECK-NEXT:                                "type": "AssignmentExpression",
// CHECK-NEXT:                                "operator": "=",
// CHECK-NEXT:                                "left": {
// CHECK-NEXT:                                  "type": "MemberExpression",
// CHECK-NEXT:                                  "object": {
// CHECK-NEXT:                                    "type": "ThisExpression"
// CHECK-NEXT:                                  },
// CHECK-NEXT:                                  "property": {
// CHECK-NEXT:                                    "type": "Identifier",
// CHECK-NEXT:                                    "name": "vz"
// CHECK-NEXT:                                  },
// CHECK-NEXT:                                  "computed": false
// CHECK-NEXT:                                },
// CHECK-NEXT:                                "right": {
// CHECK-NEXT:                                  "type": "Identifier",
// CHECK-NEXT:                                  "name": "vz"
// CHECK-NEXT:                                }
// CHECK-NEXT:                              },
// CHECK-NEXT:                              "directive": null
// CHECK-NEXT:                            },
// CHECK-NEXT:                            {
// CHECK-NEXT:                              "type": "ExpressionStatement",
// CHECK-NEXT:                              "expression": {
// CHECK-NEXT:                                "type": "AssignmentExpression",
// CHECK-NEXT:                                "operator": "=",
// CHECK-NEXT:                                "left": {
// CHECK-NEXT:                                  "type": "MemberExpression",
// CHECK-NEXT:                                  "object": {
// CHECK-NEXT:                                    "type": "ThisExpression"
// CHECK-NEXT:                                  },
// CHECK-NEXT:                                  "property": {
// CHECK-NEXT:                                    "type": "Identifier",
// CHECK-NEXT:                                    "name": "mass"
// CHECK-NEXT:                                  },
// CHECK-NEXT:                                  "computed": false
// CHECK-NEXT:                                },
// CHECK-NEXT:                                "right": {
// CHECK-NEXT:                                  "type": "Identifier",
// CHECK-NEXT:                                  "name": "mass"
// CHECK-NEXT:                                }
// CHECK-NEXT:                              },
// CHECK-NEXT:                              "directive": null
// CHECK-NEXT:                            }
// CHECK-NEXT:                          ]
// CHECK-NEXT:                        },
// CHECK-NEXT:                        "generator": false,
// CHECK-NEXT:                        "async": false
// CHECK-NEXT:                      },
// CHECK-NEXT:                      "kind": "constructor",
// CHECK-NEXT:                      "computed": false,
// CHECK-NEXT:                      "static": false
// CHECK-NEXT:                    }
// CHECK-NEXT:                  ]
// CHECK-NEXT:                }
// CHECK-NEXT:              },
// CHECK-NEXT:              {
// CHECK-NEXT:                "type": "FunctionDeclaration",
// CHECK-NEXT:                "id": {
// CHECK-NEXT:                  "type": "Identifier",
// CHECK-NEXT:                  "name": "Jupiter"
// CHECK-NEXT:                },
// CHECK-NEXT:                "params": [],
// CHECK-NEXT:                "body": {
// CHECK-NEXT:                  "type": "BlockStatement",
// CHECK-NEXT:                  "body": [
// CHECK-NEXT:                    {
// CHECK-NEXT:                      "type": "ReturnStatement",
// CHECK-NEXT:                      "argument": {
// CHECK-NEXT:                        "type": "NewExpression",
// CHECK-NEXT:                        "callee": {
// CHECK-NEXT:                          "type": "Identifier",
// CHECK-NEXT:                          "name": "Body"
// CHECK-NEXT:                        },
// CHECK-NEXT:                        "arguments": [
// CHECK-NEXT:                          {
// CHECK-NEXT:                            "type": "NumericLiteral",
// CHECK-NEXT:                            "value": 4.841431442464721,
// CHECK-NEXT:                            "raw": "4.8414314424647209"
// CHECK-NEXT:                          },
// CHECK-NEXT:                          {
// CHECK-NEXT:                            "type": "UnaryExpression",
// CHECK-NEXT:                            "operator": "-",
// CHECK-NEXT:                            "argument": {
// CHECK-NEXT:                              "type": "NumericLiteral",
// CHECK-NEXT:                              "value": 1.1603200440274284,
// CHECK-NEXT:                              "raw": "1.16032004402742839"
// CHECK-NEXT:                            },
// CHECK-NEXT:                            "prefix": true
// CHECK-NEXT:                          },
// CHECK-NEXT:                          {
// CHECK-NEXT:                            "type": "UnaryExpression",
// CHECK-NEXT:                            "operator": "-",
// CHECK-NEXT:                            "argument": {
// CHECK-NEXT:                              "type": "NumericLiteral",
// CHECK-NEXT:                              "value": 0.10362204447112311,
// CHECK-NEXT:                              "raw": "1.03622044471123109e-1"
// CHECK-NEXT:                            },
// CHECK-NEXT:                            "prefix": true
// CHECK-NEXT:                          },
// CHECK-NEXT:                          {
// CHECK-NEXT:                            "type": "BinaryExpression",
// CHECK-NEXT:                            "left": {
// CHECK-NEXT:                              "type": "NumericLiteral",
// CHECK-NEXT:                              "value": 0.001660076642744037,
// CHECK-NEXT:                              "raw": "1.66007664274403694e-3"
// CHECK-NEXT:                            },
// CHECK-NEXT:                            "right": {
// CHECK-NEXT:                              "type": "Identifier",
// CHECK-NEXT:                              "name": "DAYS_PER_YEAR"
// CHECK-NEXT:                            },
// CHECK-NEXT:                            "operator": "*"
// CHECK-NEXT:                          },
// CHECK-NEXT:                          {
// CHECK-NEXT:                            "type": "BinaryExpression",
// CHECK-NEXT:                            "left": {
// CHECK-NEXT:                              "type": "NumericLiteral",
// CHECK-NEXT:                              "value": 0.007699011184197404,
// CHECK-NEXT:                              "raw": "7.69901118419740425e-3"
// CHECK-NEXT:                            },
// CHECK-NEXT:                            "right": {
// CHECK-NEXT:                              "type": "Identifier",
// CHECK-NEXT:                              "name": "DAYS_PER_YEAR"
// CHECK-NEXT:                            },
// CHECK-NEXT:                            "operator": "*"
// CHECK-NEXT:                          },
// CHECK-NEXT:                          {
// CHECK-NEXT:                            "type": "BinaryExpression",
// CHECK-NEXT:                            "left": {
// CHECK-NEXT:                              "type": "UnaryExpression",
// CHECK-NEXT:                              "operator": "-",
// CHECK-NEXT:                              "argument": {
// CHECK-NEXT:                                "type": "NumericLiteral",
// CHECK-NEXT:                                "value": 0.0000690460016972063,
// CHECK-NEXT:                                "raw": "6.90460016972063023e-5"
// CHECK-NEXT:                              },
// CHECK-NEXT:                              "prefix": true
// CHECK-NEXT:                            },
// CHECK-NEXT:                            "right": {
// CHECK-NEXT:                              "type": "Identifier",
// CHECK-NEXT:                              "name": "DAYS_PER_YEAR"
// CHECK-NEXT:                            },
// CHECK-NEXT:                            "operator": "*"
// CHECK-NEXT:                          },
// CHECK-NEXT:                          {
// CHECK-NEXT:                            "type": "BinaryExpression",
// CHECK-NEXT:                            "left": {
// CHECK-NEXT:                              "type": "NumericLiteral",
// CHECK-NEXT:                              "value": 0.0009547919384243266,
// CHECK-NEXT:                              "raw": "9.54791938424326609e-4"
// CHECK-NEXT:                            },
// CHECK-NEXT:                            "right": {
// CHECK-NEXT:                              "type": "Identifier",
// CHECK-NEXT:                              "name": "SOLAR_MASS"
// CHECK-NEXT:                            },
// CHECK-NEXT:                            "operator": "*"
// CHECK-NEXT:                          }
// CHECK-NEXT:                        ]
// CHECK-NEXT:                      }
// CHECK-NEXT:                    }
// CHECK-NEXT:                  ]
// CHECK-NEXT:                },
// CHECK-NEXT:                "returnType": {
// CHECK-NEXT:                  "type": "TypeAnnotation",
// CHECK-NEXT:                  "typeAnnotation": {
// CHECK-NEXT:                    "type": "GenericTypeAnnotation",
// CHECK-NEXT:                    "id": {
// CHECK-NEXT:                      "type": "Identifier",
// CHECK-NEXT:                      "name": "Body"
// CHECK-NEXT:                    },
// CHECK-NEXT:                    "typeParameters": null
// CHECK-NEXT:                  }
// CHECK-NEXT:                },
// CHECK-NEXT:                "generator": false,
// CHECK-NEXT:                "async": false
// CHECK-NEXT:              },
// CHECK-NEXT:              {
// CHECK-NEXT:                "type": "FunctionDeclaration",
// CHECK-NEXT:                "id": {
// CHECK-NEXT:                  "type": "Identifier",
// CHECK-NEXT:                  "name": "Saturn"
// CHECK-NEXT:                },
// CHECK-NEXT:                "params": [],
// CHECK-NEXT:                "body": {
// CHECK-NEXT:                  "type": "BlockStatement",
// CHECK-NEXT:                  "body": [
// CHECK-NEXT:                    {
// CHECK-NEXT:                      "type": "ReturnStatement",
// CHECK-NEXT:                      "argument": {
// CHECK-NEXT:                        "type": "NewExpression",
// CHECK-NEXT:                        "callee": {
// CHECK-NEXT:                          "type": "Identifier",
// CHECK-NEXT:                          "name": "Body"
// CHECK-NEXT:                        },
// CHECK-NEXT:                        "arguments": [
// CHECK-NEXT:                          {
// CHECK-NEXT:                            "type": "NumericLiteral",
// CHECK-NEXT:                            "value": 8.34336671824458,
// CHECK-NEXT:                            "raw": "8.34336671824457987"
// CHECK-NEXT:                          },
// CHECK-NEXT:                          {
// CHECK-NEXT:                            "type": "NumericLiteral",
// CHECK-NEXT:                            "value": 4.124798564124305,
// CHECK-NEXT:                            "raw": "4.12479856412430479"
// CHECK-NEXT:                          },
// CHECK-NEXT:                          {
// CHECK-NEXT:                            "type": "UnaryExpression",
// CHECK-NEXT:                            "operator": "-",
// CHECK-NEXT:                            "argument": {
// CHECK-NEXT:                              "type": "NumericLiteral",
// CHECK-NEXT:                              "value": 0.4035234171143214,
// CHECK-NEXT:                              "raw": "4.03523417114321381e-1"
// CHECK-NEXT:                            },
// CHECK-NEXT:                            "prefix": true
// CHECK-NEXT:                          },
// CHECK-NEXT:                          {
// CHECK-NEXT:                            "type": "BinaryExpression",
// CHECK-NEXT:                            "left": {
// CHECK-NEXT:                              "type": "UnaryExpression",
// CHECK-NEXT:                              "operator": "-",
// CHECK-NEXT:                              "argument": {
// CHECK-NEXT:                                "type": "NumericLiteral",
// CHECK-NEXT:                                "value": 0.002767425107268624,
// CHECK-NEXT:                                "raw": "2.76742510726862411e-3"
// CHECK-NEXT:                              },
// CHECK-NEXT:                              "prefix": true
// CHECK-NEXT:                            },
// CHECK-NEXT:                            "right": {
// CHECK-NEXT:                              "type": "Identifier",
// CHECK-NEXT:                              "name": "DAYS_PER_YEAR"
// CHECK-NEXT:                            },
// CHECK-NEXT:                            "operator": "*"
// CHECK-NEXT:                          },
// CHECK-NEXT:                          {
// CHECK-NEXT:                            "type": "BinaryExpression",
// CHECK-NEXT:                            "left": {
// CHECK-NEXT:                              "type": "NumericLiteral",
// CHECK-NEXT:                              "value": 0.004998528012349172,
// CHECK-NEXT:                              "raw": "4.99852801234917238e-3"
// CHECK-NEXT:                            },
// CHECK-NEXT:                            "right": {
// CHECK-NEXT:                              "type": "Identifier",
// CHECK-NEXT:                              "name": "DAYS_PER_YEAR"
// CHECK-NEXT:                            },
// CHECK-NEXT:                            "operator": "*"
// CHECK-NEXT:                          },
// CHECK-NEXT:                          {
// CHECK-NEXT:                            "type": "BinaryExpression",
// CHECK-NEXT:                            "left": {
// CHECK-NEXT:                              "type": "NumericLiteral",
// CHECK-NEXT:                              "value": 0.000023041729757376393,
// CHECK-NEXT:                              "raw": "2.30417297573763929e-5"
// CHECK-NEXT:                            },
// CHECK-NEXT:                            "right": {
// CHECK-NEXT:                              "type": "Identifier",
// CHECK-NEXT:                              "name": "DAYS_PER_YEAR"
// CHECK-NEXT:                            },
// CHECK-NEXT:                            "operator": "*"
// CHECK-NEXT:                          },
// CHECK-NEXT:                          {
// CHECK-NEXT:                            "type": "BinaryExpression",
// CHECK-NEXT:                            "left": {
// CHECK-NEXT:                              "type": "NumericLiteral",
// CHECK-NEXT:                              "value": 0.0002858859806661308,
// CHECK-NEXT:                              "raw": "2.85885980666130812e-4"
// CHECK-NEXT:                            },
// CHECK-NEXT:                            "right": {
// CHECK-NEXT:                              "type": "Identifier",
// CHECK-NEXT:                              "name": "SOLAR_MASS"
// CHECK-NEXT:                            },
// CHECK-NEXT:                            "operator": "*"
// CHECK-NEXT:                          }
// CHECK-NEXT:                        ]
// CHECK-NEXT:                      }
// CHECK-NEXT:                    }
// CHECK-NEXT:                  ]
// CHECK-NEXT:                },
// CHECK-NEXT:                "returnType": {
// CHECK-NEXT:                  "type": "TypeAnnotation",
// CHECK-NEXT:                  "typeAnnotation": {
// CHECK-NEXT:                    "type": "GenericTypeAnnotation",
// CHECK-NEXT:                    "id": {
// CHECK-NEXT:                      "type": "Identifier",
// CHECK-NEXT:                      "name": "Body"
// CHECK-NEXT:                    },
// CHECK-NEXT:                    "typeParameters": null
// CHECK-NEXT:                  }
// CHECK-NEXT:                },
// CHECK-NEXT:                "generator": false,
// CHECK-NEXT:                "async": false
// CHECK-NEXT:              },
// CHECK-NEXT:              {
// CHECK-NEXT:                "type": "FunctionDeclaration",
// CHECK-NEXT:                "id": {
// CHECK-NEXT:                  "type": "Identifier",
// CHECK-NEXT:                  "name": "Uranus"
// CHECK-NEXT:                },
// CHECK-NEXT:                "params": [],
// CHECK-NEXT:                "body": {
// CHECK-NEXT:                  "type": "BlockStatement",
// CHECK-NEXT:                  "body": [
// CHECK-NEXT:                    {
// CHECK-NEXT:                      "type": "ReturnStatement",
// CHECK-NEXT:                      "argument": {
// CHECK-NEXT:                        "type": "NewExpression",
// CHECK-NEXT:                        "callee": {
// CHECK-NEXT:                          "type": "Identifier",
// CHECK-NEXT:                          "name": "Body"
// CHECK-NEXT:                        },
// CHECK-NEXT:                        "arguments": [
// CHECK-NEXT:                          {
// CHECK-NEXT:                            "type": "NumericLiteral",
// CHECK-NEXT:                            "value": 12.894369562139131,
// CHECK-NEXT:                            "raw": "1.2894369562139131e1"
// CHECK-NEXT:                          },
// CHECK-NEXT:                          {
// CHECK-NEXT:                            "type": "UnaryExpression",
// CHECK-NEXT:                            "operator": "-",
// CHECK-NEXT:                            "argument": {
// CHECK-NEXT:                              "type": "NumericLiteral",
// CHECK-NEXT:                              "value": 15.111151401698631,
// CHECK-NEXT:                              "raw": "1.51111514016986312e1"
// CHECK-NEXT:                            },
// CHECK-NEXT:                            "prefix": true
// CHECK-NEXT:                          },
// CHECK-NEXT:                          {
// CHECK-NEXT:                            "type": "UnaryExpression",
// CHECK-NEXT:                            "operator": "-",
// CHECK-NEXT:                            "argument": {
// CHECK-NEXT:                              "type": "NumericLiteral",
// CHECK-NEXT:                              "value": 0.22330757889265573,
// CHECK-NEXT:                              "raw": "2.23307578892655734e-1"
// CHECK-NEXT:                            },
// CHECK-NEXT:                            "prefix": true
// CHECK-NEXT:                          },
// CHECK-NEXT:                          {
// CHECK-NEXT:                            "type": "BinaryExpression",
// CHECK-NEXT:                            "left": {
// CHECK-NEXT:                              "type": "NumericLiteral",
// CHECK-NEXT:                              "value": 0.002964601375647616,
// CHECK-NEXT:                              "raw": "2.96460137564761618e-3"
// CHECK-NEXT:                            },
// CHECK-NEXT:                            "right": {
// CHECK-NEXT:                              "type": "Identifier",
// CHECK-NEXT:                              "name": "DAYS_PER_YEAR"
// CHECK-NEXT:                            },
// CHECK-NEXT:                            "operator": "*"
// CHECK-NEXT:                          },
// CHECK-NEXT:                          {
// CHECK-NEXT:                            "type": "BinaryExpression",
// CHECK-NEXT:                            "left": {
// CHECK-NEXT:                              "type": "NumericLiteral",
// CHECK-NEXT:                              "value": 0.0023784717395948095,
// CHECK-NEXT:                              "raw": "2.3784717395948095e-3"
// CHECK-NEXT:                            },
// CHECK-NEXT:                            "right": {
// CHECK-NEXT:                              "type": "Identifier",
// CHECK-NEXT:                              "name": "DAYS_PER_YEAR"
// CHECK-NEXT:                            },
// CHECK-NEXT:                            "operator": "*"
// CHECK-NEXT:                          },
// CHECK-NEXT:                          {
// CHECK-NEXT:                            "type": "BinaryExpression",
// CHECK-NEXT:                            "left": {
// CHECK-NEXT:                              "type": "UnaryExpression",
// CHECK-NEXT:                              "operator": "-",
// CHECK-NEXT:                              "argument": {
// CHECK-NEXT:                                "type": "NumericLiteral",
// CHECK-NEXT:                                "value": 0.000029658956854023756,
// CHECK-NEXT:                                "raw": "2.96589568540237556e-5"
// CHECK-NEXT:                              },
// CHECK-NEXT:                              "prefix": true
// CHECK-NEXT:                            },
// CHECK-NEXT:                            "right": {
// CHECK-NEXT:                              "type": "Identifier",
// CHECK-NEXT:                              "name": "DAYS_PER_YEAR"
// CHECK-NEXT:                            },
// CHECK-NEXT:                            "operator": "*"
// CHECK-NEXT:                          },
// CHECK-NEXT:                          {
// CHECK-NEXT:                            "type": "BinaryExpression",
// CHECK-NEXT:                            "left": {
// CHECK-NEXT:                              "type": "NumericLiteral",
// CHECK-NEXT:                              "value": 0.00004366244043351563,
// CHECK-NEXT:                              "raw": "4.36624404335156298e-5"
// CHECK-NEXT:                            },
// CHECK-NEXT:                            "right": {
// CHECK-NEXT:                              "type": "Identifier",
// CHECK-NEXT:                              "name": "SOLAR_MASS"
// CHECK-NEXT:                            },
// CHECK-NEXT:                            "operator": "*"
// CHECK-NEXT:                          }
// CHECK-NEXT:                        ]
// CHECK-NEXT:                      }
// CHECK-NEXT:                    }
// CHECK-NEXT:                  ]
// CHECK-NEXT:                },
// CHECK-NEXT:                "returnType": {
// CHECK-NEXT:                  "type": "TypeAnnotation",
// CHECK-NEXT:                  "typeAnnotation": {
// CHECK-NEXT:                    "type": "GenericTypeAnnotation",
// CHECK-NEXT:                    "id": {
// CHECK-NEXT:                      "type": "Identifier",
// CHECK-NEXT:                      "name": "Body"
// CHECK-NEXT:                    },
// CHECK-NEXT:                    "typeParameters": null
// CHECK-NEXT:                  }
// CHECK-NEXT:                },
// CHECK-NEXT:                "generator": false,
// CHECK-NEXT:                "async": false
// CHECK-NEXT:              },
// CHECK-NEXT:              {
// CHECK-NEXT:                "type": "FunctionDeclaration",
// CHECK-NEXT:                "id": {
// CHECK-NEXT:                  "type": "Identifier",
// CHECK-NEXT:                  "name": "Neptune"
// CHECK-NEXT:                },
// CHECK-NEXT:                "params": [],
// CHECK-NEXT:                "body": {
// CHECK-NEXT:                  "type": "BlockStatement",
// CHECK-NEXT:                  "body": [
// CHECK-NEXT:                    {
// CHECK-NEXT:                      "type": "ReturnStatement",
// CHECK-NEXT:                      "argument": {
// CHECK-NEXT:                        "type": "NewExpression",
// CHECK-NEXT:                        "callee": {
// CHECK-NEXT:                          "type": "Identifier",
// CHECK-NEXT:                          "name": "Body"
// CHECK-NEXT:                        },
// CHECK-NEXT:                        "arguments": [
// CHECK-NEXT:                          {
// CHECK-NEXT:                            "type": "NumericLiteral",
// CHECK-NEXT:                            "value": 15.379697114850917,
// CHECK-NEXT:                            "raw": "1.53796971148509165e1"
// CHECK-NEXT:                          },
// CHECK-NEXT:                          {
// CHECK-NEXT:                            "type": "UnaryExpression",
// CHECK-NEXT:                            "operator": "-",
// CHECK-NEXT:                            "argument": {
// CHECK-NEXT:                              "type": "NumericLiteral",
// CHECK-NEXT:                              "value": 25.919314609987964,
// CHECK-NEXT:                              "raw": "2.59193146099879641e1"
// CHECK-NEXT:                            },
// CHECK-NEXT:                            "prefix": true
// CHECK-NEXT:                          },
// CHECK-NEXT:                          {
// CHECK-NEXT:                            "type": "NumericLiteral",
// CHECK-NEXT:                            "value": 0.17925877295037118,
// CHECK-NEXT:                            "raw": "1.79258772950371181e-1"
// CHECK-NEXT:                          },
// CHECK-NEXT:                          {
// CHECK-NEXT:                            "type": "BinaryExpression",
// CHECK-NEXT:                            "left": {
// CHECK-NEXT:                              "type": "NumericLiteral",
// CHECK-NEXT:                              "value": 0.0026806777249038932,
// CHECK-NEXT:                              "raw": "2.68067772490389322e-3"
// CHECK-NEXT:                            },
// CHECK-NEXT:                            "right": {
// CHECK-NEXT:                              "type": "Identifier",
// CHECK-NEXT:                              "name": "DAYS_PER_YEAR"
// CHECK-NEXT:                            },
// CHECK-NEXT:                            "operator": "*"
// CHECK-NEXT:                          },
// CHECK-NEXT:                          {
// CHECK-NEXT:                            "type": "BinaryExpression",
// CHECK-NEXT:                            "left": {
// CHECK-NEXT:                              "type": "NumericLiteral",
// CHECK-NEXT:                              "value": 0.001628241700382423,
// CHECK-NEXT:                              "raw": "1.62824170038242295e-3"
// CHECK-NEXT:                            },
// CHECK-NEXT:                            "right": {
// CHECK-NEXT:                              "type": "Identifier",
// CHECK-NEXT:                              "name": "DAYS_PER_YEAR"
// CHECK-NEXT:                            },
// CHECK-NEXT:                            "operator": "*"
// CHECK-NEXT:                          },
// CHECK-NEXT:                          {
// CHECK-NEXT:                            "type": "BinaryExpression",
// CHECK-NEXT:                            "left": {
// CHECK-NEXT:                              "type": "UnaryExpression",
// CHECK-NEXT:                              "operator": "-",
// CHECK-NEXT:                              "argument": {
// CHECK-NEXT:                                "type": "NumericLiteral",
// CHECK-NEXT:                                "value": 0.00009515922545197159,
// CHECK-NEXT:                                "raw": "9.5159225451971587e-5"
// CHECK-NEXT:                              },
// CHECK-NEXT:                              "prefix": true
// CHECK-NEXT:                            },
// CHECK-NEXT:                            "right": {
// CHECK-NEXT:                              "type": "Identifier",
// CHECK-NEXT:                              "name": "DAYS_PER_YEAR"
// CHECK-NEXT:                            },
// CHECK-NEXT:                            "operator": "*"
// CHECK-NEXT:                          },
// CHECK-NEXT:                          {
// CHECK-NEXT:                            "type": "BinaryExpression",
// CHECK-NEXT:                            "left": {
// CHECK-NEXT:                              "type": "NumericLiteral",
// CHECK-NEXT:                              "value": 0.000051513890204661145,
// CHECK-NEXT:                              "raw": "5.15138902046611451e-5"
// CHECK-NEXT:                            },
// CHECK-NEXT:                            "right": {
// CHECK-NEXT:                              "type": "Identifier",
// CHECK-NEXT:                              "name": "SOLAR_MASS"
// CHECK-NEXT:                            },
// CHECK-NEXT:                            "operator": "*"
// CHECK-NEXT:                          }
// CHECK-NEXT:                        ]
// CHECK-NEXT:                      }
// CHECK-NEXT:                    }
// CHECK-NEXT:                  ]
// CHECK-NEXT:                },
// CHECK-NEXT:                "returnType": {
// CHECK-NEXT:                  "type": "TypeAnnotation",
// CHECK-NEXT:                  "typeAnnotation": {
// CHECK-NEXT:                    "type": "GenericTypeAnnotation",
// CHECK-NEXT:                    "id": {
// CHECK-NEXT:                      "type": "Identifier",
// CHECK-NEXT:                      "name": "Body"
// CHECK-NEXT:                    },
// CHECK-NEXT:                    "typeParameters": null
// CHECK-NEXT:                  }
// CHECK-NEXT:                },
// CHECK-NEXT:                "generator": false,
// CHECK-NEXT:                "async": false
// CHECK-NEXT:              },
// CHECK-NEXT:              {
// CHECK-NEXT:                "type": "FunctionDeclaration",
// CHECK-NEXT:                "id": {
// CHECK-NEXT:                  "type": "Identifier",
// CHECK-NEXT:                  "name": "Sun"
// CHECK-NEXT:                },
// CHECK-NEXT:                "params": [],
// CHECK-NEXT:                "body": {
// CHECK-NEXT:                  "type": "BlockStatement",
// CHECK-NEXT:                  "body": [
// CHECK-NEXT:                    {
// CHECK-NEXT:                      "type": "ReturnStatement",
// CHECK-NEXT:                      "argument": {
// CHECK-NEXT:                        "type": "NewExpression",
// CHECK-NEXT:                        "callee": {
// CHECK-NEXT:                          "type": "Identifier",
// CHECK-NEXT:                          "name": "Body"
// CHECK-NEXT:                        },
// CHECK-NEXT:                        "arguments": [
// CHECK-NEXT:                          {
// CHECK-NEXT:                            "type": "NumericLiteral",
// CHECK-NEXT:                            "value": 0,
// CHECK-NEXT:                            "raw": "0.0"
// CHECK-NEXT:                          },
// CHECK-NEXT:                          {
// CHECK-NEXT:                            "type": "NumericLiteral",
// CHECK-NEXT:                            "value": 0,
// CHECK-NEXT:                            "raw": "0.0"
// CHECK-NEXT:                          },
// CHECK-NEXT:                          {
// CHECK-NEXT:                            "type": "NumericLiteral",
// CHECK-NEXT:                            "value": 0,
// CHECK-NEXT:                            "raw": "0.0"
// CHECK-NEXT:                          },
// CHECK-NEXT:                          {
// CHECK-NEXT:                            "type": "NumericLiteral",
// CHECK-NEXT:                            "value": 0,
// CHECK-NEXT:                            "raw": "0.0"
// CHECK-NEXT:                          },
// CHECK-NEXT:                          {
// CHECK-NEXT:                            "type": "NumericLiteral",
// CHECK-NEXT:                            "value": 0,
// CHECK-NEXT:                            "raw": "0.0"
// CHECK-NEXT:                          },
// CHECK-NEXT:                          {
// CHECK-NEXT:                            "type": "NumericLiteral",
// CHECK-NEXT:                            "value": 0,
// CHECK-NEXT:                            "raw": "0.0"
// CHECK-NEXT:                          },
// CHECK-NEXT:                          {
// CHECK-NEXT:                            "type": "Identifier",
// CHECK-NEXT:                            "name": "SOLAR_MASS"
// CHECK-NEXT:                          }
// CHECK-NEXT:                        ]
// CHECK-NEXT:                      }
// CHECK-NEXT:                    }
// CHECK-NEXT:                  ]
// CHECK-NEXT:                },
// CHECK-NEXT:                "returnType": {
// CHECK-NEXT:                  "type": "TypeAnnotation",
// CHECK-NEXT:                  "typeAnnotation": {
// CHECK-NEXT:                    "type": "GenericTypeAnnotation",
// CHECK-NEXT:                    "id": {
// CHECK-NEXT:                      "type": "Identifier",
// CHECK-NEXT:                      "name": "Body"
// CHECK-NEXT:                    },
// CHECK-NEXT:                    "typeParameters": null
// CHECK-NEXT:                  }
// CHECK-NEXT:                },
// CHECK-NEXT:                "generator": false,
// CHECK-NEXT:                "async": false
// CHECK-NEXT:              },
// CHECK-NEXT:              {
// CHECK-NEXT:                "type": "VariableDeclaration",
// CHECK-NEXT:                "kind": "const",
// CHECK-NEXT:                "declarations": [
// CHECK-NEXT:                  {
// CHECK-NEXT:                    "type": "VariableDeclarator",
// CHECK-NEXT:                    "init": {
// CHECK-NEXT:                      "type": "ArrayExpression",
// CHECK-NEXT:                      "elements": [
// CHECK-NEXT:                        {
// CHECK-NEXT:                          "type": "CallExpression",
// CHECK-NEXT:                          "callee": {
// CHECK-NEXT:                            "type": "Identifier",
// CHECK-NEXT:                            "name": "Sun"
// CHECK-NEXT:                          },
// CHECK-NEXT:                          "arguments": []
// CHECK-NEXT:                        },
// CHECK-NEXT:                        {
// CHECK-NEXT:                          "type": "CallExpression",
// CHECK-NEXT:                          "callee": {
// CHECK-NEXT:                            "type": "Identifier",
// CHECK-NEXT:                            "name": "Jupiter"
// CHECK-NEXT:                          },
// CHECK-NEXT:                          "arguments": []
// CHECK-NEXT:                        },
// CHECK-NEXT:                        {
// CHECK-NEXT:                          "type": "CallExpression",
// CHECK-NEXT:                          "callee": {
// CHECK-NEXT:                            "type": "Identifier",
// CHECK-NEXT:                            "name": "Saturn"
// CHECK-NEXT:                          },
// CHECK-NEXT:                          "arguments": []
// CHECK-NEXT:                        },
// CHECK-NEXT:                        {
// CHECK-NEXT:                          "type": "CallExpression",
// CHECK-NEXT:                          "callee": {
// CHECK-NEXT:                            "type": "Identifier",
// CHECK-NEXT:                            "name": "Uranus"
// CHECK-NEXT:                          },
// CHECK-NEXT:                          "arguments": []
// CHECK-NEXT:                        },
// CHECK-NEXT:                        {
// CHECK-NEXT:                          "type": "CallExpression",
// CHECK-NEXT:                          "callee": {
// CHECK-NEXT:                            "type": "Identifier",
// CHECK-NEXT:                            "name": "Neptune"
// CHECK-NEXT:                          },
// CHECK-NEXT:                          "arguments": []
// CHECK-NEXT:                        }
// CHECK-NEXT:                      ],
// CHECK-NEXT:                      "trailingComma": false
// CHECK-NEXT:                    },
// CHECK-NEXT:                    "id": {
// CHECK-NEXT:                      "type": "Identifier",
// CHECK-NEXT:                      "name": "bodies",
// CHECK-NEXT:                      "typeAnnotation": {
// CHECK-NEXT:                        "type": "TypeAnnotation",
// CHECK-NEXT:                        "typeAnnotation": {
// CHECK-NEXT:                          "type": "ArrayTypeAnnotation",
// CHECK-NEXT:                          "elementType": {
// CHECK-NEXT:                            "type": "GenericTypeAnnotation",
// CHECK-NEXT:                            "id": {
// CHECK-NEXT:                              "type": "Identifier",
// CHECK-NEXT:                              "name": "Body"
// CHECK-NEXT:                            },
// CHECK-NEXT:                            "typeParameters": null
// CHECK-NEXT:                          }
// CHECK-NEXT:                        }
// CHECK-NEXT:                      }
// CHECK-NEXT:                    }
// CHECK-NEXT:                  }
// CHECK-NEXT:                ]
// CHECK-NEXT:              },
// CHECK-NEXT:              {
// CHECK-NEXT:                "type": "FunctionDeclaration",
// CHECK-NEXT:                "id": {
// CHECK-NEXT:                  "type": "Identifier",
// CHECK-NEXT:                  "name": "offsetMomentum"
// CHECK-NEXT:                },
// CHECK-NEXT:                "params": [],
// CHECK-NEXT:                "body": {
// CHECK-NEXT:                  "type": "BlockStatement",
// CHECK-NEXT:                  "body": [
// CHECK-NEXT:                    {
// CHECK-NEXT:                      "type": "VariableDeclaration",
// CHECK-NEXT:                      "kind": "let",
// CHECK-NEXT:                      "declarations": [
// CHECK-NEXT:                        {
// CHECK-NEXT:                          "type": "VariableDeclarator",
// CHECK-NEXT:                          "init": {
// CHECK-NEXT:                            "type": "NumericLiteral",
// CHECK-NEXT:                            "value": 0,
// CHECK-NEXT:                            "raw": "0"
// CHECK-NEXT:                          },
// CHECK-NEXT:                          "id": {
// CHECK-NEXT:                            "type": "Identifier",
// CHECK-NEXT:                            "name": "px"
// CHECK-NEXT:                          }
// CHECK-NEXT:                        }
// CHECK-NEXT:                      ]
// CHECK-NEXT:                    },
// CHECK-NEXT:                    {
// CHECK-NEXT:                      "type": "VariableDeclaration",
// CHECK-NEXT:                      "kind": "let",
// CHECK-NEXT:                      "declarations": [
// CHECK-NEXT:                        {
// CHECK-NEXT:                          "type": "VariableDeclarator",
// CHECK-NEXT:                          "init": {
// CHECK-NEXT:                            "type": "NumericLiteral",
// CHECK-NEXT:                            "value": 0,
// CHECK-NEXT:                            "raw": "0"
// CHECK-NEXT:                          },
// CHECK-NEXT:                          "id": {
// CHECK-NEXT:                            "type": "Identifier",
// CHECK-NEXT:                            "name": "py"
// CHECK-NEXT:                          }
// CHECK-NEXT:                        }
// CHECK-NEXT:                      ]
// CHECK-NEXT:                    },
// CHECK-NEXT:                    {
// CHECK-NEXT:                      "type": "VariableDeclaration",
// CHECK-NEXT:                      "kind": "let",
// CHECK-NEXT:                      "declarations": [
// CHECK-NEXT:                        {
// CHECK-NEXT:                          "type": "VariableDeclarator",
// CHECK-NEXT:                          "init": {
// CHECK-NEXT:                            "type": "NumericLiteral",
// CHECK-NEXT:                            "value": 0,
// CHECK-NEXT:                            "raw": "0"
// CHECK-NEXT:                          },
// CHECK-NEXT:                          "id": {
// CHECK-NEXT:                            "type": "Identifier",
// CHECK-NEXT:                            "name": "pz"
// CHECK-NEXT:                          }
// CHECK-NEXT:                        }
// CHECK-NEXT:                      ]
// CHECK-NEXT:                    },
// CHECK-NEXT:                    {
// CHECK-NEXT:                      "type": "VariableDeclaration",
// CHECK-NEXT:                      "kind": "const",
// CHECK-NEXT:                      "declarations": [
// CHECK-NEXT:                        {
// CHECK-NEXT:                          "type": "VariableDeclarator",
// CHECK-NEXT:                          "init": {
// CHECK-NEXT:                            "type": "MemberExpression",
// CHECK-NEXT:                            "object": {
// CHECK-NEXT:                              "type": "Identifier",
// CHECK-NEXT:                              "name": "bodies"
// CHECK-NEXT:                            },
// CHECK-NEXT:                            "property": {
// CHECK-NEXT:                              "type": "Identifier",
// CHECK-NEXT:                              "name": "length"
// CHECK-NEXT:                            },
// CHECK-NEXT:                            "computed": false
// CHECK-NEXT:                          },
// CHECK-NEXT:                          "id": {
// CHECK-NEXT:                            "type": "Identifier",
// CHECK-NEXT:                            "name": "size",
// CHECK-NEXT:                            "typeAnnotation": {
// CHECK-NEXT:                              "type": "TypeAnnotation",
// CHECK-NEXT:                              "typeAnnotation": {
// CHECK-NEXT:                                "type": "NumberTypeAnnotation"
// CHECK-NEXT:                              }
// CHECK-NEXT:                            }
// CHECK-NEXT:                          }
// CHECK-NEXT:                        }
// CHECK-NEXT:                      ]
// CHECK-NEXT:                    },
// CHECK-NEXT:                    {
// CHECK-NEXT:                      "type": "ForStatement",
// CHECK-NEXT:                      "init": {
// CHECK-NEXT:                        "type": "VariableDeclaration",
// CHECK-NEXT:                        "kind": "let",
// CHECK-NEXT:                        "declarations": [
// CHECK-NEXT:                          {
// CHECK-NEXT:                            "type": "VariableDeclarator",
// CHECK-NEXT:                            "init": {
// CHECK-NEXT:                              "type": "NumericLiteral",
// CHECK-NEXT:                              "value": 0,
// CHECK-NEXT:                              "raw": "0"
// CHECK-NEXT:                            },
// CHECK-NEXT:                            "id": {
// CHECK-NEXT:                              "type": "Identifier",
// CHECK-NEXT:                              "name": "i"
// CHECK-NEXT:                            }
// CHECK-NEXT:                          }
// CHECK-NEXT:                        ]
// CHECK-NEXT:                      },
// CHECK-NEXT:                      "test": {
// CHECK-NEXT:                        "type": "BinaryExpression",
// CHECK-NEXT:                        "left": {
// CHECK-NEXT:                          "type": "Identifier",
// CHECK-NEXT:                          "name": "i"
// CHECK-NEXT:                        },
// CHECK-NEXT:                        "right": {
// CHECK-NEXT:                          "type": "Identifier",
// CHECK-NEXT:                          "name": "size"
// CHECK-NEXT:                        },
// CHECK-NEXT:                        "operator": "<"
// CHECK-NEXT:                      },
// CHECK-NEXT:                      "update": {
// CHECK-NEXT:                        "type": "UpdateExpression",
// CHECK-NEXT:                        "operator": "++",
// CHECK-NEXT:                        "argument": {
// CHECK-NEXT:                          "type": "Identifier",
// CHECK-NEXT:                          "name": "i"
// CHECK-NEXT:                        },
// CHECK-NEXT:                        "prefix": false
// CHECK-NEXT:                      },
// CHECK-NEXT:                      "body": {
// CHECK-NEXT:                        "type": "BlockStatement",
// CHECK-NEXT:                        "body": [
// CHECK-NEXT:                          {
// CHECK-NEXT:                            "type": "VariableDeclaration",
// CHECK-NEXT:                            "kind": "const",
// CHECK-NEXT:                            "declarations": [
// CHECK-NEXT:                              {
// CHECK-NEXT:                                "type": "VariableDeclarator",
// CHECK-NEXT:                                "init": {
// CHECK-NEXT:                                  "type": "MemberExpression",
// CHECK-NEXT:                                  "object": {
// CHECK-NEXT:                                    "type": "Identifier",
// CHECK-NEXT:                                    "name": "bodies"
// CHECK-NEXT:                                  },
// CHECK-NEXT:                                  "property": {
// CHECK-NEXT:                                    "type": "Identifier",
// CHECK-NEXT:                                    "name": "i"
// CHECK-NEXT:                                  },
// CHECK-NEXT:                                  "computed": true
// CHECK-NEXT:                                },
// CHECK-NEXT:                                "id": {
// CHECK-NEXT:                                  "type": "Identifier",
// CHECK-NEXT:                                  "name": "body",
// CHECK-NEXT:                                  "typeAnnotation": {
// CHECK-NEXT:                                    "type": "TypeAnnotation",
// CHECK-NEXT:                                    "typeAnnotation": {
// CHECK-NEXT:                                      "type": "GenericTypeAnnotation",
// CHECK-NEXT:                                      "id": {
// CHECK-NEXT:                                        "type": "Identifier",
// CHECK-NEXT:                                        "name": "Body"
// CHECK-NEXT:                                      },
// CHECK-NEXT:                                      "typeParameters": null
// CHECK-NEXT:                                    }
// CHECK-NEXT:                                  }
// CHECK-NEXT:                                }
// CHECK-NEXT:                              }
// CHECK-NEXT:                            ]
// CHECK-NEXT:                          },
// CHECK-NEXT:                          {
// CHECK-NEXT:                            "type": "VariableDeclaration",
// CHECK-NEXT:                            "kind": "const",
// CHECK-NEXT:                            "declarations": [
// CHECK-NEXT:                              {
// CHECK-NEXT:                                "type": "VariableDeclarator",
// CHECK-NEXT:                                "init": {
// CHECK-NEXT:                                  "type": "MemberExpression",
// CHECK-NEXT:                                  "object": {
// CHECK-NEXT:                                    "type": "Identifier",
// CHECK-NEXT:                                    "name": "body"
// CHECK-NEXT:                                  },
// CHECK-NEXT:                                  "property": {
// CHECK-NEXT:                                    "type": "Identifier",
// CHECK-NEXT:                                    "name": "mass"
// CHECK-NEXT:                                  },
// CHECK-NEXT:                                  "computed": false
// CHECK-NEXT:                                },
// CHECK-NEXT:                                "id": {
// CHECK-NEXT:                                  "type": "Identifier",
// CHECK-NEXT:                                  "name": "mass",
// CHECK-NEXT:                                  "typeAnnotation": {
// CHECK-NEXT:                                    "type": "TypeAnnotation",
// CHECK-NEXT:                                    "typeAnnotation": {
// CHECK-NEXT:                                      "type": "NumberTypeAnnotation"
// CHECK-NEXT:                                    }
// CHECK-NEXT:                                  }
// CHECK-NEXT:                                }
// CHECK-NEXT:                              }
// CHECK-NEXT:                            ]
// CHECK-NEXT:                          },
// CHECK-NEXT:                          {
// CHECK-NEXT:                            "type": "ExpressionStatement",
// CHECK-NEXT:                            "expression": {
// CHECK-NEXT:                              "type": "AssignmentExpression",
// CHECK-NEXT:                              "operator": "+=",
// CHECK-NEXT:                              "left": {
// CHECK-NEXT:                                "type": "Identifier",
// CHECK-NEXT:                                "name": "px"
// CHECK-NEXT:                              },
// CHECK-NEXT:                              "right": {
// CHECK-NEXT:                                "type": "BinaryExpression",
// CHECK-NEXT:                                "left": {
// CHECK-NEXT:                                  "type": "MemberExpression",
// CHECK-NEXT:                                  "object": {
// CHECK-NEXT:                                    "type": "Identifier",
// CHECK-NEXT:                                    "name": "body"
// CHECK-NEXT:                                  },
// CHECK-NEXT:                                  "property": {
// CHECK-NEXT:                                    "type": "Identifier",
// CHECK-NEXT:                                    "name": "vx"
// CHECK-NEXT:                                  },
// CHECK-NEXT:                                  "computed": false
// CHECK-NEXT:                                },
// CHECK-NEXT:                                "right": {
// CHECK-NEXT:                                  "type": "Identifier",
// CHECK-NEXT:                                  "name": "mass"
// CHECK-NEXT:                                },
// CHECK-NEXT:                                "operator": "*"
// CHECK-NEXT:                              }
// CHECK-NEXT:                            },
// CHECK-NEXT:                            "directive": null
// CHECK-NEXT:                          },
// CHECK-NEXT:                          {
// CHECK-NEXT:                            "type": "ExpressionStatement",
// CHECK-NEXT:                            "expression": {
// CHECK-NEXT:                              "type": "AssignmentExpression",
// CHECK-NEXT:                              "operator": "+=",
// CHECK-NEXT:                              "left": {
// CHECK-NEXT:                                "type": "Identifier",
// CHECK-NEXT:                                "name": "py"
// CHECK-NEXT:                              },
// CHECK-NEXT:                              "right": {
// CHECK-NEXT:                                "type": "BinaryExpression",
// CHECK-NEXT:                                "left": {
// CHECK-NEXT:                                  "type": "MemberExpression",
// CHECK-NEXT:                                  "object": {
// CHECK-NEXT:                                    "type": "Identifier",
// CHECK-NEXT:                                    "name": "body"
// CHECK-NEXT:                                  },
// CHECK-NEXT:                                  "property": {
// CHECK-NEXT:                                    "type": "Identifier",
// CHECK-NEXT:                                    "name": "vy"
// CHECK-NEXT:                                  },
// CHECK-NEXT:                                  "computed": false
// CHECK-NEXT:                                },
// CHECK-NEXT:                                "right": {
// CHECK-NEXT:                                  "type": "Identifier",
// CHECK-NEXT:                                  "name": "mass"
// CHECK-NEXT:                                },
// CHECK-NEXT:                                "operator": "*"
// CHECK-NEXT:                              }
// CHECK-NEXT:                            },
// CHECK-NEXT:                            "directive": null
// CHECK-NEXT:                          },
// CHECK-NEXT:                          {
// CHECK-NEXT:                            "type": "ExpressionStatement",
// CHECK-NEXT:                            "expression": {
// CHECK-NEXT:                              "type": "AssignmentExpression",
// CHECK-NEXT:                              "operator": "+=",
// CHECK-NEXT:                              "left": {
// CHECK-NEXT:                                "type": "Identifier",
// CHECK-NEXT:                                "name": "pz"
// CHECK-NEXT:                              },
// CHECK-NEXT:                              "right": {
// CHECK-NEXT:                                "type": "BinaryExpression",
// CHECK-NEXT:                                "left": {
// CHECK-NEXT:                                  "type": "MemberExpression",
// CHECK-NEXT:                                  "object": {
// CHECK-NEXT:                                    "type": "Identifier",
// CHECK-NEXT:                                    "name": "body"
// CHECK-NEXT:                                  },
// CHECK-NEXT:                                  "property": {
// CHECK-NEXT:                                    "type": "Identifier",
// CHECK-NEXT:                                    "name": "vz"
// CHECK-NEXT:                                  },
// CHECK-NEXT:                                  "computed": false
// CHECK-NEXT:                                },
// CHECK-NEXT:                                "right": {
// CHECK-NEXT:                                  "type": "Identifier",
// CHECK-NEXT:                                  "name": "mass"
// CHECK-NEXT:                                },
// CHECK-NEXT:                                "operator": "*"
// CHECK-NEXT:                              }
// CHECK-NEXT:                            },
// CHECK-NEXT:                            "directive": null
// CHECK-NEXT:                          }
// CHECK-NEXT:                        ]
// CHECK-NEXT:                      }
// CHECK-NEXT:                    },
// CHECK-NEXT:                    {
// CHECK-NEXT:                      "type": "VariableDeclaration",
// CHECK-NEXT:                      "kind": "const",
// CHECK-NEXT:                      "declarations": [
// CHECK-NEXT:                        {
// CHECK-NEXT:                          "type": "VariableDeclarator",
// CHECK-NEXT:                          "init": {
// CHECK-NEXT:                            "type": "MemberExpression",
// CHECK-NEXT:                            "object": {
// CHECK-NEXT:                              "type": "Identifier",
// CHECK-NEXT:                              "name": "bodies"
// CHECK-NEXT:                            },
// CHECK-NEXT:                            "property": {
// CHECK-NEXT:                              "type": "NumericLiteral",
// CHECK-NEXT:                              "value": 0,
// CHECK-NEXT:                              "raw": "0"
// CHECK-NEXT:                            },
// CHECK-NEXT:                            "computed": true
// CHECK-NEXT:                          },
// CHECK-NEXT:                          "id": {
// CHECK-NEXT:                            "type": "Identifier",
// CHECK-NEXT:                            "name": "body",
// CHECK-NEXT:                            "typeAnnotation": {
// CHECK-NEXT:                              "type": "TypeAnnotation",
// CHECK-NEXT:                              "typeAnnotation": {
// CHECK-NEXT:                                "type": "GenericTypeAnnotation",
// CHECK-NEXT:                                "id": {
// CHECK-NEXT:                                  "type": "Identifier",
// CHECK-NEXT:                                  "name": "Body"
// CHECK-NEXT:                                },
// CHECK-NEXT:                                "typeParameters": null
// CHECK-NEXT:                              }
// CHECK-NEXT:                            }
// CHECK-NEXT:                          }
// CHECK-NEXT:                        }
// CHECK-NEXT:                      ]
// CHECK-NEXT:                    },
// CHECK-NEXT:                    {
// CHECK-NEXT:                      "type": "ExpressionStatement",
// CHECK-NEXT:                      "expression": {
// CHECK-NEXT:                        "type": "AssignmentExpression",
// CHECK-NEXT:                        "operator": "=",
// CHECK-NEXT:                        "left": {
// CHECK-NEXT:                          "type": "MemberExpression",
// CHECK-NEXT:                          "object": {
// CHECK-NEXT:                            "type": "Identifier",
// CHECK-NEXT:                            "name": "body"
// CHECK-NEXT:                          },
// CHECK-NEXT:                          "property": {
// CHECK-NEXT:                            "type": "Identifier",
// CHECK-NEXT:                            "name": "vx"
// CHECK-NEXT:                          },
// CHECK-NEXT:                          "computed": false
// CHECK-NEXT:                        },
// CHECK-NEXT:                        "right": {
// CHECK-NEXT:                          "type": "BinaryExpression",
// CHECK-NEXT:                          "left": {
// CHECK-NEXT:                            "type": "UnaryExpression",
// CHECK-NEXT:                            "operator": "-",
// CHECK-NEXT:                            "argument": {
// CHECK-NEXT:                              "type": "Identifier",
// CHECK-NEXT:                              "name": "px"
// CHECK-NEXT:                            },
// CHECK-NEXT:                            "prefix": true
// CHECK-NEXT:                          },
// CHECK-NEXT:                          "right": {
// CHECK-NEXT:                            "type": "Identifier",
// CHECK-NEXT:                            "name": "SOLAR_MASS"
// CHECK-NEXT:                          },
// CHECK-NEXT:                          "operator": "\/"
// CHECK-NEXT:                        }
// CHECK-NEXT:                      },
// CHECK-NEXT:                      "directive": null
// CHECK-NEXT:                    },
// CHECK-NEXT:                    {
// CHECK-NEXT:                      "type": "ExpressionStatement",
// CHECK-NEXT:                      "expression": {
// CHECK-NEXT:                        "type": "AssignmentExpression",
// CHECK-NEXT:                        "operator": "=",
// CHECK-NEXT:                        "left": {
// CHECK-NEXT:                          "type": "MemberExpression",
// CHECK-NEXT:                          "object": {
// CHECK-NEXT:                            "type": "Identifier",
// CHECK-NEXT:                            "name": "body"
// CHECK-NEXT:                          },
// CHECK-NEXT:                          "property": {
// CHECK-NEXT:                            "type": "Identifier",
// CHECK-NEXT:                            "name": "vy"
// CHECK-NEXT:                          },
// CHECK-NEXT:                          "computed": false
// CHECK-NEXT:                        },
// CHECK-NEXT:                        "right": {
// CHECK-NEXT:                          "type": "BinaryExpression",
// CHECK-NEXT:                          "left": {
// CHECK-NEXT:                            "type": "UnaryExpression",
// CHECK-NEXT:                            "operator": "-",
// CHECK-NEXT:                            "argument": {
// CHECK-NEXT:                              "type": "Identifier",
// CHECK-NEXT:                              "name": "py"
// CHECK-NEXT:                            },
// CHECK-NEXT:                            "prefix": true
// CHECK-NEXT:                          },
// CHECK-NEXT:                          "right": {
// CHECK-NEXT:                            "type": "Identifier",
// CHECK-NEXT:                            "name": "SOLAR_MASS"
// CHECK-NEXT:                          },
// CHECK-NEXT:                          "operator": "\/"
// CHECK-NEXT:                        }
// CHECK-NEXT:                      },
// CHECK-NEXT:                      "directive": null
// CHECK-NEXT:                    },
// CHECK-NEXT:                    {
// CHECK-NEXT:                      "type": "ExpressionStatement",
// CHECK-NEXT:                      "expression": {
// CHECK-NEXT:                        "type": "AssignmentExpression",
// CHECK-NEXT:                        "operator": "=",
// CHECK-NEXT:                        "left": {
// CHECK-NEXT:                          "type": "MemberExpression",
// CHECK-NEXT:                          "object": {
// CHECK-NEXT:                            "type": "Identifier",
// CHECK-NEXT:                            "name": "body"
// CHECK-NEXT:                          },
// CHECK-NEXT:                          "property": {
// CHECK-NEXT:                            "type": "Identifier",
// CHECK-NEXT:                            "name": "vz"
// CHECK-NEXT:                          },
// CHECK-NEXT:                          "computed": false
// CHECK-NEXT:                        },
// CHECK-NEXT:                        "right": {
// CHECK-NEXT:                          "type": "BinaryExpression",
// CHECK-NEXT:                          "left": {
// CHECK-NEXT:                            "type": "UnaryExpression",
// CHECK-NEXT:                            "operator": "-",
// CHECK-NEXT:                            "argument": {
// CHECK-NEXT:                              "type": "Identifier",
// CHECK-NEXT:                              "name": "pz"
// CHECK-NEXT:                            },
// CHECK-NEXT:                            "prefix": true
// CHECK-NEXT:                          },
// CHECK-NEXT:                          "right": {
// CHECK-NEXT:                            "type": "Identifier",
// CHECK-NEXT:                            "name": "SOLAR_MASS"
// CHECK-NEXT:                          },
// CHECK-NEXT:                          "operator": "\/"
// CHECK-NEXT:                        }
// CHECK-NEXT:                      },
// CHECK-NEXT:                      "directive": null
// CHECK-NEXT:                    }
// CHECK-NEXT:                  ]
// CHECK-NEXT:                },
// CHECK-NEXT:                "returnType": {
// CHECK-NEXT:                  "type": "TypeAnnotation",
// CHECK-NEXT:                  "typeAnnotation": {
// CHECK-NEXT:                    "type": "VoidTypeAnnotation"
// CHECK-NEXT:                  }
// CHECK-NEXT:                },
// CHECK-NEXT:                "generator": false,
// CHECK-NEXT:                "async": false
// CHECK-NEXT:              },
// CHECK-NEXT:              {
// CHECK-NEXT:                "type": "FunctionDeclaration",
// CHECK-NEXT:                "id": {
// CHECK-NEXT:                  "type": "Identifier",
// CHECK-NEXT:                  "name": "advance"
// CHECK-NEXT:                },
// CHECK-NEXT:                "params": [
// CHECK-NEXT:                  {
// CHECK-NEXT:                    "type": "Identifier",
// CHECK-NEXT:                    "name": "dt",
// CHECK-NEXT:                    "typeAnnotation": {
// CHECK-NEXT:                      "type": "TypeAnnotation",
// CHECK-NEXT:                      "typeAnnotation": {
// CHECK-NEXT:                        "type": "NumberTypeAnnotation"
// CHECK-NEXT:                      }
// CHECK-NEXT:                    }
// CHECK-NEXT:                  }
// CHECK-NEXT:                ],
// CHECK-NEXT:                "body": {
// CHECK-NEXT:                  "type": "BlockStatement",
// CHECK-NEXT:                  "body": [
// CHECK-NEXT:                    {
// CHECK-NEXT:                      "type": "VariableDeclaration",
// CHECK-NEXT:                      "kind": "const",
// CHECK-NEXT:                      "declarations": [
// CHECK-NEXT:                        {
// CHECK-NEXT:                          "type": "VariableDeclarator",
// CHECK-NEXT:                          "init": {
// CHECK-NEXT:                            "type": "MemberExpression",
// CHECK-NEXT:                            "object": {
// CHECK-NEXT:                              "type": "Identifier",
// CHECK-NEXT:                              "name": "bodies"
// CHECK-NEXT:                            },
// CHECK-NEXT:                            "property": {
// CHECK-NEXT:                              "type": "Identifier",
// CHECK-NEXT:                              "name": "length"
// CHECK-NEXT:                            },
// CHECK-NEXT:                            "computed": false
// CHECK-NEXT:                          },
// CHECK-NEXT:                          "id": {
// CHECK-NEXT:                            "type": "Identifier",
// CHECK-NEXT:                            "name": "size",
// CHECK-NEXT:                            "typeAnnotation": {
// CHECK-NEXT:                              "type": "TypeAnnotation",
// CHECK-NEXT:                              "typeAnnotation": {
// CHECK-NEXT:                                "type": "NumberTypeAnnotation"
// CHECK-NEXT:                              }
// CHECK-NEXT:                            }
// CHECK-NEXT:                          }
// CHECK-NEXT:                        }
// CHECK-NEXT:                      ]
// CHECK-NEXT:                    },
// CHECK-NEXT:                    {
// CHECK-NEXT:                      "type": "ForStatement",
// CHECK-NEXT:                      "init": {
// CHECK-NEXT:                        "type": "VariableDeclaration",
// CHECK-NEXT:                        "kind": "let",
// CHECK-NEXT:                        "declarations": [
// CHECK-NEXT:                          {
// CHECK-NEXT:                            "type": "VariableDeclarator",
// CHECK-NEXT:                            "init": {
// CHECK-NEXT:                              "type": "NumericLiteral",
// CHECK-NEXT:                              "value": 0,
// CHECK-NEXT:                              "raw": "0"
// CHECK-NEXT:                            },
// CHECK-NEXT:                            "id": {
// CHECK-NEXT:                              "type": "Identifier",
// CHECK-NEXT:                              "name": "i",
// CHECK-NEXT:                              "typeAnnotation": {
// CHECK-NEXT:                                "type": "TypeAnnotation",
// CHECK-NEXT:                                "typeAnnotation": {
// CHECK-NEXT:                                  "type": "NumberTypeAnnotation"
// CHECK-NEXT:                                }
// CHECK-NEXT:                              }
// CHECK-NEXT:                            }
// CHECK-NEXT:                          }
// CHECK-NEXT:                        ]
// CHECK-NEXT:                      },
// CHECK-NEXT:                      "test": {
// CHECK-NEXT:                        "type": "BinaryExpression",
// CHECK-NEXT:                        "left": {
// CHECK-NEXT:                          "type": "Identifier",
// CHECK-NEXT:                          "name": "i"
// CHECK-NEXT:                        },
// CHECK-NEXT:                        "right": {
// CHECK-NEXT:                          "type": "Identifier",
// CHECK-NEXT:                          "name": "size"
// CHECK-NEXT:                        },
// CHECK-NEXT:                        "operator": "<"
// CHECK-NEXT:                      },
// CHECK-NEXT:                      "update": {
// CHECK-NEXT:                        "type": "UpdateExpression",
// CHECK-NEXT:                        "operator": "++",
// CHECK-NEXT:                        "argument": {
// CHECK-NEXT:                          "type": "Identifier",
// CHECK-NEXT:                          "name": "i"
// CHECK-NEXT:                        },
// CHECK-NEXT:                        "prefix": false
// CHECK-NEXT:                      },
// CHECK-NEXT:                      "body": {
// CHECK-NEXT:                        "type": "BlockStatement",
// CHECK-NEXT:                        "body": [
// CHECK-NEXT:                          {
// CHECK-NEXT:                            "type": "VariableDeclaration",
// CHECK-NEXT:                            "kind": "const",
// CHECK-NEXT:                            "declarations": [
// CHECK-NEXT:                              {
// CHECK-NEXT:                                "type": "VariableDeclarator",
// CHECK-NEXT:                                "init": {
// CHECK-NEXT:                                  "type": "MemberExpression",
// CHECK-NEXT:                                  "object": {
// CHECK-NEXT:                                    "type": "Identifier",
// CHECK-NEXT:                                    "name": "bodies"
// CHECK-NEXT:                                  },
// CHECK-NEXT:                                  "property": {
// CHECK-NEXT:                                    "type": "Identifier",
// CHECK-NEXT:                                    "name": "i"
// CHECK-NEXT:                                  },
// CHECK-NEXT:                                  "computed": true
// CHECK-NEXT:                                },
// CHECK-NEXT:                                "id": {
// CHECK-NEXT:                                  "type": "Identifier",
// CHECK-NEXT:                                  "name": "bodyi",
// CHECK-NEXT:                                  "typeAnnotation": {
// CHECK-NEXT:                                    "type": "TypeAnnotation",
// CHECK-NEXT:                                    "typeAnnotation": {
// CHECK-NEXT:                                      "type": "GenericTypeAnnotation",
// CHECK-NEXT:                                      "id": {
// CHECK-NEXT:                                        "type": "Identifier",
// CHECK-NEXT:                                        "name": "Body"
// CHECK-NEXT:                                      },
// CHECK-NEXT:                                      "typeParameters": null
// CHECK-NEXT:                                    }
// CHECK-NEXT:                                  }
// CHECK-NEXT:                                }
// CHECK-NEXT:                              }
// CHECK-NEXT:                            ]
// CHECK-NEXT:                          },
// CHECK-NEXT:                          {
// CHECK-NEXT:                            "type": "VariableDeclaration",
// CHECK-NEXT:                            "kind": "let",
// CHECK-NEXT:                            "declarations": [
// CHECK-NEXT:                              {
// CHECK-NEXT:                                "type": "VariableDeclarator",
// CHECK-NEXT:                                "init": {
// CHECK-NEXT:                                  "type": "MemberExpression",
// CHECK-NEXT:                                  "object": {
// CHECK-NEXT:                                    "type": "Identifier",
// CHECK-NEXT:                                    "name": "bodyi"
// CHECK-NEXT:                                  },
// CHECK-NEXT:                                  "property": {
// CHECK-NEXT:                                    "type": "Identifier",
// CHECK-NEXT:                                    "name": "vx"
// CHECK-NEXT:                                  },
// CHECK-NEXT:                                  "computed": false
// CHECK-NEXT:                                },
// CHECK-NEXT:                                "id": {
// CHECK-NEXT:                                  "type": "Identifier",
// CHECK-NEXT:                                  "name": "vxi",
// CHECK-NEXT:                                  "typeAnnotation": {
// CHECK-NEXT:                                    "type": "TypeAnnotation",
// CHECK-NEXT:                                    "typeAnnotation": {
// CHECK-NEXT:                                      "type": "NumberTypeAnnotation"
// CHECK-NEXT:                                    }
// CHECK-NEXT:                                  }
// CHECK-NEXT:                                }
// CHECK-NEXT:                              }
// CHECK-NEXT:                            ]
// CHECK-NEXT:                          },
// CHECK-NEXT:                          {
// CHECK-NEXT:                            "type": "VariableDeclaration",
// CHECK-NEXT:                            "kind": "let",
// CHECK-NEXT:                            "declarations": [
// CHECK-NEXT:                              {
// CHECK-NEXT:                                "type": "VariableDeclarator",
// CHECK-NEXT:                                "init": {
// CHECK-NEXT:                                  "type": "MemberExpression",
// CHECK-NEXT:                                  "object": {
// CHECK-NEXT:                                    "type": "Identifier",
// CHECK-NEXT:                                    "name": "bodyi"
// CHECK-NEXT:                                  },
// CHECK-NEXT:                                  "property": {
// CHECK-NEXT:                                    "type": "Identifier",
// CHECK-NEXT:                                    "name": "vy"
// CHECK-NEXT:                                  },
// CHECK-NEXT:                                  "computed": false
// CHECK-NEXT:                                },
// CHECK-NEXT:                                "id": {
// CHECK-NEXT:                                  "type": "Identifier",
// CHECK-NEXT:                                  "name": "vyi",
// CHECK-NEXT:                                  "typeAnnotation": {
// CHECK-NEXT:                                    "type": "TypeAnnotation",
// CHECK-NEXT:                                    "typeAnnotation": {
// CHECK-NEXT:                                      "type": "NumberTypeAnnotation"
// CHECK-NEXT:                                    }
// CHECK-NEXT:                                  }
// CHECK-NEXT:                                }
// CHECK-NEXT:                              }
// CHECK-NEXT:                            ]
// CHECK-NEXT:                          },
// CHECK-NEXT:                          {
// CHECK-NEXT:                            "type": "VariableDeclaration",
// CHECK-NEXT:                            "kind": "let",
// CHECK-NEXT:                            "declarations": [
// CHECK-NEXT:                              {
// CHECK-NEXT:                                "type": "VariableDeclarator",
// CHECK-NEXT:                                "init": {
// CHECK-NEXT:                                  "type": "MemberExpression",
// CHECK-NEXT:                                  "object": {
// CHECK-NEXT:                                    "type": "Identifier",
// CHECK-NEXT:                                    "name": "bodyi"
// CHECK-NEXT:                                  },
// CHECK-NEXT:                                  "property": {
// CHECK-NEXT:                                    "type": "Identifier",
// CHECK-NEXT:                                    "name": "vz"
// CHECK-NEXT:                                  },
// CHECK-NEXT:                                  "computed": false
// CHECK-NEXT:                                },
// CHECK-NEXT:                                "id": {
// CHECK-NEXT:                                  "type": "Identifier",
// CHECK-NEXT:                                  "name": "vzi",
// CHECK-NEXT:                                  "typeAnnotation": {
// CHECK-NEXT:                                    "type": "TypeAnnotation",
// CHECK-NEXT:                                    "typeAnnotation": {
// CHECK-NEXT:                                      "type": "NumberTypeAnnotation"
// CHECK-NEXT:                                    }
// CHECK-NEXT:                                  }
// CHECK-NEXT:                                }
// CHECK-NEXT:                              }
// CHECK-NEXT:                            ]
// CHECK-NEXT:                          },
// CHECK-NEXT:                          {
// CHECK-NEXT:                            "type": "ForStatement",
// CHECK-NEXT:                            "init": {
// CHECK-NEXT:                              "type": "VariableDeclaration",
// CHECK-NEXT:                              "kind": "let",
// CHECK-NEXT:                              "declarations": [
// CHECK-NEXT:                                {
// CHECK-NEXT:                                  "type": "VariableDeclarator",
// CHECK-NEXT:                                  "init": {
// CHECK-NEXT:                                    "type": "BinaryExpression",
// CHECK-NEXT:                                    "left": {
// CHECK-NEXT:                                      "type": "Identifier",
// CHECK-NEXT:                                      "name": "i"
// CHECK-NEXT:                                    },
// CHECK-NEXT:                                    "right": {
// CHECK-NEXT:                                      "type": "NumericLiteral",
// CHECK-NEXT:                                      "value": 1,
// CHECK-NEXT:                                      "raw": "1"
// CHECK-NEXT:                                    },
// CHECK-NEXT:                                    "operator": "+"
// CHECK-NEXT:                                  },
// CHECK-NEXT:                                  "id": {
// CHECK-NEXT:                                    "type": "Identifier",
// CHECK-NEXT:                                    "name": "j",
// CHECK-NEXT:                                    "typeAnnotation": {
// CHECK-NEXT:                                      "type": "TypeAnnotation",
// CHECK-NEXT:                                      "typeAnnotation": {
// CHECK-NEXT:                                        "type": "NumberTypeAnnotation"
// CHECK-NEXT:                                      }
// CHECK-NEXT:                                    }
// CHECK-NEXT:                                  }
// CHECK-NEXT:                                }
// CHECK-NEXT:                              ]
// CHECK-NEXT:                            },
// CHECK-NEXT:                            "test": {
// CHECK-NEXT:                              "type": "BinaryExpression",
// CHECK-NEXT:                              "left": {
// CHECK-NEXT:                                "type": "Identifier",
// CHECK-NEXT:                                "name": "j"
// CHECK-NEXT:                              },
// CHECK-NEXT:                              "right": {
// CHECK-NEXT:                                "type": "Identifier",
// CHECK-NEXT:                                "name": "size"
// CHECK-NEXT:                              },
// CHECK-NEXT:                              "operator": "<"
// CHECK-NEXT:                            },
// CHECK-NEXT:                            "update": {
// CHECK-NEXT:                              "type": "UpdateExpression",
// CHECK-NEXT:                              "operator": "++",
// CHECK-NEXT:                              "argument": {
// CHECK-NEXT:                                "type": "Identifier",
// CHECK-NEXT:                                "name": "j"
// CHECK-NEXT:                              },
// CHECK-NEXT:                              "prefix": false
// CHECK-NEXT:                            },
// CHECK-NEXT:                            "body": {
// CHECK-NEXT:                              "type": "BlockStatement",
// CHECK-NEXT:                              "body": [
// CHECK-NEXT:                                {
// CHECK-NEXT:                                  "type": "VariableDeclaration",
// CHECK-NEXT:                                  "kind": "const",
// CHECK-NEXT:                                  "declarations": [
// CHECK-NEXT:                                    {
// CHECK-NEXT:                                      "type": "VariableDeclarator",
// CHECK-NEXT:                                      "init": {
// CHECK-NEXT:                                        "type": "MemberExpression",
// CHECK-NEXT:                                        "object": {
// CHECK-NEXT:                                          "type": "Identifier",
// CHECK-NEXT:                                          "name": "bodies"
// CHECK-NEXT:                                        },
// CHECK-NEXT:                                        "property": {
// CHECK-NEXT:                                          "type": "Identifier",
// CHECK-NEXT:                                          "name": "j"
// CHECK-NEXT:                                        },
// CHECK-NEXT:                                        "computed": true
// CHECK-NEXT:                                      },
// CHECK-NEXT:                                      "id": {
// CHECK-NEXT:                                        "type": "Identifier",
// CHECK-NEXT:                                        "name": "bodyj",
// CHECK-NEXT:                                        "typeAnnotation": {
// CHECK-NEXT:                                          "type": "TypeAnnotation",
// CHECK-NEXT:                                          "typeAnnotation": {
// CHECK-NEXT:                                            "type": "GenericTypeAnnotation",
// CHECK-NEXT:                                            "id": {
// CHECK-NEXT:                                              "type": "Identifier",
// CHECK-NEXT:                                              "name": "Body"
// CHECK-NEXT:                                            },
// CHECK-NEXT:                                            "typeParameters": null
// CHECK-NEXT:                                          }
// CHECK-NEXT:                                        }
// CHECK-NEXT:                                      }
// CHECK-NEXT:                                    }
// CHECK-NEXT:                                  ]
// CHECK-NEXT:                                },
// CHECK-NEXT:                                {
// CHECK-NEXT:                                  "type": "VariableDeclaration",
// CHECK-NEXT:                                  "kind": "const",
// CHECK-NEXT:                                  "declarations": [
// CHECK-NEXT:                                    {
// CHECK-NEXT:                                      "type": "VariableDeclarator",
// CHECK-NEXT:                                      "init": {
// CHECK-NEXT:                                        "type": "BinaryExpression",
// CHECK-NEXT:                                        "left": {
// CHECK-NEXT:                                          "type": "MemberExpression",
// CHECK-NEXT:                                          "object": {
// CHECK-NEXT:                                            "type": "Identifier",
// CHECK-NEXT:                                            "name": "bodyi"
// CHECK-NEXT:                                          },
// CHECK-NEXT:                                          "property": {
// CHECK-NEXT:                                            "type": "Identifier",
// CHECK-NEXT:                                            "name": "x"
// CHECK-NEXT:                                          },
// CHECK-NEXT:                                          "computed": false
// CHECK-NEXT:                                        },
// CHECK-NEXT:                                        "right": {
// CHECK-NEXT:                                          "type": "MemberExpression",
// CHECK-NEXT:                                          "object": {
// CHECK-NEXT:                                            "type": "Identifier",
// CHECK-NEXT:                                            "name": "bodyj"
// CHECK-NEXT:                                          },
// CHECK-NEXT:                                          "property": {
// CHECK-NEXT:                                            "type": "Identifier",
// CHECK-NEXT:                                            "name": "x"
// CHECK-NEXT:                                          },
// CHECK-NEXT:                                          "computed": false
// CHECK-NEXT:                                        },
// CHECK-NEXT:                                        "operator": "-"
// CHECK-NEXT:                                      },
// CHECK-NEXT:                                      "id": {
// CHECK-NEXT:                                        "type": "Identifier",
// CHECK-NEXT:                                        "name": "dx",
// CHECK-NEXT:                                        "typeAnnotation": {
// CHECK-NEXT:                                          "type": "TypeAnnotation",
// CHECK-NEXT:                                          "typeAnnotation": {
// CHECK-NEXT:                                            "type": "NumberTypeAnnotation"
// CHECK-NEXT:                                          }
// CHECK-NEXT:                                        }
// CHECK-NEXT:                                      }
// CHECK-NEXT:                                    }
// CHECK-NEXT:                                  ]
// CHECK-NEXT:                                },
// CHECK-NEXT:                                {
// CHECK-NEXT:                                  "type": "VariableDeclaration",
// CHECK-NEXT:                                  "kind": "const",
// CHECK-NEXT:                                  "declarations": [
// CHECK-NEXT:                                    {
// CHECK-NEXT:                                      "type": "VariableDeclarator",
// CHECK-NEXT:                                      "init": {
// CHECK-NEXT:                                        "type": "BinaryExpression",
// CHECK-NEXT:                                        "left": {
// CHECK-NEXT:                                          "type": "MemberExpression",
// CHECK-NEXT:                                          "object": {
// CHECK-NEXT:                                            "type": "Identifier",
// CHECK-NEXT:                                            "name": "bodyi"
// CHECK-NEXT:                                          },
// CHECK-NEXT:                                          "property": {
// CHECK-NEXT:                                            "type": "Identifier",
// CHECK-NEXT:                                            "name": "y"
// CHECK-NEXT:                                          },
// CHECK-NEXT:                                          "computed": false
// CHECK-NEXT:                                        },
// CHECK-NEXT:                                        "right": {
// CHECK-NEXT:                                          "type": "MemberExpression",
// CHECK-NEXT:                                          "object": {
// CHECK-NEXT:                                            "type": "Identifier",
// CHECK-NEXT:                                            "name": "bodyj"
// CHECK-NEXT:                                          },
// CHECK-NEXT:                                          "property": {
// CHECK-NEXT:                                            "type": "Identifier",
// CHECK-NEXT:                                            "name": "y"
// CHECK-NEXT:                                          },
// CHECK-NEXT:                                          "computed": false
// CHECK-NEXT:                                        },
// CHECK-NEXT:                                        "operator": "-"
// CHECK-NEXT:                                      },
// CHECK-NEXT:                                      "id": {
// CHECK-NEXT:                                        "type": "Identifier",
// CHECK-NEXT:                                        "name": "dy",
// CHECK-NEXT:                                        "typeAnnotation": {
// CHECK-NEXT:                                          "type": "TypeAnnotation",
// CHECK-NEXT:                                          "typeAnnotation": {
// CHECK-NEXT:                                            "type": "NumberTypeAnnotation"
// CHECK-NEXT:                                          }
// CHECK-NEXT:                                        }
// CHECK-NEXT:                                      }
// CHECK-NEXT:                                    }
// CHECK-NEXT:                                  ]
// CHECK-NEXT:                                },
// CHECK-NEXT:                                {
// CHECK-NEXT:                                  "type": "VariableDeclaration",
// CHECK-NEXT:                                  "kind": "const",
// CHECK-NEXT:                                  "declarations": [
// CHECK-NEXT:                                    {
// CHECK-NEXT:                                      "type": "VariableDeclarator",
// CHECK-NEXT:                                      "init": {
// CHECK-NEXT:                                        "type": "BinaryExpression",
// CHECK-NEXT:                                        "left": {
// CHECK-NEXT:                                          "type": "MemberExpression",
// CHECK-NEXT:                                          "object": {
// CHECK-NEXT:                                            "type": "Identifier",
// CHECK-NEXT:                                            "name": "bodyi"
// CHECK-NEXT:                                          },
// CHECK-NEXT:                                          "property": {
// CHECK-NEXT:                                            "type": "Identifier",
// CHECK-NEXT:                                            "name": "z"
// CHECK-NEXT:                                          },
// CHECK-NEXT:                                          "computed": false
// CHECK-NEXT:                                        },
// CHECK-NEXT:                                        "right": {
// CHECK-NEXT:                                          "type": "MemberExpression",
// CHECK-NEXT:                                          "object": {
// CHECK-NEXT:                                            "type": "Identifier",
// CHECK-NEXT:                                            "name": "bodyj"
// CHECK-NEXT:                                          },
// CHECK-NEXT:                                          "property": {
// CHECK-NEXT:                                            "type": "Identifier",
// CHECK-NEXT:                                            "name": "z"
// CHECK-NEXT:                                          },
// CHECK-NEXT:                                          "computed": false
// CHECK-NEXT:                                        },
// CHECK-NEXT:                                        "operator": "-"
// CHECK-NEXT:                                      },
// CHECK-NEXT:                                      "id": {
// CHECK-NEXT:                                        "type": "Identifier",
// CHECK-NEXT:                                        "name": "dz",
// CHECK-NEXT:                                        "typeAnnotation": {
// CHECK-NEXT:                                          "type": "TypeAnnotation",
// CHECK-NEXT:                                          "typeAnnotation": {
// CHECK-NEXT:                                            "type": "NumberTypeAnnotation"
// CHECK-NEXT:                                          }
// CHECK-NEXT:                                        }
// CHECK-NEXT:                                      }
// CHECK-NEXT:                                    }
// CHECK-NEXT:                                  ]
// CHECK-NEXT:                                },
// CHECK-NEXT:                                {
// CHECK-NEXT:                                  "type": "VariableDeclaration",
// CHECK-NEXT:                                  "kind": "const",
// CHECK-NEXT:                                  "declarations": [
// CHECK-NEXT:                                    {
// CHECK-NEXT:                                      "type": "VariableDeclarator",
// CHECK-NEXT:                                      "init": {
// CHECK-NEXT:                                        "type": "BinaryExpression",
// CHECK-NEXT:                                        "left": {
// CHECK-NEXT:                                          "type": "BinaryExpression",
// CHECK-NEXT:                                          "left": {
// CHECK-NEXT:                                            "type": "BinaryExpression",
// CHECK-NEXT:                                            "left": {
// CHECK-NEXT:                                              "type": "Identifier",
// CHECK-NEXT:                                              "name": "dx"
// CHECK-NEXT:                                            },
// CHECK-NEXT:                                            "right": {
// CHECK-NEXT:                                              "type": "Identifier",
// CHECK-NEXT:                                              "name": "dx"
// CHECK-NEXT:                                            },
// CHECK-NEXT:                                            "operator": "*"
// CHECK-NEXT:                                          },
// CHECK-NEXT:                                          "right": {
// CHECK-NEXT:                                            "type": "BinaryExpression",
// CHECK-NEXT:                                            "left": {
// CHECK-NEXT:                                              "type": "Identifier",
// CHECK-NEXT:                                              "name": "dy"
// CHECK-NEXT:                                            },
// CHECK-NEXT:                                            "right": {
// CHECK-NEXT:                                              "type": "Identifier",
// CHECK-NEXT:                                              "name": "dy"
// CHECK-NEXT:                                            },
// CHECK-NEXT:                                            "operator": "*"
// CHECK-NEXT:                                          },
// CHECK-NEXT:                                          "operator": "+"
// CHECK-NEXT:                                        },
// CHECK-NEXT:                                        "right": {
// CHECK-NEXT:                                          "type": "BinaryExpression",
// CHECK-NEXT:                                          "left": {
// CHECK-NEXT:                                            "type": "Identifier",
// CHECK-NEXT:                                            "name": "dz"
// CHECK-NEXT:                                          },
// CHECK-NEXT:                                          "right": {
// CHECK-NEXT:                                            "type": "Identifier",
// CHECK-NEXT:                                            "name": "dz"
// CHECK-NEXT:                                          },
// CHECK-NEXT:                                          "operator": "*"
// CHECK-NEXT:                                        },
// CHECK-NEXT:                                        "operator": "+"
// CHECK-NEXT:                                      },
// CHECK-NEXT:                                      "id": {
// CHECK-NEXT:                                        "type": "Identifier",
// CHECK-NEXT:                                        "name": "d2",
// CHECK-NEXT:                                        "typeAnnotation": {
// CHECK-NEXT:                                          "type": "TypeAnnotation",
// CHECK-NEXT:                                          "typeAnnotation": {
// CHECK-NEXT:                                            "type": "NumberTypeAnnotation"
// CHECK-NEXT:                                          }
// CHECK-NEXT:                                        }
// CHECK-NEXT:                                      }
// CHECK-NEXT:                                    }
// CHECK-NEXT:                                  ]
// CHECK-NEXT:                                },
// CHECK-NEXT:                                {
// CHECK-NEXT:                                  "type": "VariableDeclaration",
// CHECK-NEXT:                                  "kind": "const",
// CHECK-NEXT:                                  "declarations": [
// CHECK-NEXT:                                    {
// CHECK-NEXT:                                      "type": "VariableDeclarator",
// CHECK-NEXT:                                      "init": {
// CHECK-NEXT:                                        "type": "BinaryExpression",
// CHECK-NEXT:                                        "left": {
// CHECK-NEXT:                                          "type": "Identifier",
// CHECK-NEXT:                                          "name": "dt"
// CHECK-NEXT:                                        },
// CHECK-NEXT:                                        "right": {
// CHECK-NEXT:                                          "type": "BinaryExpression",
// CHECK-NEXT:                                          "left": {
// CHECK-NEXT:                                            "type": "Identifier",
// CHECK-NEXT:                                            "name": "d2"
// CHECK-NEXT:                                          },
// CHECK-NEXT:                                          "right": {
// CHECK-NEXT:                                            "type": "CallExpression",
// CHECK-NEXT:                                            "callee": {
// CHECK-NEXT:                                              "type": "MemberExpression",
// CHECK-NEXT:                                              "object": {
// CHECK-NEXT:                                                "type": "Identifier",
// CHECK-NEXT:                                                "name": "Math"
// CHECK-NEXT:                                              },
// CHECK-NEXT:                                              "property": {
// CHECK-NEXT:                                                "type": "Identifier",
// CHECK-NEXT:                                                "name": "sqrt"
// CHECK-NEXT:                                              },
// CHECK-NEXT:                                              "computed": false
// CHECK-NEXT:                                            },
// CHECK-NEXT:                                            "arguments": [
// CHECK-NEXT:                                              {
// CHECK-NEXT:                                                "type": "Identifier",
// CHECK-NEXT:                                                "name": "d2"
// CHECK-NEXT:                                              }
// CHECK-NEXT:                                            ]
// CHECK-NEXT:                                          },
// CHECK-NEXT:                                          "operator": "*"
// CHECK-NEXT:                                        },
// CHECK-NEXT:                                        "operator": "\/"
// CHECK-NEXT:                                      },
// CHECK-NEXT:                                      "id": {
// CHECK-NEXT:                                        "type": "Identifier",
// CHECK-NEXT:                                        "name": "mag",
// CHECK-NEXT:                                        "typeAnnotation": {
// CHECK-NEXT:                                          "type": "TypeAnnotation",
// CHECK-NEXT:                                          "typeAnnotation": {
// CHECK-NEXT:                                            "type": "NumberTypeAnnotation"
// CHECK-NEXT:                                          }
// CHECK-NEXT:                                        }
// CHECK-NEXT:                                      }
// CHECK-NEXT:                                    }
// CHECK-NEXT:                                  ]
// CHECK-NEXT:                                },
// CHECK-NEXT:                                {
// CHECK-NEXT:                                  "type": "VariableDeclaration",
// CHECK-NEXT:                                  "kind": "const",
// CHECK-NEXT:                                  "declarations": [
// CHECK-NEXT:                                    {
// CHECK-NEXT:                                      "type": "VariableDeclarator",
// CHECK-NEXT:                                      "init": {
// CHECK-NEXT:                                        "type": "MemberExpression",
// CHECK-NEXT:                                        "object": {
// CHECK-NEXT:                                          "type": "Identifier",
// CHECK-NEXT:                                          "name": "bodyj"
// CHECK-NEXT:                                        },
// CHECK-NEXT:                                        "property": {
// CHECK-NEXT:                                          "type": "Identifier",
// CHECK-NEXT:                                          "name": "mass"
// CHECK-NEXT:                                        },
// CHECK-NEXT:                                        "computed": false
// CHECK-NEXT:                                      },
// CHECK-NEXT:                                      "id": {
// CHECK-NEXT:                                        "type": "Identifier",
// CHECK-NEXT:                                        "name": "massj",
// CHECK-NEXT:                                        "typeAnnotation": {
// CHECK-NEXT:                                          "type": "TypeAnnotation",
// CHECK-NEXT:                                          "typeAnnotation": {
// CHECK-NEXT:                                            "type": "NumberTypeAnnotation"
// CHECK-NEXT:                                          }
// CHECK-NEXT:                                        }
// CHECK-NEXT:                                      }
// CHECK-NEXT:                                    }
// CHECK-NEXT:                                  ]
// CHECK-NEXT:                                },
// CHECK-NEXT:                                {
// CHECK-NEXT:                                  "type": "ExpressionStatement",
// CHECK-NEXT:                                  "expression": {
// CHECK-NEXT:                                    "type": "AssignmentExpression",
// CHECK-NEXT:                                    "operator": "-=",
// CHECK-NEXT:                                    "left": {
// CHECK-NEXT:                                      "type": "Identifier",
// CHECK-NEXT:                                      "name": "vxi"
// CHECK-NEXT:                                    },
// CHECK-NEXT:                                    "right": {
// CHECK-NEXT:                                      "type": "BinaryExpression",
// CHECK-NEXT:                                      "left": {
// CHECK-NEXT:                                        "type": "BinaryExpression",
// CHECK-NEXT:                                        "left": {
// CHECK-NEXT:                                          "type": "Identifier",
// CHECK-NEXT:                                          "name": "dx"
// CHECK-NEXT:                                        },
// CHECK-NEXT:                                        "right": {
// CHECK-NEXT:                                          "type": "Identifier",
// CHECK-NEXT:                                          "name": "massj"
// CHECK-NEXT:                                        },
// CHECK-NEXT:                                        "operator": "*"
// CHECK-NEXT:                                      },
// CHECK-NEXT:                                      "right": {
// CHECK-NEXT:                                        "type": "Identifier",
// CHECK-NEXT:                                        "name": "mag"
// CHECK-NEXT:                                      },
// CHECK-NEXT:                                      "operator": "*"
// CHECK-NEXT:                                    }
// CHECK-NEXT:                                  },
// CHECK-NEXT:                                  "directive": null
// CHECK-NEXT:                                },
// CHECK-NEXT:                                {
// CHECK-NEXT:                                  "type": "ExpressionStatement",
// CHECK-NEXT:                                  "expression": {
// CHECK-NEXT:                                    "type": "AssignmentExpression",
// CHECK-NEXT:                                    "operator": "-=",
// CHECK-NEXT:                                    "left": {
// CHECK-NEXT:                                      "type": "Identifier",
// CHECK-NEXT:                                      "name": "vyi"
// CHECK-NEXT:                                    },
// CHECK-NEXT:                                    "right": {
// CHECK-NEXT:                                      "type": "BinaryExpression",
// CHECK-NEXT:                                      "left": {
// CHECK-NEXT:                                        "type": "BinaryExpression",
// CHECK-NEXT:                                        "left": {
// CHECK-NEXT:                                          "type": "Identifier",
// CHECK-NEXT:                                          "name": "dy"
// CHECK-NEXT:                                        },
// CHECK-NEXT:                                        "right": {
// CHECK-NEXT:                                          "type": "Identifier",
// CHECK-NEXT:                                          "name": "massj"
// CHECK-NEXT:                                        },
// CHECK-NEXT:                                        "operator": "*"
// CHECK-NEXT:                                      },
// CHECK-NEXT:                                      "right": {
// CHECK-NEXT:                                        "type": "Identifier",
// CHECK-NEXT:                                        "name": "mag"
// CHECK-NEXT:                                      },
// CHECK-NEXT:                                      "operator": "*"
// CHECK-NEXT:                                    }
// CHECK-NEXT:                                  },
// CHECK-NEXT:                                  "directive": null
// CHECK-NEXT:                                },
// CHECK-NEXT:                                {
// CHECK-NEXT:                                  "type": "ExpressionStatement",
// CHECK-NEXT:                                  "expression": {
// CHECK-NEXT:                                    "type": "AssignmentExpression",
// CHECK-NEXT:                                    "operator": "-=",
// CHECK-NEXT:                                    "left": {
// CHECK-NEXT:                                      "type": "Identifier",
// CHECK-NEXT:                                      "name": "vzi"
// CHECK-NEXT:                                    },
// CHECK-NEXT:                                    "right": {
// CHECK-NEXT:                                      "type": "BinaryExpression",
// CHECK-NEXT:                                      "left": {
// CHECK-NEXT:                                        "type": "BinaryExpression",
// CHECK-NEXT:                                        "left": {
// CHECK-NEXT:                                          "type": "Identifier",
// CHECK-NEXT:                                          "name": "dz"
// CHECK-NEXT:                                        },
// CHECK-NEXT:                                        "right": {
// CHECK-NEXT:                                          "type": "Identifier",
// CHECK-NEXT:                                          "name": "massj"
// CHECK-NEXT:                                        },
// CHECK-NEXT:                                        "operator": "*"
// CHECK-NEXT:                                      },
// CHECK-NEXT:                                      "right": {
// CHECK-NEXT:                                        "type": "Identifier",
// CHECK-NEXT:                                        "name": "mag"
// CHECK-NEXT:                                      },
// CHECK-NEXT:                                      "operator": "*"
// CHECK-NEXT:                                    }
// CHECK-NEXT:                                  },
// CHECK-NEXT:                                  "directive": null
// CHECK-NEXT:                                },
// CHECK-NEXT:                                {
// CHECK-NEXT:                                  "type": "VariableDeclaration",
// CHECK-NEXT:                                  "kind": "const",
// CHECK-NEXT:                                  "declarations": [
// CHECK-NEXT:                                    {
// CHECK-NEXT:                                      "type": "VariableDeclarator",
// CHECK-NEXT:                                      "init": {
// CHECK-NEXT:                                        "type": "MemberExpression",
// CHECK-NEXT:                                        "object": {
// CHECK-NEXT:                                          "type": "Identifier",
// CHECK-NEXT:                                          "name": "bodyi"
// CHECK-NEXT:                                        },
// CHECK-NEXT:                                        "property": {
// CHECK-NEXT:                                          "type": "Identifier",
// CHECK-NEXT:                                          "name": "mass"
// CHECK-NEXT:                                        },
// CHECK-NEXT:                                        "computed": false
// CHECK-NEXT:                                      },
// CHECK-NEXT:                                      "id": {
// CHECK-NEXT:                                        "type": "Identifier",
// CHECK-NEXT:                                        "name": "massi",
// CHECK-NEXT:                                        "typeAnnotation": {
// CHECK-NEXT:                                          "type": "TypeAnnotation",
// CHECK-NEXT:                                          "typeAnnotation": {
// CHECK-NEXT:                                            "type": "NumberTypeAnnotation"
// CHECK-NEXT:                                          }
// CHECK-NEXT:                                        }
// CHECK-NEXT:                                      }
// CHECK-NEXT:                                    }
// CHECK-NEXT:                                  ]
// CHECK-NEXT:                                },
// CHECK-NEXT:                                {
// CHECK-NEXT:                                  "type": "ExpressionStatement",
// CHECK-NEXT:                                  "expression": {
// CHECK-NEXT:                                    "type": "AssignmentExpression",
// CHECK-NEXT:                                    "operator": "+=",
// CHECK-NEXT:                                    "left": {
// CHECK-NEXT:                                      "type": "MemberExpression",
// CHECK-NEXT:                                      "object": {
// CHECK-NEXT:                                        "type": "Identifier",
// CHECK-NEXT:                                        "name": "bodyj"
// CHECK-NEXT:                                      },
// CHECK-NEXT:                                      "property": {
// CHECK-NEXT:                                        "type": "Identifier",
// CHECK-NEXT:                                        "name": "vx"
// CHECK-NEXT:                                      },
// CHECK-NEXT:                                      "computed": false
// CHECK-NEXT:                                    },
// CHECK-NEXT:                                    "right": {
// CHECK-NEXT:                                      "type": "BinaryExpression",
// CHECK-NEXT:                                      "left": {
// CHECK-NEXT:                                        "type": "BinaryExpression",
// CHECK-NEXT:                                        "left": {
// CHECK-NEXT:                                          "type": "Identifier",
// CHECK-NEXT:                                          "name": "dx"
// CHECK-NEXT:                                        },
// CHECK-NEXT:                                        "right": {
// CHECK-NEXT:                                          "type": "Identifier",
// CHECK-NEXT:                                          "name": "massi"
// CHECK-NEXT:                                        },
// CHECK-NEXT:                                        "operator": "*"
// CHECK-NEXT:                                      },
// CHECK-NEXT:                                      "right": {
// CHECK-NEXT:                                        "type": "Identifier",
// CHECK-NEXT:                                        "name": "mag"
// CHECK-NEXT:                                      },
// CHECK-NEXT:                                      "operator": "*"
// CHECK-NEXT:                                    }
// CHECK-NEXT:                                  },
// CHECK-NEXT:                                  "directive": null
// CHECK-NEXT:                                },
// CHECK-NEXT:                                {
// CHECK-NEXT:                                  "type": "ExpressionStatement",
// CHECK-NEXT:                                  "expression": {
// CHECK-NEXT:                                    "type": "AssignmentExpression",
// CHECK-NEXT:                                    "operator": "+=",
// CHECK-NEXT:                                    "left": {
// CHECK-NEXT:                                      "type": "MemberExpression",
// CHECK-NEXT:                                      "object": {
// CHECK-NEXT:                                        "type": "Identifier",
// CHECK-NEXT:                                        "name": "bodyj"
// CHECK-NEXT:                                      },
// CHECK-NEXT:                                      "property": {
// CHECK-NEXT:                                        "type": "Identifier",
// CHECK-NEXT:                                        "name": "vy"
// CHECK-NEXT:                                      },
// CHECK-NEXT:                                      "computed": false
// CHECK-NEXT:                                    },
// CHECK-NEXT:                                    "right": {
// CHECK-NEXT:                                      "type": "BinaryExpression",
// CHECK-NEXT:                                      "left": {
// CHECK-NEXT:                                        "type": "BinaryExpression",
// CHECK-NEXT:                                        "left": {
// CHECK-NEXT:                                          "type": "Identifier",
// CHECK-NEXT:                                          "name": "dy"
// CHECK-NEXT:                                        },
// CHECK-NEXT:                                        "right": {
// CHECK-NEXT:                                          "type": "Identifier",
// CHECK-NEXT:                                          "name": "massi"
// CHECK-NEXT:                                        },
// CHECK-NEXT:                                        "operator": "*"
// CHECK-NEXT:                                      },
// CHECK-NEXT:                                      "right": {
// CHECK-NEXT:                                        "type": "Identifier",
// CHECK-NEXT:                                        "name": "mag"
// CHECK-NEXT:                                      },
// CHECK-NEXT:                                      "operator": "*"
// CHECK-NEXT:                                    }
// CHECK-NEXT:                                  },
// CHECK-NEXT:                                  "directive": null
// CHECK-NEXT:                                },
// CHECK-NEXT:                                {
// CHECK-NEXT:                                  "type": "ExpressionStatement",
// CHECK-NEXT:                                  "expression": {
// CHECK-NEXT:                                    "type": "AssignmentExpression",
// CHECK-NEXT:                                    "operator": "+=",
// CHECK-NEXT:                                    "left": {
// CHECK-NEXT:                                      "type": "MemberExpression",
// CHECK-NEXT:                                      "object": {
// CHECK-NEXT:                                        "type": "Identifier",
// CHECK-NEXT:                                        "name": "bodyj"
// CHECK-NEXT:                                      },
// CHECK-NEXT:                                      "property": {
// CHECK-NEXT:                                        "type": "Identifier",
// CHECK-NEXT:                                        "name": "vz"
// CHECK-NEXT:                                      },
// CHECK-NEXT:                                      "computed": false
// CHECK-NEXT:                                    },
// CHECK-NEXT:                                    "right": {
// CHECK-NEXT:                                      "type": "BinaryExpression",
// CHECK-NEXT:                                      "left": {
// CHECK-NEXT:                                        "type": "BinaryExpression",
// CHECK-NEXT:                                        "left": {
// CHECK-NEXT:                                          "type": "Identifier",
// CHECK-NEXT:                                          "name": "dz"
// CHECK-NEXT:                                        },
// CHECK-NEXT:                                        "right": {
// CHECK-NEXT:                                          "type": "Identifier",
// CHECK-NEXT:                                          "name": "massi"
// CHECK-NEXT:                                        },
// CHECK-NEXT:                                        "operator": "*"
// CHECK-NEXT:                                      },
// CHECK-NEXT:                                      "right": {
// CHECK-NEXT:                                        "type": "Identifier",
// CHECK-NEXT:                                        "name": "mag"
// CHECK-NEXT:                                      },
// CHECK-NEXT:                                      "operator": "*"
// CHECK-NEXT:                                    }
// CHECK-NEXT:                                  },
// CHECK-NEXT:                                  "directive": null
// CHECK-NEXT:                                }
// CHECK-NEXT:                              ]
// CHECK-NEXT:                            }
// CHECK-NEXT:                          },
// CHECK-NEXT:                          {
// CHECK-NEXT:                            "type": "ExpressionStatement",
// CHECK-NEXT:                            "expression": {
// CHECK-NEXT:                              "type": "AssignmentExpression",
// CHECK-NEXT:                              "operator": "=",
// CHECK-NEXT:                              "left": {
// CHECK-NEXT:                                "type": "MemberExpression",
// CHECK-NEXT:                                "object": {
// CHECK-NEXT:                                  "type": "Identifier",
// CHECK-NEXT:                                  "name": "bodyi"
// CHECK-NEXT:                                },
// CHECK-NEXT:                                "property": {
// CHECK-NEXT:                                  "type": "Identifier",
// CHECK-NEXT:                                  "name": "vx"
// CHECK-NEXT:                                },
// CHECK-NEXT:                                "computed": false
// CHECK-NEXT:                              },
// CHECK-NEXT:                              "right": {
// CHECK-NEXT:                                "type": "Identifier",
// CHECK-NEXT:                                "name": "vxi"
// CHECK-NEXT:                              }
// CHECK-NEXT:                            },
// CHECK-NEXT:                            "directive": null
// CHECK-NEXT:                          },
// CHECK-NEXT:                          {
// CHECK-NEXT:                            "type": "ExpressionStatement",
// CHECK-NEXT:                            "expression": {
// CHECK-NEXT:                              "type": "AssignmentExpression",
// CHECK-NEXT:                              "operator": "=",
// CHECK-NEXT:                              "left": {
// CHECK-NEXT:                                "type": "MemberExpression",
// CHECK-NEXT:                                "object": {
// CHECK-NEXT:                                  "type": "Identifier",
// CHECK-NEXT:                                  "name": "bodyi"
// CHECK-NEXT:                                },
// CHECK-NEXT:                                "property": {
// CHECK-NEXT:                                  "type": "Identifier",
// CHECK-NEXT:                                  "name": "vy"
// CHECK-NEXT:                                },
// CHECK-NEXT:                                "computed": false
// CHECK-NEXT:                              },
// CHECK-NEXT:                              "right": {
// CHECK-NEXT:                                "type": "Identifier",
// CHECK-NEXT:                                "name": "vyi"
// CHECK-NEXT:                              }
// CHECK-NEXT:                            },
// CHECK-NEXT:                            "directive": null
// CHECK-NEXT:                          },
// CHECK-NEXT:                          {
// CHECK-NEXT:                            "type": "ExpressionStatement",
// CHECK-NEXT:                            "expression": {
// CHECK-NEXT:                              "type": "AssignmentExpression",
// CHECK-NEXT:                              "operator": "=",
// CHECK-NEXT:                              "left": {
// CHECK-NEXT:                                "type": "MemberExpression",
// CHECK-NEXT:                                "object": {
// CHECK-NEXT:                                  "type": "Identifier",
// CHECK-NEXT:                                  "name": "bodyi"
// CHECK-NEXT:                                },
// CHECK-NEXT:                                "property": {
// CHECK-NEXT:                                  "type": "Identifier",
// CHECK-NEXT:                                  "name": "vz"
// CHECK-NEXT:                                },
// CHECK-NEXT:                                "computed": false
// CHECK-NEXT:                              },
// CHECK-NEXT:                              "right": {
// CHECK-NEXT:                                "type": "Identifier",
// CHECK-NEXT:                                "name": "vzi"
// CHECK-NEXT:                              }
// CHECK-NEXT:                            },
// CHECK-NEXT:                            "directive": null
// CHECK-NEXT:                          }
// CHECK-NEXT:                        ]
// CHECK-NEXT:                      }
// CHECK-NEXT:                    },
// CHECK-NEXT:                    {
// CHECK-NEXT:                      "type": "ForStatement",
// CHECK-NEXT:                      "init": {
// CHECK-NEXT:                        "type": "VariableDeclaration",
// CHECK-NEXT:                        "kind": "let",
// CHECK-NEXT:                        "declarations": [
// CHECK-NEXT:                          {
// CHECK-NEXT:                            "type": "VariableDeclarator",
// CHECK-NEXT:                            "init": {
// CHECK-NEXT:                              "type": "NumericLiteral",
// CHECK-NEXT:                              "value": 0,
// CHECK-NEXT:                              "raw": "0"
// CHECK-NEXT:                            },
// CHECK-NEXT:                            "id": {
// CHECK-NEXT:                              "type": "Identifier",
// CHECK-NEXT:                              "name": "i",
// CHECK-NEXT:                              "typeAnnotation": {
// CHECK-NEXT:                                "type": "TypeAnnotation",
// CHECK-NEXT:                                "typeAnnotation": {
// CHECK-NEXT:                                  "type": "NumberTypeAnnotation"
// CHECK-NEXT:                                }
// CHECK-NEXT:                              }
// CHECK-NEXT:                            }
// CHECK-NEXT:                          }
// CHECK-NEXT:                        ]
// CHECK-NEXT:                      },
// CHECK-NEXT:                      "test": {
// CHECK-NEXT:                        "type": "BinaryExpression",
// CHECK-NEXT:                        "left": {
// CHECK-NEXT:                          "type": "Identifier",
// CHECK-NEXT:                          "name": "i"
// CHECK-NEXT:                        },
// CHECK-NEXT:                        "right": {
// CHECK-NEXT:                          "type": "Identifier",
// CHECK-NEXT:                          "name": "size"
// CHECK-NEXT:                        },
// CHECK-NEXT:                        "operator": "<"
// CHECK-NEXT:                      },
// CHECK-NEXT:                      "update": {
// CHECK-NEXT:                        "type": "UpdateExpression",
// CHECK-NEXT:                        "operator": "++",
// CHECK-NEXT:                        "argument": {
// CHECK-NEXT:                          "type": "Identifier",
// CHECK-NEXT:                          "name": "i"
// CHECK-NEXT:                        },
// CHECK-NEXT:                        "prefix": false
// CHECK-NEXT:                      },
// CHECK-NEXT:                      "body": {
// CHECK-NEXT:                        "type": "BlockStatement",
// CHECK-NEXT:                        "body": [
// CHECK-NEXT:                          {
// CHECK-NEXT:                            "type": "VariableDeclaration",
// CHECK-NEXT:                            "kind": "const",
// CHECK-NEXT:                            "declarations": [
// CHECK-NEXT:                              {
// CHECK-NEXT:                                "type": "VariableDeclarator",
// CHECK-NEXT:                                "init": {
// CHECK-NEXT:                                  "type": "MemberExpression",
// CHECK-NEXT:                                  "object": {
// CHECK-NEXT:                                    "type": "Identifier",
// CHECK-NEXT:                                    "name": "bodies"
// CHECK-NEXT:                                  },
// CHECK-NEXT:                                  "property": {
// CHECK-NEXT:                                    "type": "Identifier",
// CHECK-NEXT:                                    "name": "i"
// CHECK-NEXT:                                  },
// CHECK-NEXT:                                  "computed": true
// CHECK-NEXT:                                },
// CHECK-NEXT:                                "id": {
// CHECK-NEXT:                                  "type": "Identifier",
// CHECK-NEXT:                                  "name": "body",
// CHECK-NEXT:                                  "typeAnnotation": {
// CHECK-NEXT:                                    "type": "TypeAnnotation",
// CHECK-NEXT:                                    "typeAnnotation": {
// CHECK-NEXT:                                      "type": "GenericTypeAnnotation",
// CHECK-NEXT:                                      "id": {
// CHECK-NEXT:                                        "type": "Identifier",
// CHECK-NEXT:                                        "name": "Body"
// CHECK-NEXT:                                      },
// CHECK-NEXT:                                      "typeParameters": null
// CHECK-NEXT:                                    }
// CHECK-NEXT:                                  }
// CHECK-NEXT:                                }
// CHECK-NEXT:                              }
// CHECK-NEXT:                            ]
// CHECK-NEXT:                          },
// CHECK-NEXT:                          {
// CHECK-NEXT:                            "type": "ExpressionStatement",
// CHECK-NEXT:                            "expression": {
// CHECK-NEXT:                              "type": "AssignmentExpression",
// CHECK-NEXT:                              "operator": "+=",
// CHECK-NEXT:                              "left": {
// CHECK-NEXT:                                "type": "MemberExpression",
// CHECK-NEXT:                                "object": {
// CHECK-NEXT:                                  "type": "Identifier",
// CHECK-NEXT:                                  "name": "body"
// CHECK-NEXT:                                },
// CHECK-NEXT:                                "property": {
// CHECK-NEXT:                                  "type": "Identifier",
// CHECK-NEXT:                                  "name": "x"
// CHECK-NEXT:                                },
// CHECK-NEXT:                                "computed": false
// CHECK-NEXT:                              },
// CHECK-NEXT:                              "right": {
// CHECK-NEXT:                                "type": "BinaryExpression",
// CHECK-NEXT:                                "left": {
// CHECK-NEXT:                                  "type": "Identifier",
// CHECK-NEXT:                                  "name": "dt"
// CHECK-NEXT:                                },
// CHECK-NEXT:                                "right": {
// CHECK-NEXT:                                  "type": "MemberExpression",
// CHECK-NEXT:                                  "object": {
// CHECK-NEXT:                                    "type": "Identifier",
// CHECK-NEXT:                                    "name": "body"
// CHECK-NEXT:                                  },
// CHECK-NEXT:                                  "property": {
// CHECK-NEXT:                                    "type": "Identifier",
// CHECK-NEXT:                                    "name": "vx"
// CHECK-NEXT:                                  },
// CHECK-NEXT:                                  "computed": false
// CHECK-NEXT:                                },
// CHECK-NEXT:                                "operator": "*"
// CHECK-NEXT:                              }
// CHECK-NEXT:                            },
// CHECK-NEXT:                            "directive": null
// CHECK-NEXT:                          },
// CHECK-NEXT:                          {
// CHECK-NEXT:                            "type": "ExpressionStatement",
// CHECK-NEXT:                            "expression": {
// CHECK-NEXT:                              "type": "AssignmentExpression",
// CHECK-NEXT:                              "operator": "+=",
// CHECK-NEXT:                              "left": {
// CHECK-NEXT:                                "type": "MemberExpression",
// CHECK-NEXT:                                "object": {
// CHECK-NEXT:                                  "type": "Identifier",
// CHECK-NEXT:                                  "name": "body"
// CHECK-NEXT:                                },
// CHECK-NEXT:                                "property": {
// CHECK-NEXT:                                  "type": "Identifier",
// CHECK-NEXT:                                  "name": "y"
// CHECK-NEXT:                                },
// CHECK-NEXT:                                "computed": false
// CHECK-NEXT:                              },
// CHECK-NEXT:                              "right": {
// CHECK-NEXT:                                "type": "BinaryExpression",
// CHECK-NEXT:                                "left": {
// CHECK-NEXT:                                  "type": "Identifier",
// CHECK-NEXT:                                  "name": "dt"
// CHECK-NEXT:                                },
// CHECK-NEXT:                                "right": {
// CHECK-NEXT:                                  "type": "MemberExpression",
// CHECK-NEXT:                                  "object": {
// CHECK-NEXT:                                    "type": "Identifier",
// CHECK-NEXT:                                    "name": "body"
// CHECK-NEXT:                                  },
// CHECK-NEXT:                                  "property": {
// CHECK-NEXT:                                    "type": "Identifier",
// CHECK-NEXT:                                    "name": "vy"
// CHECK-NEXT:                                  },
// CHECK-NEXT:                                  "computed": false
// CHECK-NEXT:                                },
// CHECK-NEXT:                                "operator": "*"
// CHECK-NEXT:                              }
// CHECK-NEXT:                            },
// CHECK-NEXT:                            "directive": null
// CHECK-NEXT:                          },
// CHECK-NEXT:                          {
// CHECK-NEXT:                            "type": "ExpressionStatement",
// CHECK-NEXT:                            "expression": {
// CHECK-NEXT:                              "type": "AssignmentExpression",
// CHECK-NEXT:                              "operator": "+=",
// CHECK-NEXT:                              "left": {
// CHECK-NEXT:                                "type": "MemberExpression",
// CHECK-NEXT:                                "object": {
// CHECK-NEXT:                                  "type": "Identifier",
// CHECK-NEXT:                                  "name": "body"
// CHECK-NEXT:                                },
// CHECK-NEXT:                                "property": {
// CHECK-NEXT:                                  "type": "Identifier",
// CHECK-NEXT:                                  "name": "z"
// CHECK-NEXT:                                },
// CHECK-NEXT:                                "computed": false
// CHECK-NEXT:                              },
// CHECK-NEXT:                              "right": {
// CHECK-NEXT:                                "type": "BinaryExpression",
// CHECK-NEXT:                                "left": {
// CHECK-NEXT:                                  "type": "Identifier",
// CHECK-NEXT:                                  "name": "dt"
// CHECK-NEXT:                                },
// CHECK-NEXT:                                "right": {
// CHECK-NEXT:                                  "type": "MemberExpression",
// CHECK-NEXT:                                  "object": {
// CHECK-NEXT:                                    "type": "Identifier",
// CHECK-NEXT:                                    "name": "body"
// CHECK-NEXT:                                  },
// CHECK-NEXT:                                  "property": {
// CHECK-NEXT:                                    "type": "Identifier",
// CHECK-NEXT:                                    "name": "vz"
// CHECK-NEXT:                                  },
// CHECK-NEXT:                                  "computed": false
// CHECK-NEXT:                                },
// CHECK-NEXT:                                "operator": "*"
// CHECK-NEXT:                              }
// CHECK-NEXT:                            },
// CHECK-NEXT:                            "directive": null
// CHECK-NEXT:                          }
// CHECK-NEXT:                        ]
// CHECK-NEXT:                      }
// CHECK-NEXT:                    }
// CHECK-NEXT:                  ]
// CHECK-NEXT:                },
// CHECK-NEXT:                "generator": false,
// CHECK-NEXT:                "async": false
// CHECK-NEXT:              },
// CHECK-NEXT:              {
// CHECK-NEXT:                "type": "FunctionDeclaration",
// CHECK-NEXT:                "id": {
// CHECK-NEXT:                  "type": "Identifier",
// CHECK-NEXT:                  "name": "energy"
// CHECK-NEXT:                },
// CHECK-NEXT:                "params": [],
// CHECK-NEXT:                "body": {
// CHECK-NEXT:                  "type": "BlockStatement",
// CHECK-NEXT:                  "body": [
// CHECK-NEXT:                    {
// CHECK-NEXT:                      "type": "VariableDeclaration",
// CHECK-NEXT:                      "kind": "let",
// CHECK-NEXT:                      "declarations": [
// CHECK-NEXT:                        {
// CHECK-NEXT:                          "type": "VariableDeclarator",
// CHECK-NEXT:                          "init": {
// CHECK-NEXT:                            "type": "NumericLiteral",
// CHECK-NEXT:                            "value": 0,
// CHECK-NEXT:                            "raw": "0"
// CHECK-NEXT:                          },
// CHECK-NEXT:                          "id": {
// CHECK-NEXT:                            "type": "Identifier",
// CHECK-NEXT:                            "name": "e"
// CHECK-NEXT:                          }
// CHECK-NEXT:                        }
// CHECK-NEXT:                      ]
// CHECK-NEXT:                    },
// CHECK-NEXT:                    {
// CHECK-NEXT:                      "type": "VariableDeclaration",
// CHECK-NEXT:                      "kind": "const",
// CHECK-NEXT:                      "declarations": [
// CHECK-NEXT:                        {
// CHECK-NEXT:                          "type": "VariableDeclarator",
// CHECK-NEXT:                          "init": {
// CHECK-NEXT:                            "type": "MemberExpression",
// CHECK-NEXT:                            "object": {
// CHECK-NEXT:                              "type": "Identifier",
// CHECK-NEXT:                              "name": "bodies"
// CHECK-NEXT:                            },
// CHECK-NEXT:                            "property": {
// CHECK-NEXT:                              "type": "Identifier",
// CHECK-NEXT:                              "name": "length"
// CHECK-NEXT:                            },
// CHECK-NEXT:                            "computed": false
// CHECK-NEXT:                          },
// CHECK-NEXT:                          "id": {
// CHECK-NEXT:                            "type": "Identifier",
// CHECK-NEXT:                            "name": "size",
// CHECK-NEXT:                            "typeAnnotation": {
// CHECK-NEXT:                              "type": "TypeAnnotation",
// CHECK-NEXT:                              "typeAnnotation": {
// CHECK-NEXT:                                "type": "NumberTypeAnnotation"
// CHECK-NEXT:                              }
// CHECK-NEXT:                            }
// CHECK-NEXT:                          }
// CHECK-NEXT:                        }
// CHECK-NEXT:                      ]
// CHECK-NEXT:                    },
// CHECK-NEXT:                    {
// CHECK-NEXT:                      "type": "ForStatement",
// CHECK-NEXT:                      "init": {
// CHECK-NEXT:                        "type": "VariableDeclaration",
// CHECK-NEXT:                        "kind": "let",
// CHECK-NEXT:                        "declarations": [
// CHECK-NEXT:                          {
// CHECK-NEXT:                            "type": "VariableDeclarator",
// CHECK-NEXT:                            "init": {
// CHECK-NEXT:                              "type": "NumericLiteral",
// CHECK-NEXT:                              "value": 0,
// CHECK-NEXT:                              "raw": "0"
// CHECK-NEXT:                            },
// CHECK-NEXT:                            "id": {
// CHECK-NEXT:                              "type": "Identifier",
// CHECK-NEXT:                              "name": "i"
// CHECK-NEXT:                            }
// CHECK-NEXT:                          }
// CHECK-NEXT:                        ]
// CHECK-NEXT:                      },
// CHECK-NEXT:                      "test": {
// CHECK-NEXT:                        "type": "BinaryExpression",
// CHECK-NEXT:                        "left": {
// CHECK-NEXT:                          "type": "Identifier",
// CHECK-NEXT:                          "name": "i"
// CHECK-NEXT:                        },
// CHECK-NEXT:                        "right": {
// CHECK-NEXT:                          "type": "Identifier",
// CHECK-NEXT:                          "name": "size"
// CHECK-NEXT:                        },
// CHECK-NEXT:                        "operator": "<"
// CHECK-NEXT:                      },
// CHECK-NEXT:                      "update": {
// CHECK-NEXT:                        "type": "UpdateExpression",
// CHECK-NEXT:                        "operator": "++",
// CHECK-NEXT:                        "argument": {
// CHECK-NEXT:                          "type": "Identifier",
// CHECK-NEXT:                          "name": "i"
// CHECK-NEXT:                        },
// CHECK-NEXT:                        "prefix": false
// CHECK-NEXT:                      },
// CHECK-NEXT:                      "body": {
// CHECK-NEXT:                        "type": "BlockStatement",
// CHECK-NEXT:                        "body": [
// CHECK-NEXT:                          {
// CHECK-NEXT:                            "type": "VariableDeclaration",
// CHECK-NEXT:                            "kind": "const",
// CHECK-NEXT:                            "declarations": [
// CHECK-NEXT:                              {
// CHECK-NEXT:                                "type": "VariableDeclarator",
// CHECK-NEXT:                                "init": {
// CHECK-NEXT:                                  "type": "MemberExpression",
// CHECK-NEXT:                                  "object": {
// CHECK-NEXT:                                    "type": "Identifier",
// CHECK-NEXT:                                    "name": "bodies"
// CHECK-NEXT:                                  },
// CHECK-NEXT:                                  "property": {
// CHECK-NEXT:                                    "type": "Identifier",
// CHECK-NEXT:                                    "name": "i"
// CHECK-NEXT:                                  },
// CHECK-NEXT:                                  "computed": true
// CHECK-NEXT:                                },
// CHECK-NEXT:                                "id": {
// CHECK-NEXT:                                  "type": "Identifier",
// CHECK-NEXT:                                  "name": "bodyi",
// CHECK-NEXT:                                  "typeAnnotation": {
// CHECK-NEXT:                                    "type": "TypeAnnotation",
// CHECK-NEXT:                                    "typeAnnotation": {
// CHECK-NEXT:                                      "type": "GenericTypeAnnotation",
// CHECK-NEXT:                                      "id": {
// CHECK-NEXT:                                        "type": "Identifier",
// CHECK-NEXT:                                        "name": "Body"
// CHECK-NEXT:                                      },
// CHECK-NEXT:                                      "typeParameters": null
// CHECK-NEXT:                                    }
// CHECK-NEXT:                                  }
// CHECK-NEXT:                                }
// CHECK-NEXT:                              }
// CHECK-NEXT:                            ]
// CHECK-NEXT:                          },
// CHECK-NEXT:                          {
// CHECK-NEXT:                            "type": "ExpressionStatement",
// CHECK-NEXT:                            "expression": {
// CHECK-NEXT:                              "type": "AssignmentExpression",
// CHECK-NEXT:                              "operator": "+=",
// CHECK-NEXT:                              "left": {
// CHECK-NEXT:                                "type": "Identifier",
// CHECK-NEXT:                                "name": "e"
// CHECK-NEXT:                              },
// CHECK-NEXT:                              "right": {
// CHECK-NEXT:                                "type": "BinaryExpression",
// CHECK-NEXT:                                "left": {
// CHECK-NEXT:                                  "type": "BinaryExpression",
// CHECK-NEXT:                                  "left": {
// CHECK-NEXT:                                    "type": "NumericLiteral",
// CHECK-NEXT:                                    "value": 0.5,
// CHECK-NEXT:                                    "raw": "0.5"
// CHECK-NEXT:                                  },
// CHECK-NEXT:                                  "right": {
// CHECK-NEXT:                                    "type": "MemberExpression",
// CHECK-NEXT:                                    "object": {
// CHECK-NEXT:                                      "type": "Identifier",
// CHECK-NEXT:                                      "name": "bodyi"
// CHECK-NEXT:                                    },
// CHECK-NEXT:                                    "property": {
// CHECK-NEXT:                                      "type": "Identifier",
// CHECK-NEXT:                                      "name": "mass"
// CHECK-NEXT:                                    },
// CHECK-NEXT:                                    "computed": false
// CHECK-NEXT:                                  },
// CHECK-NEXT:                                  "operator": "*"
// CHECK-NEXT:                                },
// CHECK-NEXT:                                "right": {
// CHECK-NEXT:                                  "type": "BinaryExpression",
// CHECK-NEXT:                                  "left": {
// CHECK-NEXT:                                    "type": "BinaryExpression",
// CHECK-NEXT:                                    "left": {
// CHECK-NEXT:                                      "type": "BinaryExpression",
// CHECK-NEXT:                                      "left": {
// CHECK-NEXT:                                        "type": "MemberExpression",
// CHECK-NEXT:                                        "object": {
// CHECK-NEXT:                                          "type": "Identifier",
// CHECK-NEXT:                                          "name": "bodyi"
// CHECK-NEXT:                                        },
// CHECK-NEXT:                                        "property": {
// CHECK-NEXT:                                          "type": "Identifier",
// CHECK-NEXT:                                          "name": "vx"
// CHECK-NEXT:                                        },
// CHECK-NEXT:                                        "computed": false
// CHECK-NEXT:                                      },
// CHECK-NEXT:                                      "right": {
// CHECK-NEXT:                                        "type": "MemberExpression",
// CHECK-NEXT:                                        "object": {
// CHECK-NEXT:                                          "type": "Identifier",
// CHECK-NEXT:                                          "name": "bodyi"
// CHECK-NEXT:                                        },
// CHECK-NEXT:                                        "property": {
// CHECK-NEXT:                                          "type": "Identifier",
// CHECK-NEXT:                                          "name": "vx"
// CHECK-NEXT:                                        },
// CHECK-NEXT:                                        "computed": false
// CHECK-NEXT:                                      },
// CHECK-NEXT:                                      "operator": "*"
// CHECK-NEXT:                                    },
// CHECK-NEXT:                                    "right": {
// CHECK-NEXT:                                      "type": "BinaryExpression",
// CHECK-NEXT:                                      "left": {
// CHECK-NEXT:                                        "type": "MemberExpression",
// CHECK-NEXT:                                        "object": {
// CHECK-NEXT:                                          "type": "Identifier",
// CHECK-NEXT:                                          "name": "bodyi"
// CHECK-NEXT:                                        },
// CHECK-NEXT:                                        "property": {
// CHECK-NEXT:                                          "type": "Identifier",
// CHECK-NEXT:                                          "name": "vy"
// CHECK-NEXT:                                        },
// CHECK-NEXT:                                        "computed": false
// CHECK-NEXT:                                      },
// CHECK-NEXT:                                      "right": {
// CHECK-NEXT:                                        "type": "MemberExpression",
// CHECK-NEXT:                                        "object": {
// CHECK-NEXT:                                          "type": "Identifier",
// CHECK-NEXT:                                          "name": "bodyi"
// CHECK-NEXT:                                        },
// CHECK-NEXT:                                        "property": {
// CHECK-NEXT:                                          "type": "Identifier",
// CHECK-NEXT:                                          "name": "vy"
// CHECK-NEXT:                                        },
// CHECK-NEXT:                                        "computed": false
// CHECK-NEXT:                                      },
// CHECK-NEXT:                                      "operator": "*"
// CHECK-NEXT:                                    },
// CHECK-NEXT:                                    "operator": "+"
// CHECK-NEXT:                                  },
// CHECK-NEXT:                                  "right": {
// CHECK-NEXT:                                    "type": "BinaryExpression",
// CHECK-NEXT:                                    "left": {
// CHECK-NEXT:                                      "type": "MemberExpression",
// CHECK-NEXT:                                      "object": {
// CHECK-NEXT:                                        "type": "Identifier",
// CHECK-NEXT:                                        "name": "bodyi"
// CHECK-NEXT:                                      },
// CHECK-NEXT:                                      "property": {
// CHECK-NEXT:                                        "type": "Identifier",
// CHECK-NEXT:                                        "name": "vz"
// CHECK-NEXT:                                      },
// CHECK-NEXT:                                      "computed": false
// CHECK-NEXT:                                    },
// CHECK-NEXT:                                    "right": {
// CHECK-NEXT:                                      "type": "MemberExpression",
// CHECK-NEXT:                                      "object": {
// CHECK-NEXT:                                        "type": "Identifier",
// CHECK-NEXT:                                        "name": "bodyi"
// CHECK-NEXT:                                      },
// CHECK-NEXT:                                      "property": {
// CHECK-NEXT:                                        "type": "Identifier",
// CHECK-NEXT:                                        "name": "vz"
// CHECK-NEXT:                                      },
// CHECK-NEXT:                                      "computed": false
// CHECK-NEXT:                                    },
// CHECK-NEXT:                                    "operator": "*"
// CHECK-NEXT:                                  },
// CHECK-NEXT:                                  "operator": "+"
// CHECK-NEXT:                                },
// CHECK-NEXT:                                "operator": "*"
// CHECK-NEXT:                              }
// CHECK-NEXT:                            },
// CHECK-NEXT:                            "directive": null
// CHECK-NEXT:                          },
// CHECK-NEXT:                          {
// CHECK-NEXT:                            "type": "ForStatement",
// CHECK-NEXT:                            "init": {
// CHECK-NEXT:                              "type": "VariableDeclaration",
// CHECK-NEXT:                              "kind": "let",
// CHECK-NEXT:                              "declarations": [
// CHECK-NEXT:                                {
// CHECK-NEXT:                                  "type": "VariableDeclarator",
// CHECK-NEXT:                                  "init": {
// CHECK-NEXT:                                    "type": "BinaryExpression",
// CHECK-NEXT:                                    "left": {
// CHECK-NEXT:                                      "type": "Identifier",
// CHECK-NEXT:                                      "name": "i"
// CHECK-NEXT:                                    },
// CHECK-NEXT:                                    "right": {
// CHECK-NEXT:                                      "type": "NumericLiteral",
// CHECK-NEXT:                                      "value": 1,
// CHECK-NEXT:                                      "raw": "1"
// CHECK-NEXT:                                    },
// CHECK-NEXT:                                    "operator": "+"
// CHECK-NEXT:                                  },
// CHECK-NEXT:                                  "id": {
// CHECK-NEXT:                                    "type": "Identifier",
// CHECK-NEXT:                                    "name": "j"
// CHECK-NEXT:                                  }
// CHECK-NEXT:                                }
// CHECK-NEXT:                              ]
// CHECK-NEXT:                            },
// CHECK-NEXT:                            "test": {
// CHECK-NEXT:                              "type": "BinaryExpression",
// CHECK-NEXT:                              "left": {
// CHECK-NEXT:                                "type": "Identifier",
// CHECK-NEXT:                                "name": "j"
// CHECK-NEXT:                              },
// CHECK-NEXT:                              "right": {
// CHECK-NEXT:                                "type": "Identifier",
// CHECK-NEXT:                                "name": "size"
// CHECK-NEXT:                              },
// CHECK-NEXT:                              "operator": "<"
// CHECK-NEXT:                            },
// CHECK-NEXT:                            "update": {
// CHECK-NEXT:                              "type": "UpdateExpression",
// CHECK-NEXT:                              "operator": "++",
// CHECK-NEXT:                              "argument": {
// CHECK-NEXT:                                "type": "Identifier",
// CHECK-NEXT:                                "name": "j"
// CHECK-NEXT:                              },
// CHECK-NEXT:                              "prefix": false
// CHECK-NEXT:                            },
// CHECK-NEXT:                            "body": {
// CHECK-NEXT:                              "type": "BlockStatement",
// CHECK-NEXT:                              "body": [
// CHECK-NEXT:                                {
// CHECK-NEXT:                                  "type": "VariableDeclaration",
// CHECK-NEXT:                                  "kind": "const",
// CHECK-NEXT:                                  "declarations": [
// CHECK-NEXT:                                    {
// CHECK-NEXT:                                      "type": "VariableDeclarator",
// CHECK-NEXT:                                      "init": {
// CHECK-NEXT:                                        "type": "MemberExpression",
// CHECK-NEXT:                                        "object": {
// CHECK-NEXT:                                          "type": "Identifier",
// CHECK-NEXT:                                          "name": "bodies"
// CHECK-NEXT:                                        },
// CHECK-NEXT:                                        "property": {
// CHECK-NEXT:                                          "type": "Identifier",
// CHECK-NEXT:                                          "name": "j"
// CHECK-NEXT:                                        },
// CHECK-NEXT:                                        "computed": true
// CHECK-NEXT:                                      },
// CHECK-NEXT:                                      "id": {
// CHECK-NEXT:                                        "type": "Identifier",
// CHECK-NEXT:                                        "name": "bodyj",
// CHECK-NEXT:                                        "typeAnnotation": {
// CHECK-NEXT:                                          "type": "TypeAnnotation",
// CHECK-NEXT:                                          "typeAnnotation": {
// CHECK-NEXT:                                            "type": "GenericTypeAnnotation",
// CHECK-NEXT:                                            "id": {
// CHECK-NEXT:                                              "type": "Identifier",
// CHECK-NEXT:                                              "name": "Body"
// CHECK-NEXT:                                            },
// CHECK-NEXT:                                            "typeParameters": null
// CHECK-NEXT:                                          }
// CHECK-NEXT:                                        }
// CHECK-NEXT:                                      }
// CHECK-NEXT:                                    }
// CHECK-NEXT:                                  ]
// CHECK-NEXT:                                },
// CHECK-NEXT:                                {
// CHECK-NEXT:                                  "type": "VariableDeclaration",
// CHECK-NEXT:                                  "kind": "const",
// CHECK-NEXT:                                  "declarations": [
// CHECK-NEXT:                                    {
// CHECK-NEXT:                                      "type": "VariableDeclarator",
// CHECK-NEXT:                                      "init": {
// CHECK-NEXT:                                        "type": "BinaryExpression",
// CHECK-NEXT:                                        "left": {
// CHECK-NEXT:                                          "type": "MemberExpression",
// CHECK-NEXT:                                          "object": {
// CHECK-NEXT:                                            "type": "Identifier",
// CHECK-NEXT:                                            "name": "bodyi"
// CHECK-NEXT:                                          },
// CHECK-NEXT:                                          "property": {
// CHECK-NEXT:                                            "type": "Identifier",
// CHECK-NEXT:                                            "name": "x"
// CHECK-NEXT:                                          },
// CHECK-NEXT:                                          "computed": false
// CHECK-NEXT:                                        },
// CHECK-NEXT:                                        "right": {
// CHECK-NEXT:                                          "type": "MemberExpression",
// CHECK-NEXT:                                          "object": {
// CHECK-NEXT:                                            "type": "Identifier",
// CHECK-NEXT:                                            "name": "bodyj"
// CHECK-NEXT:                                          },
// CHECK-NEXT:                                          "property": {
// CHECK-NEXT:                                            "type": "Identifier",
// CHECK-NEXT:                                            "name": "x"
// CHECK-NEXT:                                          },
// CHECK-NEXT:                                          "computed": false
// CHECK-NEXT:                                        },
// CHECK-NEXT:                                        "operator": "-"
// CHECK-NEXT:                                      },
// CHECK-NEXT:                                      "id": {
// CHECK-NEXT:                                        "type": "Identifier",
// CHECK-NEXT:                                        "name": "dx",
// CHECK-NEXT:                                        "typeAnnotation": {
// CHECK-NEXT:                                          "type": "TypeAnnotation",
// CHECK-NEXT:                                          "typeAnnotation": {
// CHECK-NEXT:                                            "type": "NumberTypeAnnotation"
// CHECK-NEXT:                                          }
// CHECK-NEXT:                                        }
// CHECK-NEXT:                                      }
// CHECK-NEXT:                                    }
// CHECK-NEXT:                                  ]
// CHECK-NEXT:                                },
// CHECK-NEXT:                                {
// CHECK-NEXT:                                  "type": "VariableDeclaration",
// CHECK-NEXT:                                  "kind": "const",
// CHECK-NEXT:                                  "declarations": [
// CHECK-NEXT:                                    {
// CHECK-NEXT:                                      "type": "VariableDeclarator",
// CHECK-NEXT:                                      "init": {
// CHECK-NEXT:                                        "type": "BinaryExpression",
// CHECK-NEXT:                                        "left": {
// CHECK-NEXT:                                          "type": "MemberExpression",
// CHECK-NEXT:                                          "object": {
// CHECK-NEXT:                                            "type": "Identifier",
// CHECK-NEXT:                                            "name": "bodyi"
// CHECK-NEXT:                                          },
// CHECK-NEXT:                                          "property": {
// CHECK-NEXT:                                            "type": "Identifier",
// CHECK-NEXT:                                            "name": "y"
// CHECK-NEXT:                                          },
// CHECK-NEXT:                                          "computed": false
// CHECK-NEXT:                                        },
// CHECK-NEXT:                                        "right": {
// CHECK-NEXT:                                          "type": "MemberExpression",
// CHECK-NEXT:                                          "object": {
// CHECK-NEXT:                                            "type": "Identifier",
// CHECK-NEXT:                                            "name": "bodyj"
// CHECK-NEXT:                                          },
// CHECK-NEXT:                                          "property": {
// CHECK-NEXT:                                            "type": "Identifier",
// CHECK-NEXT:                                            "name": "y"
// CHECK-NEXT:                                          },
// CHECK-NEXT:                                          "computed": false
// CHECK-NEXT:                                        },
// CHECK-NEXT:                                        "operator": "-"
// CHECK-NEXT:                                      },
// CHECK-NEXT:                                      "id": {
// CHECK-NEXT:                                        "type": "Identifier",
// CHECK-NEXT:                                        "name": "dy",
// CHECK-NEXT:                                        "typeAnnotation": {
// CHECK-NEXT:                                          "type": "TypeAnnotation",
// CHECK-NEXT:                                          "typeAnnotation": {
// CHECK-NEXT:                                            "type": "NumberTypeAnnotation"
// CHECK-NEXT:                                          }
// CHECK-NEXT:                                        }
// CHECK-NEXT:                                      }
// CHECK-NEXT:                                    }
// CHECK-NEXT:                                  ]
// CHECK-NEXT:                                },
// CHECK-NEXT:                                {
// CHECK-NEXT:                                  "type": "VariableDeclaration",
// CHECK-NEXT:                                  "kind": "const",
// CHECK-NEXT:                                  "declarations": [
// CHECK-NEXT:                                    {
// CHECK-NEXT:                                      "type": "VariableDeclarator",
// CHECK-NEXT:                                      "init": {
// CHECK-NEXT:                                        "type": "BinaryExpression",
// CHECK-NEXT:                                        "left": {
// CHECK-NEXT:                                          "type": "MemberExpression",
// CHECK-NEXT:                                          "object": {
// CHECK-NEXT:                                            "type": "Identifier",
// CHECK-NEXT:                                            "name": "bodyi"
// CHECK-NEXT:                                          },
// CHECK-NEXT:                                          "property": {
// CHECK-NEXT:                                            "type": "Identifier",
// CHECK-NEXT:                                            "name": "z"
// CHECK-NEXT:                                          },
// CHECK-NEXT:                                          "computed": false
// CHECK-NEXT:                                        },
// CHECK-NEXT:                                        "right": {
// CHECK-NEXT:                                          "type": "MemberExpression",
// CHECK-NEXT:                                          "object": {
// CHECK-NEXT:                                            "type": "Identifier",
// CHECK-NEXT:                                            "name": "bodyj"
// CHECK-NEXT:                                          },
// CHECK-NEXT:                                          "property": {
// CHECK-NEXT:                                            "type": "Identifier",
// CHECK-NEXT:                                            "name": "z"
// CHECK-NEXT:                                          },
// CHECK-NEXT:                                          "computed": false
// CHECK-NEXT:                                        },
// CHECK-NEXT:                                        "operator": "-"
// CHECK-NEXT:                                      },
// CHECK-NEXT:                                      "id": {
// CHECK-NEXT:                                        "type": "Identifier",
// CHECK-NEXT:                                        "name": "dz",
// CHECK-NEXT:                                        "typeAnnotation": {
// CHECK-NEXT:                                          "type": "TypeAnnotation",
// CHECK-NEXT:                                          "typeAnnotation": {
// CHECK-NEXT:                                            "type": "NumberTypeAnnotation"
// CHECK-NEXT:                                          }
// CHECK-NEXT:                                        }
// CHECK-NEXT:                                      }
// CHECK-NEXT:                                    }
// CHECK-NEXT:                                  ]
// CHECK-NEXT:                                },
// CHECK-NEXT:                                {
// CHECK-NEXT:                                  "type": "VariableDeclaration",
// CHECK-NEXT:                                  "kind": "const",
// CHECK-NEXT:                                  "declarations": [
// CHECK-NEXT:                                    {
// CHECK-NEXT:                                      "type": "VariableDeclarator",
// CHECK-NEXT:                                      "init": {
// CHECK-NEXT:                                        "type": "CallExpression",
// CHECK-NEXT:                                        "callee": {
// CHECK-NEXT:                                          "type": "MemberExpression",
// CHECK-NEXT:                                          "object": {
// CHECK-NEXT:                                            "type": "Identifier",
// CHECK-NEXT:                                            "name": "Math"
// CHECK-NEXT:                                          },
// CHECK-NEXT:                                          "property": {
// CHECK-NEXT:                                            "type": "Identifier",
// CHECK-NEXT:                                            "name": "sqrt"
// CHECK-NEXT:                                          },
// CHECK-NEXT:                                          "computed": false
// CHECK-NEXT:                                        },
// CHECK-NEXT:                                        "arguments": [
// CHECK-NEXT:                                          {
// CHECK-NEXT:                                            "type": "BinaryExpression",
// CHECK-NEXT:                                            "left": {
// CHECK-NEXT:                                              "type": "BinaryExpression",
// CHECK-NEXT:                                              "left": {
// CHECK-NEXT:                                                "type": "BinaryExpression",
// CHECK-NEXT:                                                "left": {
// CHECK-NEXT:                                                  "type": "Identifier",
// CHECK-NEXT:                                                  "name": "dx"
// CHECK-NEXT:                                                },
// CHECK-NEXT:                                                "right": {
// CHECK-NEXT:                                                  "type": "Identifier",
// CHECK-NEXT:                                                  "name": "dx"
// CHECK-NEXT:                                                },
// CHECK-NEXT:                                                "operator": "*"
// CHECK-NEXT:                                              },
// CHECK-NEXT:                                              "right": {
// CHECK-NEXT:                                                "type": "BinaryExpression",
// CHECK-NEXT:                                                "left": {
// CHECK-NEXT:                                                  "type": "Identifier",
// CHECK-NEXT:                                                  "name": "dy"
// CHECK-NEXT:                                                },
// CHECK-NEXT:                                                "right": {
// CHECK-NEXT:                                                  "type": "Identifier",
// CHECK-NEXT:                                                  "name": "dy"
// CHECK-NEXT:                                                },
// CHECK-NEXT:                                                "operator": "*"
// CHECK-NEXT:                                              },
// CHECK-NEXT:                                              "operator": "+"
// CHECK-NEXT:                                            },
// CHECK-NEXT:                                            "right": {
// CHECK-NEXT:                                              "type": "BinaryExpression",
// CHECK-NEXT:                                              "left": {
// CHECK-NEXT:                                                "type": "Identifier",
// CHECK-NEXT:                                                "name": "dz"
// CHECK-NEXT:                                              },
// CHECK-NEXT:                                              "right": {
// CHECK-NEXT:                                                "type": "Identifier",
// CHECK-NEXT:                                                "name": "dz"
// CHECK-NEXT:                                              },
// CHECK-NEXT:                                              "operator": "*"
// CHECK-NEXT:                                            },
// CHECK-NEXT:                                            "operator": "+"
// CHECK-NEXT:                                          }
// CHECK-NEXT:                                        ]
// CHECK-NEXT:                                      },
// CHECK-NEXT:                                      "id": {
// CHECK-NEXT:                                        "type": "Identifier",
// CHECK-NEXT:                                        "name": "distance",
// CHECK-NEXT:                                        "typeAnnotation": {
// CHECK-NEXT:                                          "type": "TypeAnnotation",
// CHECK-NEXT:                                          "typeAnnotation": {
// CHECK-NEXT:                                            "type": "NumberTypeAnnotation"
// CHECK-NEXT:                                          }
// CHECK-NEXT:                                        }
// CHECK-NEXT:                                      }
// CHECK-NEXT:                                    }
// CHECK-NEXT:                                  ]
// CHECK-NEXT:                                },
// CHECK-NEXT:                                {
// CHECK-NEXT:                                  "type": "ExpressionStatement",
// CHECK-NEXT:                                  "expression": {
// CHECK-NEXT:                                    "type": "AssignmentExpression",
// CHECK-NEXT:                                    "operator": "-=",
// CHECK-NEXT:                                    "left": {
// CHECK-NEXT:                                      "type": "Identifier",
// CHECK-NEXT:                                      "name": "e"
// CHECK-NEXT:                                    },
// CHECK-NEXT:                                    "right": {
// CHECK-NEXT:                                      "type": "BinaryExpression",
// CHECK-NEXT:                                      "left": {
// CHECK-NEXT:                                        "type": "BinaryExpression",
// CHECK-NEXT:                                        "left": {
// CHECK-NEXT:                                          "type": "MemberExpression",
// CHECK-NEXT:                                          "object": {
// CHECK-NEXT:                                            "type": "Identifier",
// CHECK-NEXT:                                            "name": "bodyi"
// CHECK-NEXT:                                          },
// CHECK-NEXT:                                          "property": {
// CHECK-NEXT:                                            "type": "Identifier",
// CHECK-NEXT:                                            "name": "mass"
// CHECK-NEXT:                                          },
// CHECK-NEXT:                                          "computed": false
// CHECK-NEXT:                                        },
// CHECK-NEXT:                                        "right": {
// CHECK-NEXT:                                          "type": "MemberExpression",
// CHECK-NEXT:                                          "object": {
// CHECK-NEXT:                                            "type": "Identifier",
// CHECK-NEXT:                                            "name": "bodyj"
// CHECK-NEXT:                                          },
// CHECK-NEXT:                                          "property": {
// CHECK-NEXT:                                            "type": "Identifier",
// CHECK-NEXT:                                            "name": "mass"
// CHECK-NEXT:                                          },
// CHECK-NEXT:                                          "computed": false
// CHECK-NEXT:                                        },
// CHECK-NEXT:                                        "operator": "*"
// CHECK-NEXT:                                      },
// CHECK-NEXT:                                      "right": {
// CHECK-NEXT:                                        "type": "Identifier",
// CHECK-NEXT:                                        "name": "distance"
// CHECK-NEXT:                                      },
// CHECK-NEXT:                                      "operator": "\/"
// CHECK-NEXT:                                    }
// CHECK-NEXT:                                  },
// CHECK-NEXT:                                  "directive": null
// CHECK-NEXT:                                }
// CHECK-NEXT:                              ]
// CHECK-NEXT:                            }
// CHECK-NEXT:                          }
// CHECK-NEXT:                        ]
// CHECK-NEXT:                      }
// CHECK-NEXT:                    },
// CHECK-NEXT:                    {
// CHECK-NEXT:                      "type": "ReturnStatement",
// CHECK-NEXT:                      "argument": {
// CHECK-NEXT:                        "type": "Identifier",
// CHECK-NEXT:                        "name": "e"
// CHECK-NEXT:                      }
// CHECK-NEXT:                    }
// CHECK-NEXT:                  ]
// CHECK-NEXT:                },
// CHECK-NEXT:                "returnType": {
// CHECK-NEXT:                  "type": "TypeAnnotation",
// CHECK-NEXT:                  "typeAnnotation": {
// CHECK-NEXT:                    "type": "NumberTypeAnnotation"
// CHECK-NEXT:                  }
// CHECK-NEXT:                },
// CHECK-NEXT:                "generator": false,
// CHECK-NEXT:                "async": false
// CHECK-NEXT:              },
// CHECK-NEXT:              {
// CHECK-NEXT:                "type": "FunctionDeclaration",
// CHECK-NEXT:                "id": {
// CHECK-NEXT:                  "type": "Identifier",
// CHECK-NEXT:                  "name": "nbody"
// CHECK-NEXT:                },
// CHECK-NEXT:                "params": [],
// CHECK-NEXT:                "body": {
// CHECK-NEXT:                  "type": "BlockStatement",
// CHECK-NEXT:                  "body": [
// CHECK-NEXT:                    {
// CHECK-NEXT:                      "type": "VariableDeclaration",
// CHECK-NEXT:                      "kind": "const",
// CHECK-NEXT:                      "declarations": [
// CHECK-NEXT:                        {
// CHECK-NEXT:                          "type": "VariableDeclarator",
// CHECK-NEXT:                          "init": {
// CHECK-NEXT:                            "type": "NumericLiteral",
// CHECK-NEXT:                            "value": 400000,
// CHECK-NEXT:                            "raw": "400_000"
// CHECK-NEXT:                          },
// CHECK-NEXT:                          "id": {
// CHECK-NEXT:                            "type": "Identifier",
// CHECK-NEXT:                            "name": "n"
// CHECK-NEXT:                          }
// CHECK-NEXT:                        }
// CHECK-NEXT:                      ]
// CHECK-NEXT:                    },
// CHECK-NEXT:                    {
// CHECK-NEXT:                      "type": "ExpressionStatement",
// CHECK-NEXT:                      "expression": {
// CHECK-NEXT:                        "type": "CallExpression",
// CHECK-NEXT:                        "callee": {
// CHECK-NEXT:                          "type": "Identifier",
// CHECK-NEXT:                          "name": "offsetMomentum"
// CHECK-NEXT:                        },
// CHECK-NEXT:                        "arguments": []
// CHECK-NEXT:                      },
// CHECK-NEXT:                      "directive": null
// CHECK-NEXT:                    },
// CHECK-NEXT:                    {
// CHECK-NEXT:                      "type": "ForStatement",
// CHECK-NEXT:                      "init": {
// CHECK-NEXT:                        "type": "VariableDeclaration",
// CHECK-NEXT:                        "kind": "let",
// CHECK-NEXT:                        "declarations": [
// CHECK-NEXT:                          {
// CHECK-NEXT:                            "type": "VariableDeclarator",
// CHECK-NEXT:                            "init": {
// CHECK-NEXT:                              "type": "NumericLiteral",
// CHECK-NEXT:                              "value": 0,
// CHECK-NEXT:                              "raw": "0"
// CHECK-NEXT:                            },
// CHECK-NEXT:                            "id": {
// CHECK-NEXT:                              "type": "Identifier",
// CHECK-NEXT:                              "name": "i"
// CHECK-NEXT:                            }
// CHECK-NEXT:                          }
// CHECK-NEXT:                        ]
// CHECK-NEXT:                      },
// CHECK-NEXT:                      "test": {
// CHECK-NEXT:                        "type": "BinaryExpression",
// CHECK-NEXT:                        "left": {
// CHECK-NEXT:                          "type": "Identifier",
// CHECK-NEXT:                          "name": "i"
// CHECK-NEXT:                        },
// CHECK-NEXT:                        "right": {
// CHECK-NEXT:                          "type": "Identifier",
// CHECK-NEXT:                          "name": "n"
// CHECK-NEXT:                        },
// CHECK-NEXT:                        "operator": "<"
// CHECK-NEXT:                      },
// CHECK-NEXT:                      "update": {
// CHECK-NEXT:                        "type": "UpdateExpression",
// CHECK-NEXT:                        "operator": "++",
// CHECK-NEXT:                        "argument": {
// CHECK-NEXT:                          "type": "Identifier",
// CHECK-NEXT:                          "name": "i"
// CHECK-NEXT:                        },
// CHECK-NEXT:                        "prefix": false
// CHECK-NEXT:                      },
// CHECK-NEXT:                      "body": {
// CHECK-NEXT:                        "type": "BlockStatement",
// CHECK-NEXT:                        "body": [
// CHECK-NEXT:                          {
// CHECK-NEXT:                            "type": "ExpressionStatement",
// CHECK-NEXT:                            "expression": {
// CHECK-NEXT:                              "type": "CallExpression",
// CHECK-NEXT:                              "callee": {
// CHECK-NEXT:                                "type": "Identifier",
// CHECK-NEXT:                                "name": "advance"
// CHECK-NEXT:                              },
// CHECK-NEXT:                              "arguments": [
// CHECK-NEXT:                                {
// CHECK-NEXT:                                  "type": "NumericLiteral",
// CHECK-NEXT:                                  "value": 0.01,
// CHECK-NEXT:                                  "raw": "0.01"
// CHECK-NEXT:                                }
// CHECK-NEXT:                              ]
// CHECK-NEXT:                            },
// CHECK-NEXT:                            "directive": null
// CHECK-NEXT:                          }
// CHECK-NEXT:                        ]
// CHECK-NEXT:                      }
// CHECK-NEXT:                    },
// CHECK-NEXT:                    {
// CHECK-NEXT:                      "type": "ReturnStatement",
// CHECK-NEXT:                      "argument": {
// CHECK-NEXT:                        "type": "CallExpression",
// CHECK-NEXT:                        "callee": {
// CHECK-NEXT:                          "type": "Identifier",
// CHECK-NEXT:                          "name": "energy"
// CHECK-NEXT:                        },
// CHECK-NEXT:                        "arguments": []
// CHECK-NEXT:                      }
// CHECK-NEXT:                    }
// CHECK-NEXT:                  ]
// CHECK-NEXT:                },
// CHECK-NEXT:                "returnType": {
// CHECK-NEXT:                  "type": "TypeAnnotation",
// CHECK-NEXT:                  "typeAnnotation": {
// CHECK-NEXT:                    "type": "NumberTypeAnnotation"
// CHECK-NEXT:                  }
// CHECK-NEXT:                },
// CHECK-NEXT:                "generator": false,
// CHECK-NEXT:                "async": false
// CHECK-NEXT:              },
// CHECK-NEXT:              {
// CHECK-NEXT:                "type": "ExpressionStatement",
// CHECK-NEXT:                "expression": {
// CHECK-NEXT:                  "type": "CallExpression",
// CHECK-NEXT:                  "callee": {
// CHECK-NEXT:                    "type": "Identifier",
// CHECK-NEXT:                    "name": "print"
// CHECK-NEXT:                  },
// CHECK-NEXT:                  "arguments": [
// CHECK-NEXT:                    {
// CHECK-NEXT:                      "type": "CallExpression",
// CHECK-NEXT:                      "callee": {
// CHECK-NEXT:                        "type": "Identifier",
// CHECK-NEXT:                        "name": "nbody"
// CHECK-NEXT:                      },
// CHECK-NEXT:                      "arguments": []
// CHECK-NEXT:                    }
// CHECK-NEXT:                  ]
// CHECK-NEXT:                },
// CHECK-NEXT:                "directive": null
// CHECK-NEXT:              }
// CHECK-NEXT:            ]
// CHECK-NEXT:          },
// CHECK-NEXT:          "generator": false,
// CHECK-NEXT:          "async": false
// CHECK-NEXT:        },
// CHECK-NEXT:        "arguments": []
// CHECK-NEXT:      },
// CHECK-NEXT:      "directive": null
// CHECK-NEXT:    }
// CHECK-NEXT:  ]
// CHECK-NEXT:}
