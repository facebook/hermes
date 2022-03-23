/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

//! Hermes Non-standard UTF-8 Support
//!
//! Hermes uses a custom UTF-8 encoding format, suitable for JavaScript.
//! JavaScript strings are a sequence of 16-bit values that do not necessarily
//! represent a valid UTF-16 string (although most of them usually are).
//! Specifically the values can be unmatched surrogate pairs.
//!
//! The Hermes UTF-8 encoding was thus chosen to represent JavaScript strings
//! losslessly. Each 16-bit value is encoded separately using the principles of
//! UTF-8, ignoring whether it is a part o surrogate pair. The resulting byte
//! sequence is definitely *NOT* guaranteed to be a valid UTF-8 string (although
//! in practice it is in many cases, if only valid Unicode characters smaller
//! than 0x10000 were used in the input).
//!
//! Note that even correct UTF-16 strings do not result a valid UTF-8 sequence,
//! because Unicode characters larger than 0xFFFF are represented in UTF-16
//! with a surrogate pair, and each element of the pair is encoded separately
//! into UTF-8.

use thiserror::Error;

#[derive(Clone, Copy, Error, Debug, Eq, PartialEq)]
pub enum UTFError {
    // General.
    #[error("invalid Unicode character")]
    InvalidUnicodeCharacter,
    // UTF8.
    #[error("incomplete UTF-8 continuation")]
    UTF8IncompleteCont,
    #[error("invalid UTF-8 continuation byte")]
    UTF8InvalidContByte,
    #[error("non-canonical UTF-8 encoding")]
    UTF8NonCanonicalEncoding,
    #[error("invalid UTF-8 leading byte")]
    UTF8InvalidLeadByte,
    // UTF16.
    #[error("invalid UTF-16 character")]
    UTF16InvalidCharacter,
    #[error("unmatched UTF-16 low surrogate")]
    UTF16UnmatchedLowSurrogate,
    #[error("incomplete UTF-16 surrogate pair")]
    UTF16IncompleteSurrogatePair,
}

const UNICODE_MAX_VALUE: u32 = char::MAX as u32;
/// The start of the surrogate range.
const UNICODE_SURROGATE_FIRST: u32 = 0xD800;
/// The last character of the surrogate range (inclusive).
const UNICODE_SURROGATE_LAST: u32 = 0xDFFF;
const UTF16_HIGH_SURROGATE: u32 = 0xD800;
const UTF16_LOW_SURROGATE: u32 = 0xDC00;
const UNICODE_REPLACEMENT_CHARACTER: u32 = char::REPLACEMENT_CHARACTER as u32;

/// Return whether the character is part of a UTF-8 sequence.
#[inline]
pub fn is_utf8(ch: u8) -> bool {
    (ch & 0x80) != 0
}

/// Return true if this is a UTF-8 leading byte.
#[inline]
pub fn is_utf8_lead(ch: u8) -> bool {
    (ch & 0xC0) == 0xC0
}

/// Return true if this is a UTF-8 continuation byte, or in other words, this
/// is a byte in the "middle" of a UTF-8 codepoint.
#[inline]
pub fn is_utf8_continuation(ch: u8) -> bool {
    (ch & 0xC0) == 0x80
}

/// Returns whether cp is a high surrogate.
#[inline]
fn is_high_surrogate(cp: u32) -> bool {
    UNICODE_SURROGATE_FIRST <= cp && cp < UTF16_LOW_SURROGATE
}

/// Returns whether cp is a low surrogate.
#[inline]
fn is_low_surrogate(cp: u32) -> bool {
    UTF16_LOW_SURROGATE <= cp && cp <= UNICODE_SURROGATE_LAST
}

/// Decode a surrogate pair [\p hi, \p lo] into a code point.
#[inline]
fn decode_surrogate_pair(hi: u32, lo: u32) -> u32 {
    debug_assert!(
        is_high_surrogate(hi) && is_low_surrogate(lo),
        "Not a surrogate pair"
    );
    ((hi - UTF16_HIGH_SURROGATE) << 10) + (lo - UTF16_LOW_SURROGATE) + 0x10000
}

