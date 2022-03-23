/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -parse-flow -dump-ast -dump-source-location=loc -pretty-json %s | %FileCheck %s --match-full-lines

// CHECK-LABEL: {
// CHECK-NEXT:   "type": "Program",
// CHECK-NEXT:   "body": [

declare export default any

declare export default any;

declare export * from 'other'

declare export * from 'other';

declare export var x

declare export var x: any

declare export var x;

declare export default function foo(): any;

declare export default class C {}

// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "DeclareExportDeclaration",
// CHECK-NEXT:       "declaration": {
// CHECK-NEXT:         "type": "AnyTypeAnnotation",
// CHECK-NEXT:         "loc": {
// CHECK-NEXT:           "start": {
// CHECK-NEXT:             "line": 14,
// CHECK-NEXT:             "column": 24
// CHECK-NEXT:           },
// CHECK-NEXT:           "end": {
// CHECK-NEXT:             "line": 14,
// CHECK-NEXT:             "column": 27
// CHECK-NEXT:           }
// CHECK-NEXT:         }
// CHECK-NEXT:       },
// CHECK-NEXT:       "specifiers": [],
// CHECK-NEXT:       "source": null,
// CHECK-NEXT:       "default": true,
// CHECK-NEXT:       "loc": {
// CHECK-NEXT:         "start": {
// CHECK-NEXT:           "line": 14,
// CHECK-NEXT:           "column": 1
// CHECK-NEXT:         },
// CHECK-NEXT:         "end": {
// CHECK-NEXT:           "line": 14,
// CHECK-NEXT:           "column": 27
// CHECK-NEXT:         }
// CHECK-NEXT:       }
// CHECK-NEXT:     },

// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "DeclareExportDeclaration",
// CHECK-NEXT:       "declaration": {
// CHECK-NEXT:         "type": "AnyTypeAnnotation",
// CHECK-NEXT:         "loc": {
// CHECK-NEXT:           "start": {
// CHECK-NEXT:             "line": 16,
// CHECK-NEXT:             "column": 24
// CHECK-NEXT:           },
// CHECK-NEXT:           "end": {
// CHECK-NEXT:             "line": 16,
// CHECK-NEXT:             "column": 27
// CHECK-NEXT:           }
// CHECK-NEXT:         }
// CHECK-NEXT:       },
// CHECK-NEXT:       "specifiers": [],
// CHECK-NEXT:       "source": null,
// CHECK-NEXT:       "default": true,
// CHECK-NEXT:       "loc": {
// CHECK-NEXT:         "start": {
// CHECK-NEXT:           "line": 16,
// CHECK-NEXT:           "column": 1
// CHECK-NEXT:         },
// CHECK-NEXT:         "end": {
// CHECK-NEXT:           "line": 16,
// CHECK-NEXT:           "column": 28
// CHECK-NEXT:         }
// CHECK-NEXT:       }
// CHECK-NEXT:     },

// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "DeclareExportAllDeclaration",
// CHECK-NEXT:       "source": {
// CHECK-NEXT:         "type": "StringLiteral",
// CHECK-NEXT:         "value": "other",
// CHECK-NEXT:         "loc": {
// CHECK-NEXT:           "start": {
// CHECK-NEXT:             "line": 18,
// CHECK-NEXT:             "column": 23
// CHECK-NEXT:           },
// CHECK-NEXT:           "end": {
// CHECK-NEXT:             "line": 18,
// CHECK-NEXT:             "column": 30
// CHECK-NEXT:           }
// CHECK-NEXT:         }
// CHECK-NEXT:       },
// CHECK-NEXT:       "loc": {
// CHECK-NEXT:         "start": {
// CHECK-NEXT:           "line": 18,
// CHECK-NEXT:           "column": 1
// CHECK-NEXT:         },
// CHECK-NEXT:         "end": {
// CHECK-NEXT:           "line": 18,
// CHECK-NEXT:           "column": 30
// CHECK-NEXT:         }
// CHECK-NEXT:       }
// CHECK-NEXT:     },

// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "DeclareExportAllDeclaration",
// CHECK-NEXT:       "source": {
// CHECK-NEXT:         "type": "StringLiteral",
// CHECK-NEXT:         "value": "other",
// CHECK-NEXT:         "loc": {
// CHECK-NEXT:           "start": {
// CHECK-NEXT:             "line": 20,
// CHECK-NEXT:             "column": 23
// CHECK-NEXT:           },
// CHECK-NEXT:           "end": {
// CHECK-NEXT:             "line": 20,
// CHECK-NEXT:             "column": 30
// CHECK-NEXT:           }
// CHECK-NEXT:         }
// CHECK-NEXT:       },
// CHECK-NEXT:       "loc": {
// CHECK-NEXT:         "start": {
// CHECK-NEXT:           "line": 20,
// CHECK-NEXT:           "column": 1
// CHECK-NEXT:         },
// CHECK-NEXT:         "end": {
// CHECK-NEXT:           "line": 20,
// CHECK-NEXT:           "column": 31
// CHECK-NEXT:         }
// CHECK-NEXT:       }
// CHECK-NEXT:     },

// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "DeclareExportDeclaration",
// CHECK-NEXT:       "declaration": {
// CHECK-NEXT:         "type": "DeclareVariable",
// CHECK-NEXT:         "id": {
// CHECK-NEXT:           "type": "Identifier",
// CHECK-NEXT:           "name": "x",
// CHECK-NEXT:           "loc": {
// CHECK-NEXT:             "start": {
// CHECK-NEXT:               "line": 22,
// CHECK-NEXT:               "column": 20
// CHECK-NEXT:             },
// CHECK-NEXT:             "end": {
// CHECK-NEXT:               "line": 22,
// CHECK-NEXT:               "column": 21
// CHECK-NEXT:             }
// CHECK-NEXT:           }
// CHECK-NEXT:         },
// CHECK-NEXT:         "loc": {
// CHECK-NEXT:           "start": {
// CHECK-NEXT:             "line": 22,
// CHECK-NEXT:             "column": 16
// CHECK-NEXT:           },
// CHECK-NEXT:           "end": {
// CHECK-NEXT:             "line": 22,
// CHECK-NEXT:             "column": 21
// CHECK-NEXT:           }
// CHECK-NEXT:         }
// CHECK-NEXT:       },
// CHECK-NEXT:       "specifiers": [],
// CHECK-NEXT:       "source": null,
// CHECK-NEXT:       "default": false,
// CHECK-NEXT:       "loc": {
// CHECK-NEXT:         "start": {
// CHECK-NEXT:           "line": 22,
// CHECK-NEXT:           "column": 1
// CHECK-NEXT:         },
// CHECK-NEXT:         "end": {
// CHECK-NEXT:           "line": 22,
// CHECK-NEXT:           "column": 21
// CHECK-NEXT:         }
// CHECK-NEXT:       }
// CHECK-NEXT:     },

// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "DeclareExportDeclaration",
// CHECK-NEXT:       "declaration": {
// CHECK-NEXT:         "type": "DeclareVariable",
// CHECK-NEXT:         "id": {
// CHECK-NEXT:           "type": "Identifier",
// CHECK-NEXT:           "name": "x",
// CHECK-NEXT:           "typeAnnotation": {
// CHECK-NEXT:             "type": "TypeAnnotation",
// CHECK-NEXT:             "typeAnnotation": {
// CHECK-NEXT:               "type": "AnyTypeAnnotation",
// CHECK-NEXT:               "loc": {
// CHECK-NEXT:                 "start": {
// CHECK-NEXT:                   "line": 24,
// CHECK-NEXT:                   "column": 23
// CHECK-NEXT:                 },
// CHECK-NEXT:                 "end": {
// CHECK-NEXT:                   "line": 24,
// CHECK-NEXT:                   "column": 26
// CHECK-NEXT:                 }
// CHECK-NEXT:               }
// CHECK-NEXT:             },
// CHECK-NEXT:             "loc": {
// CHECK-NEXT:               "start": {
// CHECK-NEXT:                 "line": 24,
// CHECK-NEXT:                 "column": 21
// CHECK-NEXT:               },
// CHECK-NEXT:               "end": {
// CHECK-NEXT:                 "line": 24,
// CHECK-NEXT:                 "column": 26
// CHECK-NEXT:               }
// CHECK-NEXT:             }
// CHECK-NEXT:           },
// CHECK-NEXT:           "loc": {
// CHECK-NEXT:             "start": {
// CHECK-NEXT:               "line": 24,
// CHECK-NEXT:               "column": 20
// CHECK-NEXT:             },
// CHECK-NEXT:             "end": {
// CHECK-NEXT:               "line": 24,
// CHECK-NEXT:               "column": 26
// CHECK-NEXT:             }
// CHECK-NEXT:           }
// CHECK-NEXT:         },
// CHECK-NEXT:         "loc": {
// CHECK-NEXT:           "start": {
// CHECK-NEXT:             "line": 24,
// CHECK-NEXT:             "column": 16
// CHECK-NEXT:           },
// CHECK-NEXT:           "end": {
// CHECK-NEXT:             "line": 24,
// CHECK-NEXT:             "column": 26
// CHECK-NEXT:           }
// CHECK-NEXT:         }
// CHECK-NEXT:       },
// CHECK-NEXT:       "specifiers": [],
// CHECK-NEXT:       "source": null,
// CHECK-NEXT:       "default": false,
// CHECK-NEXT:       "loc": {
// CHECK-NEXT:         "start": {
// CHECK-NEXT:           "line": 24,
// CHECK-NEXT:           "column": 1
// CHECK-NEXT:         },
// CHECK-NEXT:         "end": {
// CHECK-NEXT:           "line": 24,
// CHECK-NEXT:           "column": 26
// CHECK-NEXT:         }
// CHECK-NEXT:       }
// CHECK-NEXT:     },

// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "DeclareExportDeclaration",
// CHECK-NEXT:       "declaration": {
// CHECK-NEXT:         "type": "DeclareVariable",
// CHECK-NEXT:         "id": {
// CHECK-NEXT:           "type": "Identifier",
// CHECK-NEXT:           "name": "x",
// CHECK-NEXT:           "loc": {
// CHECK-NEXT:             "start": {
// CHECK-NEXT:               "line": 26,
// CHECK-NEXT:               "column": 20
// CHECK-NEXT:             },
// CHECK-NEXT:             "end": {
// CHECK-NEXT:               "line": 26,
// CHECK-NEXT:               "column": 21
// CHECK-NEXT:             }
// CHECK-NEXT:           }
// CHECK-NEXT:         },
// CHECK-NEXT:         "loc": {
// CHECK-NEXT:           "start": {
// CHECK-NEXT:             "line": 26,
// CHECK-NEXT:             "column": 16
// CHECK-NEXT:           },
// CHECK-NEXT:           "end": {
// CHECK-NEXT:             "line": 26,
// CHECK-NEXT:             "column": 22
// CHECK-NEXT:           }
// CHECK-NEXT:         }
// CHECK-NEXT:       },
// CHECK-NEXT:       "specifiers": [],
// CHECK-NEXT:       "source": null,
// CHECK-NEXT:       "default": false,
// CHECK-NEXT:       "loc": {
// CHECK-NEXT:         "start": {
// CHECK-NEXT:           "line": 26,
// CHECK-NEXT:           "column": 1
// CHECK-NEXT:         },
// CHECK-NEXT:         "end": {
// CHECK-NEXT:           "line": 26,
// CHECK-NEXT:           "column": 22
// CHECK-NEXT:         }
// CHECK-NEXT:       }
// CHECK-NEXT:     },

// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "DeclareExportDeclaration",
// CHECK-NEXT:       "declaration": {
// CHECK-NEXT:         "type": "DeclareFunction",
// CHECK-NEXT:         "id": {
// CHECK-NEXT:           "type": "Identifier",
// CHECK-NEXT:           "name": "foo",
// CHECK-NEXT:           "typeAnnotation": {
// CHECK-NEXT:             "type": "TypeAnnotation",
// CHECK-NEXT:             "typeAnnotation": {
// CHECK-NEXT:               "type": "FunctionTypeAnnotation",
// CHECK-NEXT:               "params": [],
// CHECK-NEXT:               "this": null,
// CHECK-NEXT:               "returnType": {
// CHECK-NEXT:                 "type": "AnyTypeAnnotation",
// CHECK-NEXT:                 "loc": {
// CHECK-NEXT:                   "start": {
// CHECK-NEXT:                     "line": 28,
// CHECK-NEXT:                     "column": 40
// CHECK-NEXT:                   },
// CHECK-NEXT:                   "end": {
// CHECK-NEXT:                     "line": 28,
// CHECK-NEXT:                     "column": 43
// CHECK-NEXT:                   }
// CHECK-NEXT:                 }
// CHECK-NEXT:               },
// CHECK-NEXT:               "rest": null,
// CHECK-NEXT:               "typeParameters": null,
// CHECK-NEXT:               "loc": {
// CHECK-NEXT:                 "start": {
// CHECK-NEXT:                   "line": 28,
// CHECK-NEXT:                   "column": 36
// CHECK-NEXT:                 },
// CHECK-NEXT:                 "end": {
// CHECK-NEXT:                   "line": 28,
// CHECK-NEXT:                   "column": 43
// CHECK-NEXT:                 }
// CHECK-NEXT:               }
// CHECK-NEXT:             },
// CHECK-NEXT:             "loc": {
// CHECK-NEXT:               "start": {
// CHECK-NEXT:                 "line": 28,
// CHECK-NEXT:                 "column": 36
// CHECK-NEXT:               },
// CHECK-NEXT:               "end": {
// CHECK-NEXT:                 "line": 28,
// CHECK-NEXT:                 "column": 43
// CHECK-NEXT:               }
// CHECK-NEXT:             }
// CHECK-NEXT:           },
// CHECK-NEXT:           "loc": {
// CHECK-NEXT:             "start": {
// CHECK-NEXT:               "line": 28,
// CHECK-NEXT:               "column": 33
// CHECK-NEXT:             },
// CHECK-NEXT:             "end": {
// CHECK-NEXT:               "line": 28,
// CHECK-NEXT:               "column": 43
// CHECK-NEXT:             }
// CHECK-NEXT:           }
// CHECK-NEXT:         },
// CHECK-NEXT:         "predicate": null,
// CHECK-NEXT:         "loc": {
// CHECK-NEXT:           "start": {
// CHECK-NEXT:             "line": 28,
// CHECK-NEXT:             "column": 24
// CHECK-NEXT:           },
// CHECK-NEXT:           "end": {
// CHECK-NEXT:             "line": 28,
// CHECK-NEXT:             "column": 44
// CHECK-NEXT:           }
// CHECK-NEXT:         }
// CHECK-NEXT:       },
// CHECK-NEXT:       "specifiers": [],
// CHECK-NEXT:       "source": null,
// CHECK-NEXT:       "default": true,
// CHECK-NEXT:       "loc": {
// CHECK-NEXT:         "start": {
// CHECK-NEXT:           "line": 28,
// CHECK-NEXT:           "column": 1
// CHECK-NEXT:         },
// CHECK-NEXT:         "end": {
// CHECK-NEXT:           "line": 28,
// CHECK-NEXT:           "column": 44
// CHECK-NEXT:         }
// CHECK-NEXT:       }
// CHECK-NEXT:     },

// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "DeclareExportDeclaration",
// CHECK-NEXT:       "declaration": {
// CHECK-NEXT:         "type": "DeclareClass",
// CHECK-NEXT:         "id": {
// CHECK-NEXT:           "type": "Identifier",
// CHECK-NEXT:           "name": "C",
// CHECK-NEXT:           "loc": {
// CHECK-NEXT:             "start": {
// CHECK-NEXT:               "line": 30,
// CHECK-NEXT:               "column": 30
// CHECK-NEXT:             },
// CHECK-NEXT:             "end": {
// CHECK-NEXT:               "line": 30,
// CHECK-NEXT:               "column": 31
// CHECK-NEXT:             }
// CHECK-NEXT:           }
// CHECK-NEXT:         },
// CHECK-NEXT:         "typeParameters": null,
// CHECK-NEXT:         "extends": [],
// CHECK-NEXT:         "implements": [],
// CHECK-NEXT:         "mixins": [],
// CHECK-NEXT:         "body": {
// CHECK-NEXT:           "type": "ObjectTypeAnnotation",
// CHECK-NEXT:           "properties": [],
// CHECK-NEXT:           "indexers": [],
// CHECK-NEXT:           "callProperties": [],
// CHECK-NEXT:           "internalSlots": [],
// CHECK-NEXT:           "inexact": false,
// CHECK-NEXT:           "exact": false,
// CHECK-NEXT:           "loc": {
// CHECK-NEXT:             "start": {
// CHECK-NEXT:               "line": 30,
// CHECK-NEXT:               "column": 32
// CHECK-NEXT:             },
// CHECK-NEXT:             "end": {
// CHECK-NEXT:               "line": 30,
// CHECK-NEXT:               "column": 34
// CHECK-NEXT:             }
// CHECK-NEXT:           }
// CHECK-NEXT:         },
// CHECK-NEXT:         "loc": {
// CHECK-NEXT:           "start": {
// CHECK-NEXT:             "line": 30,
// CHECK-NEXT:             "column": 24
// CHECK-NEXT:           },
// CHECK-NEXT:           "end": {
// CHECK-NEXT:             "line": 30,
// CHECK-NEXT:             "column": 34
// CHECK-NEXT:           }
// CHECK-NEXT:         }
// CHECK-NEXT:       },
// CHECK-NEXT:       "specifiers": [],
// CHECK-NEXT:       "source": null,
// CHECK-NEXT:       "default": true,
// CHECK-NEXT:       "loc": {
// CHECK-NEXT:         "start": {
// CHECK-NEXT:           "line": 30,
// CHECK-NEXT:           "column": 1
// CHECK-NEXT:         },
// CHECK-NEXT:         "end": {
// CHECK-NEXT:           "line": 30,
// CHECK-NEXT:           "column": 34
// CHECK-NEXT:         }
// CHECK-NEXT:       }
// CHECK-NEXT:     }

// CHECK-NEXT:   ],
// CHECK-NEXT:   "loc": {
// CHECK-NEXT:     "start": {
// CHECK-NEXT:       "line": 14,
// CHECK-NEXT:       "column": 1
// CHECK-NEXT:     },
// CHECK-NEXT:     "end": {
// CHECK-NEXT:       "line": 30,
// CHECK-NEXT:       "column": 34
// CHECK-NEXT:     }
// CHECK-NEXT:   }
// CHECK-NEXT: }
