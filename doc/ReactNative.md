---
id: react-native
title: Hermes and React Native
---

Hermes is the default JavaScript engine in React Native. This page collects
the React Native specific information; the rest of the documentation treats
Hermes as a standalone engine.

## Using Hermes in an app

You do not need this repository or a Hermes build. React Native bundles a
matching Hermes with every release. Follow the
[React Native Hermes guide](https://reactnative.dev/docs/hermes).

## Versions

Each React Native version pins a specific Hermes build. Hermes bytecode and
the engine ABI change between versions, so mixing a Hermes from one release
with a React Native from another can crash at start-up. Use the Hermes that
your React Native version ships with. Release notes are on the
[releases page](https://github.com/facebook/hermes/releases).

## Hermes V1

"Hermes V1" is what React Native calls releases from this repository's
`static_h` branch, versioned independently of React Native by branch date and
bytecode version (for example 260318099.0.0) instead of the legacy 0.x. React
Native 0.82 added it as an opt-in and later versions use it by default. See
[Branches.md](Branches.md) for how the branches and version numbers relate.

To move an app on an older React Native onto a current Hermes V1, see
[rn-0.xx-hermes-v1](https://github.com/tmikov/rn-0.xx-hermes-v1), a guide
with worked examples for each starting version.

## Custom builds and native crashes

To use a Hermes you built yourself in a React Native app, and to symbolicate
native crashes in Hermes, see
[ReactNativeIntegration.md](ReactNativeIntegration.md).