/// @param ch the byte at `src[from]`, which also implies that `src[from]` is a valid index.
fn decode_utf8<const ALLOW_SURROGATES: bool>(
    src: &[u8],
    from: &mut usize,
    ch: u8,
) -> Result<u32, UTFError> {
    if !is_utf8(ch) {
        *from += 1;
        Ok(ch as u32)
    } else {
        decode_utf8_slow_path::<ALLOW_SURROGATES>(src, from, ch as u32)
    }
}

/// @param ch the byte at `src[from]`, which also implies that `src[from]` is a valid index.
fn decode_utf8_slow_path<const ALLOW_SURROGATES: bool>(
    src: &[u8],
    from: &mut usize,
    ch: u32,
) -> Result<u32, UTFError> {
    debug_assert!(is_utf8(ch as u8));
    let result: u32;
    let len = src.len();
    if (ch & 0xE0) == 0xC0 {
        if *from + 1 >= len {
            *from = len;
            return Err(UTFError::UTF8IncompleteCont);
        }
        let ch1 = unsafe { *src.get_unchecked(*from + 1) } as u32;
        if (ch1 & 0xC0) != 0x80 {
            *from += 1;
            return Err(UTFError::UTF8InvalidContByte);
        }
        *from += 2;
        result = ((ch & 0x1F) << 6) | (ch1 & 0x3F);
        if result <= 0x7F {
            return Err(UTFError::UTF8NonCanonicalEncoding);
        }
    } else if (ch & 0xF0) == 0xE0 {
        if *from + 2 >= len {
            *from = len;
            return Err(UTFError::UTF8IncompleteCont);
        }
        let ch1 = unsafe { *src.get_unchecked(*from + 1) } as u32;
        if (ch1 & 0x40) != 0 || (ch1 & 0x80) == 0 {
            *from += 1;
            return Err(UTFError::UTF8InvalidContByte);
        }
        let ch2 = unsafe { *src.get_unchecked(*from + 2) } as u32;
        if (ch2 & 0x40) != 0 || (ch2 & 0x80) == 0 {
            *from += 2;
            return Err(UTFError::UTF8InvalidContByte);
        }
        *from += 3;
        result = ((ch & 0x0F) << 12) | ((ch1 & 0x3F) << 6) | (ch2 & 0x3F);
        if result <= 0x7FF {
            return Err(UTFError::UTF8NonCanonicalEncoding);
        }
        if !ALLOW_SURROGATES
            && result >= UNICODE_SURROGATE_FIRST
            && result <= UNICODE_SURROGATE_LAST
        {
            return Err(UTFError::InvalidUnicodeCharacter);
        }
    } else if (ch & 0xF8) == 0xF0 {
        if *from + 3 >= len {
            *from = len;
            return Err(UTFError::UTF8IncompleteCont);
        }
        let ch1 = unsafe { *src.get_unchecked(*from + 1) } as u32;
        if (ch1 & 0x40) != 0 || (ch1 & 0x80) == 0 {
            *from += 1;
            return Err(UTFError::UTF8InvalidContByte);
        }
        let ch2 = unsafe { *src.get_unchecked(*from + 2) } as u32;
        if (ch2 & 0x40) != 0 || (ch2 & 0x80) == 0 {
            *from += 2;
            return Err(UTFError::UTF8InvalidContByte);
        }
        let ch3 = unsafe { *src.get_unchecked(*from + 3) } as u32;
        if (ch3 & 0x40) != 0 || (ch3 & 0x80) == 0 {
            *from += 3;
            return Err(UTFError::UTF8InvalidContByte);
        }
        *from += 4;
        result = ((ch & 0x07) << 18) | ((ch1 & 0x3F) << 12) | ((ch2 & 0x3F) << 6) | (ch3 & 0x3F);
        if result <= 0xFFFF {
            return Err(UTFError::UTF8NonCanonicalEncoding);
        }
        if result > UNICODE_MAX_VALUE {
            return Err(UTFError::InvalidUnicodeCharacter);
        }
    } else {
        *from += 1;
        return Err(UTFError::UTF8InvalidLeadByte);
    }

    Ok(result)
}

