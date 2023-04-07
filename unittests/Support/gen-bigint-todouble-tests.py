# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

import collections
import struct

DoubleComponents = collections.namedtuple(
    "DoubleComponents", ["sign", "exp", "mantissa"]
)


def to_double(n):
    """Return n converted to float, or +/- inf if n is too large."""
    try:
        return float(n)
    except OverflowError:
        return float(f"{'-' if n < 0 else '+'}inf")


def double_to_bytes(f):
    """Split f into a sequence of 8 bytes."""
    assert type(f) is float, type(f)
    return [c for c in struct.pack("!d", f)]


def bitcast_double_to_int(f):
    """Bit-casts the float f to a 64-bit integer."""
    ret = 0
    for b in double_to_bytes(f):
        ret = (ret << 8) + b
    return ret


def double_to_components(d):
    """Returns the DoubleComponents tuple for the given double."""
    tmp = bitcast_double_to_int(d)
    return DoubleComponents(
        "negative" if ((tmp >> 63) & 1) else "positive",
        ((tmp >> 52) & ((1 << 11) - 1)) - 0x3FF,
        "0x{:0>16x}".format((tmp & ((1 << 52) - 1))),
    )


def int_to_digits(n):
    """Convert the given integer to BigInt digits.

    Each digit is in the form

    digit([bytes])

    digit is the helper test function used to create immutable bigint refs.
    """

    # copy the bytes (in two's complement) to n_bytes. This terminates when
    # either n == 0 (if the original n >= 0) or when n == -1 (if the original
    # n <= -1).
    n_bytes = []
    while n != 0 and n != -1:
        n_bytes.append(n & 0xFF)
        n >>= 8

    # Now adjust the sign byte, making sure n_bytes represents a negative value
    # if the original n was negative; or a non-negative value otherwise.
    if n == -1:
        # if n == -1, the original n was negative. Thus make sure the most
        # significant bit in n_bytes is set.
        if not n_bytes or n_bytes[-1] <= 0x7F:
            # The most significant bit in n_bytes is cleared; add a byte
            # sign-extending n_bytes.
            n_bytes.append(255)
    else:
        assert n == 0
        # if n == 0, the original n was positive or zero. Thus make sure the
        # most significant bit in n_bytes is cleared.
        if not n_bytes or n_bytes[-1] > 0x7F:
            # The most significant bit in n_bytes is cleared; add a byte
            # zero-extending n_bytes.
            n_bytes.append(0)

    # Digits is the sequence of "digits([bytes])" strings that will become the
    # input to the test.
    digits = []
    i = 0
    while i < len(n_bytes):
        curr_digit = n_bytes[i : min(len(n_bytes), i + 8)]
        curr_digit.reverse()
        digits.append(f"digit({', '.join('0x{:0>2x}'.format(x) for x in curr_digit)})")
        i += 8

    # Reversing digits so the most significant digit is digit[0].
    digits.reverse()

    return digits


def level_print(*args, **kwargs):
    """Drop-in replacement for print that supports a level keyword args.

    If present, level is used to prefix the call to print with 2*level spaces.
    """
    level = 0 if "level" not in kwargs else kwargs.pop("level")
    if level:
        print("  " * (level), end="")
    print(*args, **kwargs)


def print_expectation(d, curr_level=0):
    """Prints the test expectation.

    Prints the test expectation for a test. Handles the values 0.0, +inf, and
    -inf specially. If d is none of those, use makeDouble to "assemble" the
    expectation.
    """

    if d == 0:
        level_print("zero,", level=curr_level + 1)
    elif d == float("inf"):
        level_print(f"infinity,", level=curr_level + 1)
    elif d == -float("inf"):
        level_print(f"neg_infinity,", level=curr_level + 1)
    else:
        c = double_to_components(d)
        level_print(
            "makeDouble({c.sign}, exp({c.exp}), mantissa({c.mantissa}ull)),".format(
                c=c
            ),
            level=curr_level + 1,
        )


