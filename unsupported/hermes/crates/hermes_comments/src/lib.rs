/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

use hermes_estree::ESTreeNode;
use hermes_estree::Node;
use hermes_estree::Program;
use hermes_estree::Range;
use hermes_estree::SourceRange;
use hermes_estree::Visitor;
use hermes_parser::Comment;

struct CommentAttachmentVisitor<'a> {
    comments: &'a [Comment],
    idx: usize,
    attached_comments: Vec<(&'a str, SourceRange, Node<'a>, SourceRange)>,
}

impl<'a> Visitor<'a> for CommentAttachmentVisitor<'a> {
    const VISIT_NODE: bool = true;

    // Avoid attaching comments to Program
    fn visit_program(&mut self, ast: &'a Program) {
        for body in &ast.body {
            self.visit_module_item(body);
        }
    }

    fn visit_node<T: ESTreeNode + Range>(&mut self, node: &'a T) -> bool {
        if self.idx >= self.comments.len() {
            return true;
        }
        let node_start = node.range().start;
        // If there are multiple comments, only attch the last one closest to the node
        let mut found_node = false;
        while self.idx < self.comments.len() {
            if Into::<u32>::into(self.comments[self.idx].range.end) < node_start {
                found_node = true;
                self.idx += 1;
            } else {
                break;
            }
        }
        if found_node {
            self.attached_comments.push((
                &self.comments[self.idx - 1].value,
                self.comments[self.idx - 1].range,
                node.as_node_enum(),
                node.range(),
            ));
            return true;
        }
        false
    }
}

impl<'a> CommentAttachmentVisitor<'a> {
    fn new(comments: &'a [Comment]) -> Self {
        Self {
            comments,
            idx: 0,
            attached_comments: Default::default(),
        }
    }

    fn result(self) -> Vec<(&'a str, SourceRange, Node<'a>, SourceRange)> {
        self.attached_comments
    }
}

/// For every comment, attach the outermost node that follows it.
/// This has the assumption that comments and AST nodes are visited in the order they appear in the file.
pub fn find_nodes_after_comments<'a>(
    program: &'a Program,
    comments: &'a [Comment],
) -> Vec<(&'a str, SourceRange, Node<'a>, SourceRange)> {
    let mut comment_attachment_visitor = CommentAttachmentVisitor::new(comments);
    comment_attachment_visitor.visit_program(program);
    comment_attachment_visitor.result()
}
