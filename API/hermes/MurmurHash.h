// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.
#pragma once

#include <cstddef>
#include <cstdint>

// Computes the hash of key using MurmurHash3 algorithm, the value is planced in the "hash" output parameter
// The function returns whether or not key is comprised of only ASCII characters (<=127)
bool murmurhash(const uint8_t *key, size_t length, uint64_t &hash);