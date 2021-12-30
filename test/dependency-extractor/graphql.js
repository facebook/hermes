/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %dependency-extractor %s | %FileCheck --match-full-lines %s

func(C, {
  foo: graphql`
    fragment Foo on Bar {
      x: y(
        p1: asdf
        p2: "x"
      ) {
        id
        ... on bar {
          baz(key: "two")
        }
      }
    }
    fragment Foo2 on Bar {
      x: y(
        p1: asdf
        p2: "x"
      ) {
        id
        ... on bar {
          baz(key: "two")
        }
      }
    }
  `,
});
// CHECK: GraphQL | Foo.graphql
// CHECK-NEXT: GraphQL | Foo2.graphql

not_graphql`
  query Bar {
  }
  mutation Bar2 {
  }
  subscription Bar3 {
  }
`;

graphql`
  query Bar {
  }
  mutation Bar2 {
  }
  subscription Bar3 {
  }
`;
// CHECK-NEXT: GraphQL | Bar.graphql
// CHECK-NEXT: GraphQL | Bar2.graphql
// CHECK-NEXT: GraphQL | Bar3.graphql
