/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

use std::env;

use hermes_comments::*;
use hermes_estree::SourceType;
use hermes_parser::parse;
use hermes_parser::ParserDialect;
use hermes_parser::ParserFlags;
use insta::assert_snapshot;
use insta::glob;

#[test]
fn fixtures() {
    let mut settings = insta::Settings::clone_current();
    if let Ok(path) = env::var("SNAPSHOT_PATH") {
        settings.set_snapshot_path(path);
    }
    let _guard = settings.bind_to_scope();
    glob!("fixtures/**.js", |path| {
        println!("fixture {}", path.to_str().unwrap());
        let input = std::fs::read_to_string(path).unwrap();
        let result = parse(
            &input,
            path.to_str().unwrap(),
            ParserFlags {
                strict_mode: true,
                enable_jsx: false,
                dialect: ParserDialect::Flow,
                store_doc_block: false,
                store_comments: true,
            },
        )
        .unwrap();
        // TODO: hack to prevent changing lots of fixtures all at once
        let mut ast = result.ast;
        let comments = result.comments;
        ast.source_type = SourceType::Script;
        let mut attached_comments = find_nodes_after_comments(&ast, &comments);
        attached_comments
            .sort_by(|(comment1, _, _), (comment2, _, _)| comment1.trim().cmp(comment2.trim()));
        assert_snapshot!(format!(
            "Input:\n{}\n\nOutput:\n{}",
            input,
            serde_json::to_string_pretty(&attached_comments).unwrap()
        ))
    });
}
