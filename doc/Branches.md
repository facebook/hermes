---
id: branches
title: Branches and Releases
---

The branch layout of this repository reflects its history more than a plan,
so here is a map.

## `static_h`: active development

All development happens on `static_h`. Pull requests are opened against it
(see [CONTRIBUTING.md](../CONTRIBUTING.md)). The name is a leftover from the
"Static Hermes" project that started on this branch; see the README for the
history. Everything described in this documentation refers to `static_h`
unless it says otherwise.

## `main`: the legacy line

`main` carries the original Hermes, the engine that shipped in React Native
for years with 0.x version numbers. It receives few changes and no new
features. It is not the base for new work.

## Release branches

Releases from `static_h` are the ones React Native calls **Hermes V1**. They
are versioned independently of React Native. A release lives on a pair of
branches named after its version:

* `<version>-staging`: where a release is prepared.
* `<version>-stable`: the released line. Fixes are grafted onto it from
  `static_h` as needed, and patch releases are cut from it.

For example, `260318099.0.0-stable` is the stable branch of release
260318099.0.0. The leading number encodes the date the release was branched
and the bytecode version it ships, so `260318099` is 2026-03-18 with
bytecode version 99.

The older `release-v0.x` branches and `v0.x` tags belong to the legacy line.
