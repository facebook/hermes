/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

use juno::ast::Context;
use juno::ast::GCLock;
use juno::ast::Node;
use juno::ast::NodeRc;
use juno::ast::TransformResult;

use crate::passes::*;

/// Manager to create pipelines of multiple passes over the AST.
#[derive(Default)]
pub struct PassManager {
    passes: Vec<Box<dyn Pass>>,
}

impl PassManager {
    /// Create with an empty pipeline.
    pub fn new() -> Self {
        Default::default()
    }

    /// Add `pass` to the pipeline.
    pub fn add_pass(&mut self, pass: Box<dyn Pass>) {
        self.passes.push(pass)
    }

    /// Pipeline containing a list of standard passes.
    pub fn standard() -> Self {
        Self {
            passes: vec![Box::new(reduce_conditional::ReduceConditional::new())],
        }
    }

    /// Pipeline containing only the Flow type stripping pass.
    pub fn strip_flow() -> Self {
        Self {
            passes: vec![Box::new(strip_flow::StripFlow::new())],
        }
    }

    /// Run the pipeline on `node`, consuming it in the process.
    pub fn run(mut self, ctx: &mut Context, node: NodeRc) -> NodeRc {
        let mut result = node;
        for pass in &mut self.passes {
            {
                let gc = GCLock::new(ctx);
                result = match pass.run(&gc, result.node(&gc)) {
                    TransformResult::Unchanged => result,
                    TransformResult::Removed => {
                        panic!("Program node removed");
                    }
                    TransformResult::Changed(new_node) => NodeRc::from_node(&gc, new_node),
                    TransformResult::Expanded(..) => {
                        panic!("Program node cannot be expanded");
                    }
                };
            }
            ctx.gc();
        }
        result
    }
}

/// A single pass over the AST.
pub trait Pass {
    /// Short name of the pass.
    fn name(&self) -> &'static str;

    /// Description of what the pass is and what it does.
    fn description(&self) -> &'static str;

    /// Execute the pass on the root `node` and return a `TransformResult`.
    fn run<'gc>(
        &mut self,
        gc: &'gc GCLock,
        node: &'gc Node<'gc>,
    ) -> TransformResult<&'gc Node<'gc>>;
}
