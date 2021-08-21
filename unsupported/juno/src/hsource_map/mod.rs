/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

mod c_api;

use crate::nullbuf::NullTerminatedBuf;
use c_api::*;
use thiserror::Error;

#[derive(Error, Debug)]
#[error("{msg}")]
pub struct Error {
    msg: String,
}

#[derive(Clone, Debug, PartialEq, Eq)]
pub struct Location {
    /// Index in the paths owned by the Source Map.
    pub path_index: usize,
    /// 1-based line.
    pub line: u32,
    /// 1-based column.
    pub column: u32,
}

/// A successfully parsed SourceMap that can be used to query locations.
pub struct SourceMap {
    map: *mut HermesSourceMap,
    paths: Vec<String>,
}

impl Drop for SourceMap {
    fn drop(&mut self) {
        unsafe { hermes_source_map_free(self.map) }
    }
}

impl SourceMap {
    /// Parse a null-terminated buffer into a SourceMap or an Error if the map is invalid.
    pub fn parse(input: NullTerminatedBuf) -> Result<SourceMap, Error> {
        unsafe {
            let map = hermes_source_map_parse(input.as_c_char_ptr(), input.len());
            let err = hermes_source_map_get_error(map);
            if !err.is_null() {
                return Err(Error {
                    msg: String::from(String::from_utf8_lossy(err.as_slice())),
                });
            }

            let path_count = hermes_source_map_get_num_paths(map);
            let mut paths = Vec::with_capacity(path_count as usize);
            for i in 0..path_count {
                paths.push(String::from(String::from_utf8_lossy(
                    hermes_source_map_get_full_path(map, i).as_slice(),
                )));
            }

            Ok(SourceMap { map, paths })
        }
    }

    /// Return all paths stored in this source map.
    pub fn paths(&self) -> &[String] {
        self.paths.as_slice()
    }

    /// Find the path with the specified value and return its index. This is a slow function
    /// intended only for debugging.
    fn find_path_slow(&self, p: &str) -> Option<usize> {
        self.paths.iter().position(|s| s == p)
    }

    /// Find a location in the source map by line and column (both 1-based), and returning Location
    /// or None if it can't be found.
    pub fn find_location(&self, line: u32, column: u32) -> Option<Location> {
        unsafe {
            let res = hermes_source_map_get_location(self.map, line, column);
            if res.is_null() {
                return None;
            }

            Some(Location {
                path_index: (*res).path_index as usize,
                line: (*res).line,
                column: (*res).column,
            })
        }
    }
}

impl HermesDataRef {
    pub fn is_null(&self) -> bool {
        self.data.is_null()
    }
    pub fn is_empty(&self) -> bool {
        self.length == 0
    }
    pub fn as_slice<T>(&self) -> &[T] {
        // Rust doesn't allow slices with null pointers.
        if self.data.is_null() {
            &[]
        } else {
            unsafe { std::slice::from_raw_parts(self.data as *const T, self.length) }
        }
    }
}

#[cfg(test)]
mod test {
    use super::*;

    const MAP: &str = r#"
    {"version":3,"sources":["throw.js"],"names":["entryPoint","helper","s","x","y","z","Error"],"mappings":"AAOA,SAASA,aACPC,SAGF,SAASA,SACP,IAAIC,EAAI,MACR,IAAIC,EAAI,EACR,IAAIC,EACJ,IAAIC,EAAI,MACR,MAAM,IAAIC,MAAM,uBAGlBN"}
    "#;

    #[test]
    fn test_error() {
        assert!(SourceMap::parse(NullTerminatedBuf::from_str_copy("not value")).is_err());
    }

    #[test]
    fn test_ok() {
        let map = SourceMap::parse(NullTerminatedBuf::from_str_copy(MAP)).unwrap();

        assert_eq!(
            map.find_location(1, 98).unwrap(),
            Location {
                path_index: map.find_path_slow("throw.js").unwrap(),
                line: 17,
                column: 13,
            }
        );
    }
}
