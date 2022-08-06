/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

//! URL fetcher.
//!
//! Provides an abstraction to fetch data from an arbitrary URL. For now
//! only file: and data: schemes are supported, but more can be added easily
//! (for example http:).
//!
//! This all could be made flexible and configurable with providers and so on,
//! but I see no point to complicate it at this point.

use std::fs::File;
use std::io::Read;

use anyhow;
use base64;
use thiserror;
use url::Url;

#[derive(thiserror::Error, Debug)]
pub enum FetchError {
    #[error("URL parse error")]
    URLParseError(#[source] anyhow::Error),
    #[error("unsupported URL scheme")]
    UnsupportedScheme,
    #[error("URL scheme prohibited by policy")]
    ProhibitedScheme,
    #[error("invalid URL: {0}")]
    InvalidURL(&'static str),
    #[error("I/O error")]
    IOError(#[source] std::io::Error),
    #[error("error decoding data")]
    DecodeError(#[source] anyhow::Error),
}

impl From<std::io::Error> for FetchError {
    fn from(err: std::io::Error) -> Self {
        FetchError::IOError(err)
    }
}

/// Data data returned by [fetch_url()]. It could potentially be a file memory
/// mapping or who knows what else.
#[derive(Debug)]
pub struct Data {
    buf: Vec<u8>,
}

impl AsRef<[u8]> for Data {
    fn as_ref(&self) -> &[u8] {
        self.buf.as_slice()
    }
}

/// Flags to customize behavior of `fetch_url()` and more specifically to restrict it from accessing
/// file system or network.
pub struct FetchFlags {
    /// Allow fetching from a file.
    pub allow_file: bool,
    /// Allow fetching from a supported network protocol.
    #[allow(dead_code)]
    pub allow_network: bool,
}

impl Default for FetchFlags {
    fn default() -> Self {
        FetchFlags {
            allow_file: true,
            allow_network: true,
        }
    }
}

/// Fetch data from a URL with optional restrictions from accessing file system or network (if
/// everything is restricted, the only allowed option are data URLs where everything is encoded
/// in the URL itself).
pub fn fetch_url(url: &Url, flags: FetchFlags) -> Result<Data, FetchError> {
    match url.scheme() {
        "file" => {
            if flags.allow_file {
                fetch_file(url)
            } else {
                Err(FetchError::ProhibitedScheme)
            }
        }
        "data" => fetch_data(url),
        _ => Err(FetchError::UnsupportedScheme),
    }
}

/// A convenience wrapper around [fetch_url()] with a string URL and default flags.
pub fn fetch_from_str_url(str: &str) -> Result<Data, FetchError> {
    fetch_url(
        &Url::parse(str).map_err(|e| FetchError::URLParseError(anyhow::anyhow!(e)))?,
        Default::default(),
    )
}

/// Fetch from a data URL.
/// Data URL format is: `data:[<mediatype>][;base64],<data>`
fn fetch_data(url: &Url) -> Result<Data, FetchError> {
    if !url.cannot_be_a_base() || url.has_host() {
        return Err(FetchError::InvalidURL(
            "data URL cannot be a base or contain a host",
        ));
    }

    let path = url.path();
    // Split using ',' into mtype[;encoding] and base
    let (mtype_enc, data) = path
        .split_once(',')
        .ok_or(FetchError::InvalidURL("data URL missing ',' delimiter"))?;
    // Split using ';' into mtype and encoding. We require the encoding.
    let (_, enc) = mtype_enc
        .split_once(';')
        .ok_or(FetchError::InvalidURL("data URL missing ';encoding'"))?;

    // Only support base64 for now.
    if enc != "base64" {
        return Err(FetchError::InvalidURL("data URL unsupported encoding"));
    }

    let buf = base64::decode_config(data, base64::URL_SAFE).map_err(|e| {
        FetchError::DecodeError(anyhow::anyhow!(e).context("error decoding data URL"))
    })?;

    Ok(Data { buf })
}

/// Fetch from a file URL.
fn fetch_file(url: &Url) -> Result<Data, FetchError> {
    let path = url
        .to_file_path()
        .map_err(|_| FetchError::InvalidURL("invalid file URL"))?;
    let mut file = File::open(path)?;

    let mut buf = Vec::new();
    file.read_to_end(&mut buf)?;
    buf.shrink_to_fit();

    Ok(Data { buf })
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_data_url() {
        assert_eq!(
            fetch_from_str_url("data:application/json;base64,YQ==")
                .unwrap()
                .as_ref(),
            "a".as_bytes()
        );

        // Invalid base64 encoding.
        assert!(matches!(
            fetch_from_str_url("data:application/json;base64,Y").unwrap_err(),
            FetchError::DecodeError(_)
        ));

        // Missing ,data
        assert!(matches!(
            fetch_from_str_url("data:text/plain;base64;YQ==").unwrap_err(),
            FetchError::InvalidURL("data URL missing ',' delimiter")
        ));

        // Missing ;base64
        assert!(matches!(
            fetch_from_str_url("data:,YQ==").unwrap_err(),
            FetchError::InvalidURL("data URL missing ';encoding'")
        ));
        assert!(matches!(
            fetch_from_str_url("data:text/plain,YQ==").unwrap_err(),
            FetchError::InvalidURL("data URL missing ';encoding'")
        ));

        // Check invalid encoding.
        assert!(matches!(
            fetch_from_str_url("data:text/plain;base63,YQ==").unwrap_err(),
            FetchError::InvalidURL("data URL unsupported encoding")
        ));
    }
}