def print_bigint_digits(digits, curr_level=0):
    """Prints the BigInt digits that are passed to toDouble.

    Ensures that a ' +' is printed between digits.
    """
    for i in range(len(digits)):
        last_digit = i == len(digits) - 1
        end_sep = "" if last_digit else "\n"
        level_print(
            f"{digits[i]}{'' if last_digit else ' +'}",
            level=curr_level + 2,
            end=end_sep,
        )


def print_test(n):
    """Prints a BigInt toDouble test."""
    d = to_double(n)

    digits = int_to_digits(n)

    level_print("EXPECT_EQ(", level=0)
    print_expectation(d, curr_level=0)

    level_print("toDouble(toImmutableRef(", level=1)
    print_bigint_digits(digits, curr_level=1)
    level_print(")));\n")


def bn(s, e):
    """Builds an integer with the given string.

    Helper function that creates an integer using the given string of binary
    digits s. e is the exponent of the most significant "1" digit.
    """

    # Dropping leading zeros that could have been used for formatting purposes.
    i = 0
    while i < len(s) and s[i] == "0":
        i += 1

    # s is a string of zeros -- thus return 0.
    if len(s) == i:
        return 0

    # First convert the given binary string to an integer, then shift it left by
    # e taking into account the number of bits in s.
    s = s[i:]
    return int(f"0b{s}", 2) << (e - len(s) + 1)


# Test generation below.

print(
    """/*
* Copyright (c) Meta Platforms, Inc. and affiliates.
*
* This source code is licensed under the MIT license found in the
* LICENSE file in the root directory of this source tree.
*/

// clang-format off
"""
)

print_test(0)

print("// Mantissa is 52 ones.")
print_test((1 << 53) - 1)

print("// Largest positive number that fits a double without precision loss.")
print_test(1 << 53)

print("// Smallest negative number that fits a double without precision loss.")
print_test(-(1 << 53))

# 1 << 53 -> Mantissa: 0x20000000000000, thus m: = 0, 1 bits dropped
#                               #  +  L  +  M  + Round? +  m  +
print_test((1 << 53) + 0b0000)  #  |  0  |  0  |  No    | 0x0 |
print_test((1 << 53) + 0b0001)  #  |  0  |  1  |  No    | 0x0 |
print_test((1 << 53) + 0b0010)  #  |  1  |  0  |  No    | 0x1 |
print_test((1 << 53) + 0b0011)  #  |  1  |  1  |  Yes   | 0x2 |
#                          ^^
#                          |Most significant bit not in the mantissa. (M)
#                          Mantissa's least significant bit. (L)

# 1 << 54 -> Mantissa: 0x40000000000000, thus m: = 0, 2 bits dropped
#                               #  +  L  +  M  +  d  + Round? +  m  +
print_test((1 << 54) + 0b0000)  #  |  0  |  0  |  0  |  No    | 0x0 |
print_test((1 << 54) + 0b0001)  #  |  0  |  0  |  1  |  No    | 0x0 |
print_test((1 << 54) + 0b0010)  #  |  0  |  1  |  0  |  No    | 0x0 |
print_test((1 << 54) + 0b0011)  #  |  0  |  1  |  1  |  Yes   | 0x1 |
print_test((1 << 54) + 0b0100)  #  |  1  |  0  |  0  |  No    | 0x1 |
print_test((1 << 54) + 0b0101)  #  |  1  |  0  |  1  |  No    | 0x1 |
print_test((1 << 54) + 0b0110)  #  |  1  |  1  |  0  |  Yes   | 0x2 |
print_test((1 << 54) + 0b0111)  #  |  1  |  1  |  1  |  Yes   | 0x2 |
#                         ^^^
#                         ||d
#                         |Most significant bit not in the mantissa. (M)
#                         Mantissa's least significant bit.(L)

# The following test mantissa is 0xa5a5a5a5a5a5a (when L = 0), or
# 0xa5a5a5a5a5a5b (when L = 1). It is then utilized to ensure toDouble
# can convert any
test_mantissa = "1101001011010010110100101101001011010010110100101101L"
#                                                                    ^
#                                                                    Mantissa's least significant bit.

