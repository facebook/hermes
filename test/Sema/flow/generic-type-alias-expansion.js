/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -Werror --typed --dump-sema -fno-std-globals %s | %FileCheck %s

// A long chain of distinct aliases is not runaway expansion.
type Chain0<T> = Chain1<T>;
type Chain1<T> = Chain2<T>;
type Chain2<T> = Chain3<T>;
type Chain3<T> = Chain4<T>;
type Chain4<T> = Chain5<T>;
type Chain5<T> = Chain6<T>;
type Chain6<T> = Chain7<T>;
type Chain7<T> = Chain8<T>;
type Chain8<T> = Chain9<T>;
type Chain9<T> = Chain10<T>;
type Chain10<T> = Chain11<T>;
type Chain11<T> = Chain12<T>;
type Chain12<T> = Chain13<T>;
type Chain13<T> = Chain14<T>;
type Chain14<T> = Chain15<T>;
type Chain15<T> = Chain16<T>;
type Chain16<T> = Chain17<T>;
type Chain17<T> = Chain18<T>;
type Chain18<T> = Chain19<T>;
type Chain19<T> = Chain20<T>;
type Chain20<T> = Chain21<T>;
type Chain21<T> = Chain22<T>;
type Chain22<T> = Chain23<T>;
type Chain23<T> = Chain24<T>;
type Chain24<T> = Chain25<T>;
type Chain25<T> = Chain26<T>;
type Chain26<T> = Chain27<T>;
type Chain27<T> = Chain28<T>;
type Chain28<T> = Chain29<T>;
type Chain29<T> = Chain30<T>;
type Chain30<T> = Chain31<T>;
type Chain31<T> = Chain32<T>;
type Chain32<T> = Chain33<T>;
type Chain33<T> = Chain34<T>;
type Chain34<T> = Chain35<T>;
type Chain35<T> = Chain36<T>;
type Chain36<T> = Chain37<T>;
type Chain37<T> = Chain38<T>;
type Chain38<T> = Chain39<T>;
type Chain39<T> = Chain40<T>;
type Chain40<T> = [T];
let chain: Chain0<number>;

// Repeated expansions with unresolved arguments reuse the first expansion.
type Pair<X, Y> = [X, Y];
type Fan<T> = Pair<Fan1<T>, Fan1<T>>;
type Fan1<T> = Pair<Fan2<T>, Fan2<T>>;
type Fan2<T> = Pair<Fan3<T>, Fan3<T>>;
type Fan3<T> = Pair<Fan4<T>, Fan4<T>>;
type Fan4<T> = Pair<Fan5<T>, Fan5<T>>;
type Fan5<T> = Pair<Fan6<T>, Fan6<T>>;
type Fan6<T> = Pair<Fan7<T>, Fan7<T>>;
type Fan7<T> = Pair<Fan8<T>, Fan8<T>>;
type Fan8<T> = number | T;
type Fanned = Fan<Fanned>;
let fanned: Fanned;

type Branch<T> = Pair<Branch<T>, Branch<T>>;
type Branched = Branch<Branched>;
let branched: Branched;

// CHECK-DAG: [[CHAIN:%tuple\.[0-9]+]] = tuple(number)
// CHECK-DAG: [[BRANCH:%tuple\.[0-9]+]] = tuple([[BRANCH]], [[BRANCH]])
// CHECK: Decl {{.*}} 'chain' Let : [[CHAIN]]
// CHECK-NEXT: Decl {{.*}} 'fanned' Let : %tuple.{{[0-9]+}}
// CHECK-NEXT: Decl {{.*}} 'branched' Let : [[BRANCH]]
