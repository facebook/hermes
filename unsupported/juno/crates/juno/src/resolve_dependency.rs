/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

use crate::ast::GCLock;
use juno_support::source_manager::SourceId;
use juno_support::source_manager::SourceManager;
use std::collections::HashMap;
use std::path::Component;
use std::path::Path;
use std::path::PathBuf;

#[derive(Debug, Copy, Clone, PartialEq, Eq)]
pub enum DependencyKind {
    Require,
    Import,
}

/// Function used for resolving dependencies.
pub type ResolveDependency = fn(&GCLock, SourceId, &str, DependencyKind) -> Option<SourceId>;

pub trait DependencyResolver {
    fn resolve_dependency(
        &self,
        lock: &GCLock,
        file: SourceId,
        path: &str,
        kind: DependencyKind,
    ) -> Option<SourceId>;
}

#[derive(Debug, Default)]
pub struct DefaultResolver {
    /// Modules with unique names, allowing `require` to avoid specifying the full path.
    /// e.g. given `/foo/bar/baz.js`, we store `baz` as the key in this map.
    module_names: HashMap<String, SourceId>,
}

impl DefaultResolver {
    pub fn new(source_manager: &SourceManager) -> Self {
        let mut module_names = HashMap::new();

        for i in 0..source_manager.num_sources() {
            let id = SourceId(i as u32);
            let name = source_manager.source_name(id);
            let name_str: String = name.into();

            let stem = Path::new(&name_str)
                .file_stem()
                .expect("Module filename must not be empty")
                .to_str()
                .expect("Module filename must be valid unicode");
            module_names.insert(stem.to_string(), id);
        }

        DefaultResolver { module_names }
    }
}

impl DependencyResolver for DefaultResolver {
    /// Default resoution for a dependency on another file from source file `file`.
    /// Paths starting with `.` are treated as relative.
    /// Return the ID of the resolved file if it could be resolved, else `None`.
    fn resolve_dependency(
        &self,
        lock: &GCLock,
        file: SourceId,
        path: &str,
        _kind: DependencyKind,
    ) -> Option<SourceId> {
        if path.starts_with('.') {
            // Relative path resolution
            let mut buf: PathBuf = lock.sm().source_name(file).into();
            buf.pop();
            buf.push(path);
            let mut filename = remove_dots(buf).display().to_string();
            lock.sm().lookup_name(&filename).or_else(|| {
                filename += ".js";
                lock.sm().lookup_name(&filename)
            })
        } else {
            self.module_names.get(path).copied()
        }
    }
}

/// Remove all non-leading `.` and `..` from the path, treating `..` as a parent directory.
fn remove_dots(buf: PathBuf) -> PathBuf {
    let mut result = PathBuf::new();
    let mut seen_non_dot = false;
    for (i, component) in buf.components().enumerate() {
        match component {
            Component::CurDir => {
                if i == 0 {
                    // Keep the leading `./` if necessary.
                    result.push(component);
                }
            }
            Component::ParentDir => {
                if seen_non_dot {
                    // Pop if we've seen a component that wasn't a `.` (i.e. it's useful to pop it).
                    result.pop();
                }
            }
            _ => {
                // All other components are simply moved over.
                seen_non_dot = true;
                result.push(component);
            }
        };
    }
    result
}