print(
    """/********************************************************************
 *
 * The following tests have a 53-bit mantissa that fits the double
 * precisely, and they have trailing zeros. No rounding should occur given
 * that the most significant bit not in the mantissa is always 0.
 *
 ********************************************************************/
"""
)
# Use step=3 to avoid test explosion while getting good coverage.
for i in range(52, 128, 3):
    test_mantissa = test_mantissa[:-1] + "0"
    print_test(bn(test_mantissa, i))
    print_test(-bn(test_mantissa, i))

    test_mantissa = test_mantissa[:-1] + "1"
    print_test(bn(test_mantissa, i))
    print_test(-bn(test_mantissa, i))

# The following test mantissa is 0xa5a5a5a5a5a5a (when L = 0), or
# 0xa5a5a5a5a5a5b (when L = 1). It is then utilized to ensure toDouble
# can convert any
test_mantissa = "1101001011010010110100101101001011010010110100101101LM"
#                                                                    ^^
#                                                                    |Most Significant bit not in the mantissa
#                                                                    Mantissa's least significant bit.

print(
    """/********************************************************************
 *
 * The following tests have 54-bits. The 2 lowest significant bits are
 * modified to exercise different types of rounding.
 *
 ********************************************************************/
"""
)
for i in range(53, 128, 41):
    print("// L=0, M=0 -> never round")
    test_mantissa = test_mantissa[:-2] + "00"
    print_test(bn(test_mantissa, i) + 1)
    print_test(-bn(test_mantissa, i) - 1)

    print("// L=0, M=1 -> round if d != 0")
    test_mantissa = test_mantissa[:-2] + "01"
    print_test(bn(test_mantissa, i) + 1)
    print_test(-bn(test_mantissa, i) - 1)

    print("// L=1, M=0 -> never round")
    test_mantissa = test_mantissa[:-2] + "10"
    print_test(bn(test_mantissa, i) + 1)
    print_test(-bn(test_mantissa, i) - 1)

    print("// L=1, M=1 -> always round")
    test_mantissa = test_mantissa[:-2] + "11"
    print_test(bn(test_mantissa, i) + 1)
    print_test(-bn(test_mantissa, i) - 1)


for i in range(128, 1024, 499):
    print("// L=0, M=0 -> never round")
    test_mantissa = test_mantissa[:-2] + "00"
    print_test(bn(test_mantissa, i) + (1 << 87))
    print_test(-bn(test_mantissa, i) - (1 << 93))

    print("// L=0, M=1 -> round if d != 0")
    test_mantissa = test_mantissa[:-2] + "01"
    print_test(bn(test_mantissa, i) + (1 << 103))
    print_test(-bn(test_mantissa, i) - (1 << 73))

    print("// L=1, M=0 -> never round")
    test_mantissa = test_mantissa[:-2] + "10"
    print_test(bn(test_mantissa, i) + (1 << 115))
    print_test(-bn(test_mantissa, i) - (1 << 71))

    print("// L=1, M=1 -> always round")
    test_mantissa = test_mantissa[:-2] + "11"
    print_test(bn(test_mantissa, i) + (1 << 123))
    print_test(-bn(test_mantissa, i) - (1 << 101))

print("// Infinity -- 1 ** 1024 is too large to fit a double.")
print_test(bn("1", 1024))
print_test(-bn("1", 1024))

print("// Not infinity, but almost")
print_test(bn("111111111111111111111111111111111111111111111111111110", 1023))
#                                                                  LM
print('// This is the "first" "infinite" bigint')
print_test(bn("111111111111111111111111111111111111111111111111111111", 1023))
#                                                                  LM

print("// Not -infinity, but almost")
print_test(-bn("111111111111111111111111111111111111111111111111111110", 1023))
#                                                                   LM
print('// This is the "first" "-infinite" bigint')
print_test(-bn("111111111111111111111111111111111111111111111111111111", 1023))
#                                                                   LM