pub fn utf8_with_surrogates_to_utf16(src: &[u8]) -> Result<Vec<u16>, UTFError> {
    let mut from: usize = 0;
    let mut v = Vec::new();
    let len = src.len();

    v.reserve(len);
    while from < len {
        // We checked `from` already.
        let b = unsafe { *src.get_unchecked(from) };
        if !is_utf8(b) {
            from += 1;
            v.push(b as u16);
            continue;
        }

        v.push(
            match decode_utf8_slow_path::<true>(src, &mut from, b as u32)? {
                x if x <= 0xFFFF => x as u16,
                _ => return Err(UTFError::UTF16InvalidCharacter),
            },
        )
    }

    v.shrink_to_fit();
    Ok(v)
}

/// Returns a string with replacement and an optional error.
fn utf8_with_surrogates_to_string_helper(src: &[u8]) -> (String, Option<UTFError>) {
    let mut from: usize = 0;
    let mut str = String::new();
    let len = src.len();
    let mut err: Option<UTFError> = None;

    str.reserve(len);
    while from < len {
        // We checked `from` already.
        let b = unsafe { *src.get_unchecked(from) };
        if !is_utf8(b) {
            from += 1;
            str.push(b as char);
            continue;
        }

        let mut cp: u32;
        match decode_utf8_slow_path::<true>(src, &mut from, b as u32) {
            Ok(x) => cp = x,
            Err(e) => {
                err = err.or(Some(e));
                cp = UNICODE_REPLACEMENT_CHARACTER;
            }
        }
        if is_low_surrogate(cp) {
            err = err.or(Some(UTFError::UTF16UnmatchedLowSurrogate));
            cp = UNICODE_REPLACEMENT_CHARACTER;
        } else if is_high_surrogate(cp) {
            if from >= len {
                err = err.or(Some(UTFError::UTF16IncompleteSurrogatePair));
                cp = UNICODE_REPLACEMENT_CHARACTER;
            } else {
                // We checked `from` already.
                let b1 = unsafe { *src.get_unchecked(from) };
                match decode_utf8::<true>(src, &mut from, b1) {
                    Ok(cp_low) => {
                        if !is_low_surrogate(cp_low) {
                            err = err.or(Some(UTFError::UTF16IncompleteSurrogatePair));
                            cp = UNICODE_REPLACEMENT_CHARACTER;
                        } else {
                            cp = decode_surrogate_pair(cp, cp_low);
                        }
                    }
                    Err(e) => {
                        err = err.or(Some(e));
                        cp = UNICODE_REPLACEMENT_CHARACTER;
                    }
                }
            }
        }
        str.push(unsafe { char::from_u32_unchecked(cp) });
    }

    str.shrink_to_fit();
    (str, err)
}

pub fn utf8_with_surrogates_to_string(src: &[u8]) -> Result<String, UTFError> {
    match utf8_with_surrogates_to_string_helper(src) {
        (_, Some(e)) => Err(e),
        (s, None) => Ok(s),
    }
}

pub fn utf8_with_surrogates_to_string_lossy(src: &[u8]) -> String {
    utf8_with_surrogates_to_string_helper(src).0
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_utf8_with_surrogates() {
        assert_eq!(
            utf8_with_surrogates_to_string(&[97, 98, 99]).unwrap(),
            "abc"
        );

        assert_eq!(
            {
                let x: Vec<u16> = "😹".encode_utf16().collect();
                x
            },
            [0xD83D, 0xDE39]
        );
        assert_eq!(
            utf8_with_surrogates_to_utf16(&[0xED, 0xA0, 0xBD, 0xED, 0xB8, 0xB9]).unwrap(),
            [0xD83D, 0xDE39]
        );
        assert_eq!(
            utf8_with_surrogates_to_string(&[0xED, 0xA0, 0xBD, 0xED, 0xB8, 0xB9]).unwrap(),
            "😹"
        );
    }
    #[test]
    fn test_lossy() {
        assert_eq!(
            utf8_with_surrogates_to_string_lossy(&[0xED, 0xA0, 0x30, 0xED, 0xB8, 0xB9]),
            "�0�"
        );
    }
}
