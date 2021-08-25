/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// Integration test of Hermes Parser with Source Map

use juno::ast;
use juno::hparser::{parse_with_cvt, CvtLoc};
use juno::hsource_map::SourceMap;
use juno::nullbuf::NullTerminatedBuf;

#[test]
fn test_parser_source_map() {
    use std::convert::TryFrom;

    const MAP: &str = r#"{"version":3,"sources":["throw.js"],"names":["entryPoint","helper","s","x","y","z","Error"],"mappings":"AAOA,SAASA,aACPC,SAGF,SAASA,SACP,IAAIC,EAAI,MACR,IAAIC,EAAI,EACR,IAAIC,EACJ,IAAIC,EAAI,MACR,MAAM,IAAIC,MAAM,uBAGlBN"}"#;
    const SOURCE: &str = r#"function entryPoint(){helper()}function helper(){var s="abc";var x=1;var y;var z=false;throw new Error("exception is thrown")}entryPoint();"#;

    struct Converter {
        map: SourceMap,
    }
    impl CvtLoc for Converter {
        fn convert_loc(&self, file_id: u32, line: u32, column: u32) -> (u32, u32, u32) {
            assert_eq!(file_id, 1);
            match self.map.find_location(line, column) {
                Some(l) => (u32::try_from(l.path_index).unwrap(), l.line, l.column),
                None => (file_id, line, column),
            }
        }
    }

    let cvt = Converter {
        map: SourceMap::parse(NullTerminatedBuf::from_str_copy(MAP)).unwrap(),
    };

    let ast = parse_with_cvt(SOURCE, 1, &cvt).unwrap();
    assert_eq!(
        ast.range,
        ast::SourceRange {
            file: 0,
            start: ast::SourceLoc { line: 8, col: 1 },
            // FIXME: this is wrong!
            end: ast::SourceLoc { line: 20, col: 1 },
        }
    );
}
