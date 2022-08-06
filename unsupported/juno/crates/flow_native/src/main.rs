/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

use std::collections::hash_map::Entry;
use std::collections::HashMap;
use std::collections::HashSet;
use std::convert::TryFrom;
use std::fmt;
use std::fs::File;
use std::io::stdout;
use std::io::BufWriter;
use std::io::Write;
use std::path::Path;
use std::path::PathBuf;
use std::process::exit;
use std::rc::Rc;

use anyhow::Context;
use command_line::CommandLine;
use command_line::Opt;
use command_line::OptDesc;
use juno::ast;
use juno::ast::node_cast;
use juno::ast::NodeRc;
use juno::hparser;
use juno::hparser::ParserDialect;
use juno::resolve_dependency;
use juno::sema;
use juno::sema::DeclId;
use juno::sema::DeclKind;
use juno::sema::FunctionInfoId;
use juno::sema::LexicalScopeId;
use juno::sema::Resolution;
use juno::sema::SemContext;
use juno_support::atom_table::Atom;
use juno_support::atom_table::AtomU16;
use juno_support::source_manager::SourceId;
use juno_support::source_manager::SourceRange;
use juno_support::NullTerminatedBuf;

struct Options {
    /// Input file to parse.
    input_path: Opt<PathBuf>,

    /// Whether to emit #line statements.
    debug: Opt<bool>,
}

impl Options {
    pub fn new(cl: &mut CommandLine) -> Options {
        Options {
            input_path: Opt::<PathBuf>::new(
                cl,
                OptDesc {
                    desc: Some("'input-path'"),
                    min_count: 1,
                    ..Default::default()
                },
            ),

            debug: Opt::new_flag(
                cl,
                OptDesc {
                    short: Some("g"),
                    desc: Some("Emit #line statements"),
                    init: Some(false),
                    ..Default::default()
                },
            ),
        }
    }
}

struct FindEscapes {
    sem: Rc<SemContext>,
    escaped_decls: HashSet<DeclId>,
    cur_fun: FunctionInfoId,
}

impl<'gc> ast::Visitor<'gc> for FindEscapes {
    fn call(
        &mut self,
        lock: &'gc ast::GCLock,
        node: &'gc ast::Node<'gc>,
        _path: Option<ast::Path>,
    ) {
        use ast::*;

        match node {
            Node::FunctionExpression(FunctionExpression { body, .. })
            | Node::FunctionDeclaration(FunctionDeclaration { body, .. }) => {
                let scope_id = self.sem.node_scope(NodeRc::from_node(lock, body)).unwrap();
                self.cur_fun = self.sem.scope(scope_id).parent_function;
                node.visit_children(lock, self);
            }
            Node::Identifier(..) => {
                // Check if this resolves to an actual variable, as opposed to being an identifier
                // in a non-computed member expression, or object literal.
                let ident_decl = self.sem.ident_decl(&NodeRc::from_node(lock, node));
                if let Some(Resolution::Decl(decl_id)) = ident_decl {
                    let decl = self.sem.decl(decl_id);
                    match decl.kind {
                        DeclKind::UndeclaredGlobalProperty | DeclKind::GlobalProperty => {}
                        _ => {
                            // This is a local variable in some function. If it is being accessed
                            // outside that function, then it escapes.
                            let decl_fn = self.sem.scope(decl.scope).parent_function;
                            if decl_fn != self.cur_fun {
                                self.escaped_decls.insert(decl_id);
                            }
                        }
                    }
                }
            }
            _ => {
                node.visit_children(lock, self);
            }
        };
    }
}

/// A type used to represent the result of an intermediate computation stored in
/// a temporary variable in C++. It implements Display to print the name of that
/// variable.
#[derive(Copy, Clone, Debug, Eq, PartialEq, Hash)]
struct ValueId(usize);
impl fmt::Display for ValueId {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(f, "t{}", self.0)
    }
}

/// Represents an expression that can be on the left side of an assignment. The
/// LRef can be stored to and loaded from to implement update operators.
#[derive(Clone, Copy, Debug)]
enum LRef {
    MemberVal {
        object: ValueId,
        property: ValueId,
    },
    MemberLit {
        object: ValueId,
        property_name: AtomU16,
    },
    Var(DeclId),
}

impl LRef {
    /// Return the base object
    fn get_base_object(&self) -> Option<ValueId> {
        match self {
            LRef::MemberVal { object, .. } => Some(*object),
            LRef::MemberLit { object, .. } => Some(*object),
            LRef::Var(_) => None,
        }
    }
}

#[derive(Copy, Clone, Debug, Eq, PartialEq, Hash)]
struct CompilerString(u32);

/// A map from a string literal (Atom16) to a string index known at compile time.
/// At runtime it is an index in a per-compilation unit array, which resolves to
/// the actual runtime string.
#[derive(Debug, Default)]
struct CompilerStrings {
    /// Map from a string atom to a unique compiler string index.
    map: HashMap<AtomU16, CompilerString>,
    /// The strings in insertion order.
    ordered: Vec<AtomU16>,
}

impl CompilerStrings {
    /// Return the unique compiler string index corresponding to an Atom16.
    /// Either an existing index is returned, or a new compiler string is added.
    fn add(&mut self, atom: AtomU16) -> CompilerString {
        match self.map.entry(atom) {
            Entry::Occupied(o) => *o.get(),
            Entry::Vacant(v) => {
                let res =
                    CompilerString(u32::try_from(self.ordered.len()).expect("too many strings"));
                self.ordered.push(atom);
                v.insert(res);
                res
            }
        }
    }

    /// Return an iterator over the strings in insertion order.
    fn iter(&self) -> std::slice::Iter<'_, AtomU16> {
        self.ordered.iter()
    }
}

/// Splitting the func storage out of the Compiler struct only to placate the
/// borrower. This struct may be mutably borrowed simultaneously with another
/// field in [`Compiler`].
struct Output {
    /// The functions generated by the compiler.
    chunks: Vec<Vec<u8>>,
    /// The function currently being generated, index in [`funcs`].
    cur_chunk: usize,
    /// List of freed chunks.
    free_chunks: Vec<usize>,
}

impl Output {
    /// Write to the current chunk. Used via the `out!` macro. The output must be ASCII.
    fn write_ascii(&mut self, args: fmt::Arguments<'_>) {
        let buf = format!("{}", args);
        debug_assert!(buf.is_ascii(), "Output must be ASCII");
        self.chunks[self.cur_chunk]
            .write_all(buf.as_bytes())
            .unwrap();
    }

    /// Crete a new chunk, make it active, return its index.
    fn new_chunk(&mut self) -> usize {
        if let Some(freed) = self.free_chunks.pop() {
            self.cur_chunk = freed;
        } else {
            self.cur_chunk = self.chunks.len();
            self.chunks.push(Vec::new());
        }
        self.cur_chunk
    }

    /// Free a chunk when it is no longer needed.
    #[allow(dead_code)]
    fn free_chunk(&mut self, index: usize) {
        self.chunks[index].clear();
        self.free_chunks.push(index);
    }

    /// Append the context of chunk `src` into chunk `dest` and empty `src`.
    #[allow(dead_code)]
    fn append_chunk(&mut self, dest: usize, src: usize) {
        let mut tmp = std::mem::take(&mut self.chunks[src]);
        self.chunks[dest].append(&mut tmp);
    }

    /// Set the current chunk to the specified index.
    fn set_chunk(&mut self, prev: usize) {
        self.cur_chunk = prev;
    }
}

struct Compiler<'w> {
    writer: BufWriter<&'w mut dyn Write>,
    sem: Rc<SemContext>,
    output: Output,
    /// Whether to emit line statements or just comments.
    debug: bool,
    /// File of last emitted statement.
    last_stmt_file: SourceId,
    /// Start line of last emitted statement.
    last_stmt_line: u32,
    /// The number of ValueIds that have been created so far. This is also used
    /// to give a unique index to each one.
    num_values: usize,
    escaped_decls: HashSet<DeclId>,
    compiler_strings: CompilerStrings,
}

/// Print to the current function.
/// `$compiler` is a mutable reference to the Compiler struct.
/// `$arg` arguments follow the format pattern used by `format!`.
/// The output must be ASCII.
macro_rules! out {
    ($compiler:expr, $($arg:tt)*) => {{
        $compiler.output.write_ascii(format_args!($($arg)*));
    }}
}

impl Compiler<'_> {
    fn find_escapes<'gc>(
        sem: Rc<SemContext>,
        node: &'gc ast::Node<'gc>,
        lock: &'gc ast::GCLock,
    ) -> HashSet<DeclId> {
        let top_scope_id = sem.node_scope(NodeRc::from_node(lock, node)).unwrap();
        let top_fn_id = sem.scope(top_scope_id).parent_function;
        let mut find_escapes = FindEscapes {
            sem,
            escaped_decls: HashSet::new(),
            cur_fun: top_fn_id,
        };
        node.visit_children(lock, &mut find_escapes);
        find_escapes.escaped_decls
    }

    pub fn compile(
        debug: bool,
        out: &mut dyn Write,
        mut ctx: ast::Context,
        ast: NodeRc,
        sem: Rc<SemContext>,
    ) {
        let lock = ast::GCLock::new(&mut ctx);
        let escaped_decls = Self::find_escapes(Rc::clone(&sem), ast.node(&lock), &lock);
        let mut comp = Compiler {
            writer: BufWriter::new(out),
            sem,
            output: Output {
                chunks: vec![Vec::new()],
                cur_chunk: 0,
                free_chunks: Default::default(),
            },
            debug,
            last_stmt_file: SourceId::INVALID,
            last_stmt_line: 0,
            num_values: 0,
            escaped_decls,
            compiler_strings: Default::default(),
        };
        comp.gen_program(ast.node(&lock), &lock)
    }

    /// Returns a newly created unique ValueId.
    fn new_value(&mut self) -> ValueId {
        let result = ValueId(self.num_values);
        self.num_values += 1;
        result
    }

    fn gen_program<'gc>(&mut self, node: &'gc ast::Node<'gc>, lock: &'gc ast::GCLock) {
        use ast::*;
        out!(self, "#include \"FNRuntime.h\"\n");
        self.gen_context();

        let main_chunk = self.output.new_chunk();
        debug_assert!(main_chunk == 1);
        out!(self, "static void init_strings();\n");
        out!(self, "int main(){{\n");
        out!(self, "  init_strings();\n");
        let scope = self.sem.node_scope(NodeRc::from_node(lock, node)).unwrap();
        let Module { body, .. } = node_cast!(Node::Module, node);
        self.create_scope(scope);
        for stmt in body.iter() {
            self.gen_stmt(stmt, scope, lock);
        }
        out!(self, "return 0;\n}}");
        // Emit the compiler strings.
        out!(self, "static void init_strings() {{\n");
        for (index, atom) in self.compiler_strings.iter().enumerate() {
            let str = String::from_utf16_lossy(lock.str_u16(*atom));
            out!(self, "  fnAddCompilerString({:?}, {});\n", str, index);
        }
        out!(self, "}}\n");
        self.output.set_chunk(0);

        self.writer
            .write_all(self.output.chunks[0].as_slice())
            .expect("Error writing out program");

        for i in 2..self.output.chunks.len() {
            self.writer
                .write_all(self.output.chunks[i].as_slice())
                .expect("Error writing out program");
        }

        // Output the chunk containing main() last.
        self.writer
            .write_all(self.output.chunks[1].as_slice())
            .expect("Error writing out program");
    }

    fn gen_context(&mut self) {
        // Forward declare scope structs.
        for i in 0..self.sem.all_scopes().len() {
            out!(self, "struct Scope{};\n", i)
        }
        // Define scope structs.
        for (i, scope) in self.sem.all_scopes().iter().enumerate() {
            out!(self, "struct Scope{}{{\n", i);
            if let Some(parent) = scope.parent_scope {
                out!(self, "Scope{} *parent;\n", parent);
            }
            for decl in &scope.decls {
                if self.escaped_decls.contains(decl) {
                    out!(self, "FNValue var{}=FNValue::encodeUndefined();\n", decl);
                }
            }
            out!(self, "}};\n");
        }
    }

    fn param_list_for_arg_count(&mut self, count: usize) {
        out!(self, "void *parent_scope, FNValue js_this");
        for i in 0..count {
            out!(self, ", FNValue param{}", i)
        }
    }

    fn gen_call<'gc>(
        &mut self,
        callee: ValueId,
        this: ValueId,
        arguments: &'gc ast::NodeList<'gc>,
        scope: LexicalScopeId,
        lock: &'gc ast::GCLock,
    ) -> ValueId {
        // Evaluate each argument to get a list of ValueIds.
        let arguments: Vec<ValueId> = arguments
            .iter()
            .map(|arg| self.gen_expr(arg, scope, lock))
            .collect();
        let result = self.new_value();
        // Cast the function pointer based on the number of arguments.
        out!(self, "FNValue {}=reinterpret_cast<FNValue (*)(", result);
        self.param_list_for_arg_count(arguments.len());
        // Pass in the closure's environment as the first argument.
        out!(
            self,
            ")>({0}.getClosure()->func)({0}.getClosure()->env,{1}",
            callee,
            this
        );
        // Pass in the list of arguments.
        for arg in arguments {
            out!(self, ",{}", arg);
        }
        out!(self, ");");
        result
    }

    fn create_scope(&mut self, scope: LexicalScopeId) {
        out!(self, "Scope{0} *scope{0} = new Scope{0}();\n", scope);
        let scope = self.sem.scope(scope);
        for decl in &scope.decls {
            if !self.escaped_decls.contains(decl) {
                out!(self, "FNValue v{} = FNValue::encodeUndefined();\n", decl);
            }
        }
    }
    fn init_scope<'gc>(
        &mut self,
        node: &'gc ast::Node<'gc>,
        scope: LexicalScopeId,
        lock: &'gc ast::GCLock,
    ) -> LexicalScopeId {
        if let Some(new_scope) = self.sem.node_scope(NodeRc::from_node(lock, node)) {
            self.create_scope(new_scope);
            out!(self, "scope{}->parent = scope{};\n", new_scope, scope);
            new_scope
        } else {
            scope
        }
    }

    fn gen_function_exp<'gc>(
        &mut self,
        params: &'gc ast::NodeList<'gc>,
        block: &'gc ast::Node<'gc>,
        scope: LexicalScopeId,
        lock: &'gc ast::GCLock,
    ) -> ValueId {
        use ast::*;
        // Allocate a new buffer for this function.
        let prev_chunk = self.output.cur_chunk;
        let this_chunk = self.output.new_chunk();

        // Emit the declaration.
        self.output.set_chunk(0);
        out!(self, "static FNValue func_{}(", this_chunk);
        self.param_list_for_arg_count(params.len());
        out!(self, ");\n");

        // Emit the definition.
        self.output.set_chunk(this_chunk);
        out!(self, "static FNValue func_{}(", this_chunk);
        self.param_list_for_arg_count(params.len());
        out!(self, ") {{\n");
        out!(
            self,
            "\nScope{scope} *scope{scope} = (Scope{scope}*)parent_scope;"
        );
        let fn_scope = self.init_scope(block, scope, lock);
        // Store each parameter into its location in the current scope.
        for (i, param) in params.iter().enumerate() {
            let lref = self.new_lref(param, fn_scope, lock);
            let val = self.new_value();
            out!(self, "FNValue {}=param{};", val, i);
            self.gen_store(lref, val, scope);
        }
        let BlockStatement { body, .. } = node_cast!(Node::BlockStatement, block);
        for stmt in body.iter() {
            self.gen_stmt(stmt, fn_scope, lock);
        }
        out!(self, "\n  return FNValue::encodeUndefined();\n");
        out!(self, "}}\n");

        self.output.set_chunk(prev_chunk);

        let result = self.new_value();
        out!(
            self,
            "FNValue {}=FNValue::encodeClosure(new FNClosure{{(void(*)(void))func_{}, scope{scope}}});",
            result,
            this_chunk
        );
        result
    }

    /// Emit the code needed to access the variable declared by decl_id from the
    /// current scope.
    fn gen_var(&mut self, decl_id: DeclId, scope: LexicalScopeId) {
        // If the variable does not escape, access it locally. Otherwise, traverse the scope chain
        // to resolve the variable.
        if !self.escaped_decls.contains(&decl_id) {
            out!(self, "v{}", decl_id);
            return;
        }

        // Get information about the scope where the variable was declared.
        let decl_scope = self.sem.decl(decl_id).scope;
        let decl_depth = self.sem.scope(decl_scope).depth;

        // Get information about the function level scope.
        let func = self.sem.scope(scope).parent_function;
        let func_scope = self.sem.function(func).scopes[0];
        let func_depth = self.sem.scope(func_scope).depth;

        // If the variable was declared in the current function, then we can directly access its
        // scope. Otherwise, we can start traversing scopes from the function scope.
        let use_scope = if decl_depth >= func_depth {
            decl_scope
        } else {
            func_scope
        };
        let diff: u32 = self.sem.scope(use_scope).depth - decl_depth;
        out!(self, "scope{}->", use_scope);
        for _ in 0..diff {
            out!(self, "parent->");
        }
        out!(self, "var{}", decl_id);
    }

    fn gen_loop<'gc>(
        &mut self,
        test: Option<&'gc ast::Node<'gc>>,
        update: Option<&'gc ast::Node<'gc>>,
        body: &'gc ast::Node<'gc>,
        scope: LexicalScopeId,
        lock: &'gc ast::GCLock,
    ) {
        out!(self, "while(true){{");
        // The test may correspond to multiple lines of C++, so it needs to be
        // emitted separately in the body of the loop.
        if let Some(test) = test {
            let test = self.gen_expr(test, scope, lock);
            out!(self, "if(!{}.getBool()) break;", test)
        }
        self.gen_stmt(body, scope, lock);
        if let Some(update) = update {
            self.gen_expr(update, scope, lock);
        }
        out!(self, "}}");
    }

    /// Convert an identifier to a utf-16 Atom.
    fn ident_to_atom16(ident: Atom, lock: &ast::GCLock) -> AtomU16 {
        // Convert the identifier to utf-16 string.
        let prop_utf16: Vec<u16> = str::encode_utf16(lock.str(ident)).collect();
        // Add to the atom table.
        lock.atom_u16(prop_utf16)
    }

    /// Returns an LRef for the given AST node. For object property accesses, it
    /// evaluates the object and key expressions and stores their values to
    /// ensure that they are only evaluated once.
    fn new_lref<'gc>(
        &mut self,
        node: &'gc ast::Node<'gc>,
        scope: LexicalScopeId,
        lock: &'gc ast::GCLock,
    ) -> LRef {
        use ast::*;
        match node {
            Node::MemberExpression(MemberExpression {
                object,
                property: Node::Identifier(id),
                computed: false,
                ..
            }) => {
                let obj_val = self.gen_expr(object, scope, lock);
                LRef::MemberLit {
                    object: obj_val,
                    property_name: Self::ident_to_atom16(id.name, lock),
                }
            }
            Node::MemberExpression(MemberExpression {
                object,
                property: Node::StringLiteral(str_lit),
                computed: true,
                ..
            }) => {
                let obj_val = self.gen_expr(object, scope, lock);
                LRef::MemberLit {
                    object: obj_val,
                    property_name: str_lit.value,
                }
            }
            Node::MemberExpression(MemberExpression {
                object,
                property,
                computed: true,
                ..
            }) => {
                let obj_val = self.gen_expr(object, scope, lock);
                let prop_val = self.gen_expr(property, scope, lock);
                LRef::MemberVal {
                    object: obj_val,
                    property: prop_val,
                }
            }
            Node::Identifier(Identifier { name, .. }) => {
                let decl_id = match self.sem.ident_decl(&NodeRc::from_node(lock, node)) {
                    Some(Resolution::Decl(decl)) => decl,
                    _ => panic!("Unresolved variable: {}", lock.str(*name)),
                };
                let decl = self.sem.decl(decl_id);
                match decl.kind {
                    DeclKind::UndeclaredGlobalProperty | DeclKind::GlobalProperty => {
                        // Global properties are just member expressions where
                        // the object is the global object.
                        let obj_val = self.new_value();
                        out!(self, "FNValue {}=FNValue::encodeObject(global());", obj_val);
                        LRef::MemberLit {
                            object: obj_val,
                            property_name: Self::ident_to_atom16(*name, lock),
                        }
                    }
                    _ => LRef::Var(decl_id),
                }
            }
            _ => unimplemented!("Unimplemented lvalue: {:?}", node.variant()),
        }
    }

    /// Returns a tuple containing an optional base object and value when loading from an LRef
    fn gen_load(&mut self, lref: LRef, cur_scope: LexicalScopeId) -> ValueId {
        let res = self.new_value();
        match lref {
            LRef::Var(decl_id) => {
                out!(self, "FNValue {} = ", res);
                self.gen_var(decl_id, cur_scope);
                out!(self, ";");
                res
            }
            LRef::MemberLit {
                object,
                property_name,
            } => {
                let cs = self.compiler_strings.add(property_name);
                out!(
                    self,
                    "FNValue {} = {}.getObject()->getByName(fnCompilerUniqueString({}));",
                    res,
                    object,
                    cs.0
                );
                res
            }
            LRef::MemberVal { object, property } => {
                out!(
                    self,
                    "FNValue {} = {}.getObject()->getByVal({});",
                    res,
                    object,
                    property
                );
                res
            }
        }
    }

    /// Stores a value to the lref.
    fn gen_store(&mut self, lref: LRef, value: ValueId, cur_scope: LexicalScopeId) {
        match lref {
            LRef::Var(decl_id) => {
                self.gen_var(decl_id, cur_scope);
                out!(self, " = {};", value);
            }
            LRef::MemberLit {
                object,
                property_name,
            } => {
                let cs = self.compiler_strings.add(property_name);
                out!(
                    self,
                    "{}.getObject()->putByName(fnCompilerUniqueString({}), {});",
                    object,
                    cs.0,
                    value
                );
            }
            LRef::MemberVal { object, property } => {
                out!(
                    self,
                    "{}.getObject()->putByVal({}, {});",
                    object,
                    property,
                    value
                );
            }
        }
    }

    fn gen_expr<'gc>(
        &mut self,
        node: &'gc ast::Node<'gc>,
        scope: LexicalScopeId,
        lock: &'gc ast::GCLock,
    ) -> ValueId {
        use ast::*;
        match node {
            Node::FunctionExpression(FunctionExpression { params, body, .. }) => {
                self.gen_function_exp(params, body, scope, lock)
            }
            Node::ObjectExpression(ObjectExpression { properties, .. }) => {
                let object = self.new_value();
                // Allocate a new object.
                out!(
                    self,
                    "FNValue {}=FNValue::encodeObject(new FNObject());\n",
                    object
                );

                // Add each property and its corresponding value to the new
                // object.
                for prop in properties.iter() {
                    let Property {
                        key,
                        value,
                        computed,
                        ..
                    } = node_cast!(Node::Property, prop);
                    // Construct an lref for different types of keys. Strings keys are always
                    // treated as non-computed. Number keys are always treated as computed.
                    let lref = match (*computed, *key) {
                        (_, Node::StringLiteral(str)) => LRef::MemberLit {
                            object,
                            property_name: str.value,
                        },
                        (false, Node::Identifier(id)) => LRef::MemberLit {
                            object,
                            property_name: Self::ident_to_atom16(id.name, lock),
                        },
                        (_, Node::NumericLiteral(..)) | (true, _) => LRef::MemberVal {
                            object,
                            property: self.gen_expr(*key, scope, lock),
                        },
                        _ => panic!("Unexpected property key: {:?}", key.variant()),
                    };
                    let value = self.gen_expr(value, scope, lock);
                    self.gen_store(lref, value, scope);
                }
                object
            }
            Node::ArrayExpression(ArrayExpression { elements, .. }) => {
                let result = self.new_value();
                // Evaluate each element to get a list of ValueIds.
                let elements: Vec<ValueId> = elements
                    .iter()
                    .map(|elem| self.gen_expr(elem, scope, lock))
                    .collect();
                // Create a new array, and use the list of values as the
                // initializer.
                out!(
                    self,
                    "FNValue {}=FNValue::encodeObject(new FNArray({{",
                    result
                );
                for elem in elements {
                    out!(self, "{},", elem);
                }
                out!(self, "}}));");
                result
            }
            Node::CallExpression(CallExpression {
                callee, arguments, ..
            }) => {
                let (callee, this) = match callee {
                    Node::MemberExpression(..) => {
                        // For member expressions, the this parameter is the
                        // object being accessed.
                        let lref = self.new_lref(*callee, scope, lock);
                        let callee = self.gen_load(lref, scope);
                        // unwrap() must return an object for MemberExpression.
                        (callee, lref.get_base_object().unwrap())
                    }
                    _ => {
                        // For all other types of calls, the this parameter
                        // is undefined.
                        let undefined = self.new_value();
                        out!(self, "FNValue {}=FNValue::encodeUndefined();", undefined);
                        let callee = self.gen_expr(callee, scope, lock);
                        (callee, undefined)
                    }
                };
                self.gen_call(callee, this, arguments, scope, lock)
            }
            Node::NewExpression(NewExpression {
                callee, arguments, ..
            }) => {
                let callee = self.gen_expr(callee, scope, lock);
                let new_obj = self.new_value();
                // Create the new object and set its prototype.
                out!(self, "FNObject* {}=new FNObject();", new_obj,);
                out!(
                    self,
                    "{}->parent={}.getClosure()->getByName(FNPredefined::prototype).getObject();",
                    new_obj,
                    callee
                );
                let this = self.new_value();
                out!(self, "FNValue {}=FNValue::encodeObject({});", this, new_obj);
                // Call the function, using the new object as its "this" parameter.
                let ret = self.gen_call(callee, this, arguments, scope, lock);
                let result = self.new_value();
                out!(
                    self,
                    "FNValue {0}={1}.isObject()?{1}:{2};",
                    result,
                    ret,
                    this
                );
                result
            }
            Node::MemberExpression(..) | Node::Identifier(..) => {
                // Generate an LRef for the expression and load from it.
                let lref = self.new_lref(node, scope, lock);
                self.gen_load(lref, scope)
            }
            Node::AssignmentExpression(AssignmentExpression {
                left,
                right,
                operator: op,
                ..
            }) => {
                let lref = self.new_lref(left, scope, lock);
                let right = self.gen_expr(right, scope, lock);
                // Helper to apply the given mathematical operator to the left
                // and right sides.
                let mut update_op = |op: &str| {
                    let old_val = self.gen_load(lref, scope);
                    let new_val = self.new_value();
                    out!(
                        self,
                        "FNValue {}=FNValue::encodeNumber({}.getNumber(){}{}.getNumber());",
                        new_val,
                        old_val,
                        op,
                        right
                    );
                    new_val
                };
                // Determine the updated value based on the operator.
                let new_val = match op {
                    AssignmentExpressionOperator::Assign => right,
                    AssignmentExpressionOperator::PlusAssign => update_op("+"),
                    AssignmentExpressionOperator::MinusAssign => update_op("-"),
                    AssignmentExpressionOperator::ModAssign => update_op("%"),
                    AssignmentExpressionOperator::DivAssign => update_op("/"),
                    AssignmentExpressionOperator::MultAssign => update_op("*"),
                    _ => panic!("Unsupported assignment: {:?}", op),
                };
                // Store the updated value and return it as the result of this
                // expression.
                self.gen_store(lref, new_val, scope);
                new_val
            }
            Node::LogicalExpression(LogicalExpression {
                left,
                right,
                operator,
                ..
            }) => {
                let left = self.gen_expr(left, scope, lock);
                let result = self.new_value();
                out!(self, "FNValue {}={};", result, left);
                match operator {
                    LogicalExpressionOperator::And => {
                        out!(self, "if({}.getBool()){{", result);
                        let right = self.gen_expr(right, scope, lock);
                        out!(self, "{}={};", result, right);
                        out!(self, "}}");
                    }
                    LogicalExpressionOperator::Or => {
                        out!(self, "if(!{}.getBool()){{", result);
                        let right = self.gen_expr(right, scope, lock);
                        out!(self, "{}={};", result, right);
                        out!(self, "}}");
                    }
                    LogicalExpressionOperator::NullishCoalesce => {
                        out!(self, "if({0}.isNull() || {0}.isUndefined()){{", left);
                        let right = self.gen_expr(right, scope, lock);
                        out!(self, "{}={};}}", result, right);
                        out!(self, "");
                    }
                }
                result
            }
            Node::BinaryExpression(BinaryExpression {
                left,
                right,
                operator: op,
                ..
            }) => {
                let left = self.gen_expr(left, scope, lock);
                let right = self.gen_expr(right, scope, lock);
                let result = self.new_value();
                out!(self, "FNValue {}=", result);
                match op {
                    BinaryExpressionOperator::LooseEquals
                    | BinaryExpressionOperator::StrictEquals => {
                        // The isEqual helper just compares the tag and value on
                        // both sides to ensure they are eequal.
                        out!(
                            self,
                            "FNValue::encodeBool(FNValue::isEqual({},{}));",
                            left,
                            right
                        );
                    }
                    BinaryExpressionOperator::LooseNotEquals
                    | BinaryExpressionOperator::StrictNotEquals => {
                        out!(
                            self,
                            "FNValue::encodeBool(!FNValue::isEqual({},{}));",
                            left,
                            right
                        );
                    }
                    BinaryExpressionOperator::Less
                    | BinaryExpressionOperator::LessEquals
                    | BinaryExpressionOperator::Greater
                    | BinaryExpressionOperator::GreaterEquals => {
                        // Infer the operands to be doubles and compare them
                        // accordingly.
                        out!(
                            self,
                            "FNValue::encodeBool({}.getNumber(){}{}.getNumber());",
                            left,
                            op.as_str(),
                            right
                        );
                    }
                    BinaryExpressionOperator::LShift => out!(
                        self,
                        "FNValue::encodeNumber(truncateToInt32({}.getNumber()) << (truncateToInt32({}.getNumber()) & 0x1f));",
                        left,
                        right
                    ),
                    BinaryExpressionOperator::RShift => out!(
                        self,
                        "FNValue::encodeNumber(truncateToInt32({}.getNumber()) >> (truncateToInt32({}.getNumber()) & 0x1f));",
                        left,
                        right
                    ),
                    BinaryExpressionOperator::RShift3 => out!(
                        self,
                        "FNValue::encodeNumber((uint32_t)truncateToInt32({}.getNumber()) >> (truncateToInt32({}.getNumber()) & 0x1f));",
                        left,
                        right
                    ),
                    BinaryExpressionOperator::BitAnd
                    | BinaryExpressionOperator::BitOr
                    | BinaryExpressionOperator::BitXor => {
                        out!(
                            self,
                            "FNValue::encodeNumber(truncateToInt32({}.getNumber()){}truncateToInt32({}.getNumber()));",
                            left,
                            op.as_str(),
                            right
                        );
                    }
                    BinaryExpressionOperator::Mod => {
                        out!(
                            self,
                            "FNValue::encodeNumber(std::fmod({}.getNumber(),{}.getNumber()));",
                            left,
                            right
                        );
                    }
                    BinaryExpressionOperator::Plus
                    | BinaryExpressionOperator::Minus
                    | BinaryExpressionOperator::Mult
                    | BinaryExpressionOperator::Div => {
                        // Infer the operands to be doubles and directly apply
                        // the operator on them as a C++ operator. We may want
                        // to implement each operator separately later to
                        // control the semantics more precisely.
                        out!(
                            self,
                            "FNValue::encodeNumber({}.getNumber(){}{}.getNumber());",
                            left,
                            op.as_str(),
                            right
                        );
                    }
                    _ => panic!("Unsupported operator: {:?}", op),
                }
                result
            }
            Node::UnaryExpression(UnaryExpression {
                operator, argument, ..
            }) => {
                let argument = self.gen_expr(argument, scope, lock);
                let result = self.new_value();
                match operator {
                    UnaryExpressionOperator::Plus => out!(
                        self,
                        "FNValue {}=FNValue::encodeNumber({}.getNumber());",
                        result,
                        argument
                    ),
                    UnaryExpressionOperator::Minus => out!(
                        self,
                        "FNValue {}=FNValue::encodeNumber(-{}.getNumber());",
                        result,
                        argument
                    ),
                    UnaryExpressionOperator::BitNot => out!(
                        self,
                        "FNValue {}=FNValue::encodeNumber(~truncateToInt32({}.getNumber()));",
                        result,
                        argument
                    ),
                    UnaryExpressionOperator::Not => {
                        out!(
                            self,
                            "FNValue {}=FNValue::encodeBool(!{}.getBool());",
                            result,
                            argument
                        );
                    }
                    UnaryExpressionOperator::Typeof => {
                        out!(
                            self,
                            "FNValue {}=FNValue::encodeString(FNValue::typeOf({}));",
                            result,
                            argument
                        );
                    }
                    _ => panic!("Unsupported operator: {:?}", operator),
                }
                result
            }
            Node::UpdateExpression(UpdateExpression {
                operator,
                argument,
                prefix,
                ..
            }) => {
                let lref = self.new_lref(argument, scope, lock);
                let old_val = self.gen_load(lref, scope);
                let new_val = self.new_value();
                let op = match operator {
                    UpdateExpressionOperator::Increment => "+1",
                    UpdateExpressionOperator::Decrement => "-1",
                };
                // Calculate the new value using the operator being applied.
                out!(
                    self,
                    "FNValue {}=FNValue::encodeNumber({}.getNumber(){});",
                    new_val,
                    old_val,
                    op
                );
                // Store the updated value and return the old or new value,
                // depending on whether this is a postfix or prefix operator.
                self.gen_store(lref, new_val, scope);
                if *prefix { new_val } else { old_val }
            }
            Node::ThisExpression(..) => {
                let result = self.new_value();
                out!(self, "FNValue {}=js_this;", result);
                result
            }
            Node::NumericLiteral(NumericLiteral { value, .. }) => {
                let result = self.new_value();
                out!(self, "FNValue {}=FNValue::encodeNumber({});", result, value);
                result
            }
            Node::BooleanLiteral(BooleanLiteral { value, .. }) => {
                let result = self.new_value();
                out!(self, "FNValue {}=FNValue::encodeBool({});", result, value);
                result
            }
            Node::StringLiteral(StringLiteral { value, .. }) => {
                let cs = self.compiler_strings.add(*value);
                let result = self.new_value();
                out!(
                    self,
                    "FNValue {}=FNValue::encodeString(fnCompilerFNString({}));",
                    result,
                    cs.0
                );
                result
            }
            Node::NullLiteral(..) => {
                let res = self.new_value();
                out!(self, "FNValue {}=FNValue::encodeNull();", res);
                res
            }
            _ => unimplemented!("Unimplemented expression: {:?}", node.variant()),
        }
    }

    fn gen_stmt<'gc>(
        &mut self,
        node: &'gc ast::Node<'gc>,
        scope: LexicalScopeId,
        lock: &'gc ast::GCLock,
    ) {
        let r = *node.range();
        if r.file.is_valid()
            && r.start.line != 0
            && (r.file != self.last_stmt_file || r.start.line != self.last_stmt_line)
        {
            out!(
                self,
                "\n{} {}",
                if self.debug { "#line" } else { "// line" },
                r.start.line
            );
            if r.file != self.last_stmt_file {
                out!(self, " {:?}", lock.sm().source_name(r.file));
            }
            out!(self, "\n");
            self.last_stmt_file = r.file;
            self.last_stmt_line = r.start.line;
        }

        use ast::*;
        match node {
            Node::BlockStatement(BlockStatement { body, .. }) => {
                out!(self, "{{\n");
                let inner_scope = self.init_scope(node, scope, lock);
                for exp in body.iter() {
                    self.gen_stmt(exp, inner_scope, lock)
                }
                out!(self, "}}\n");
            }
            Node::VariableDeclaration(VariableDeclaration { declarations, .. }) => {
                for decl in declarations.iter() {
                    self.gen_stmt(decl, scope, lock)
                }
            }
            Node::VariableDeclarator(VariableDeclarator {
                init: init_opt,
                id: ident,
                ..
            }) => {
                if let Some(init) = init_opt {
                    // Initialize the variable with init.
                    let lref = self.new_lref(ident, scope, lock);
                    let init = self.gen_expr(init, scope, lock);
                    self.gen_store(lref, init, scope);
                }
            }
            Node::FunctionDeclaration(FunctionDeclaration {
                id: ident_opt,
                params,
                body,
                ..
            }) => {
                // Evaluate the function as a value.
                let fn_id = self.gen_function_exp(params, body, scope, lock);
                if let Some(ident) = ident_opt {
                    // Initialize the identifier for the function with the
                    // generated value.
                    let lref = self.new_lref(ident, scope, lock);
                    self.gen_store(lref, fn_id, scope);
                }
            }

            Node::ReturnStatement(ReturnStatement { argument, .. }) => match argument {
                Some(argument) => {
                    let argument = self.gen_expr(argument, scope, lock);
                    out!(self, "return {};", argument);
                }
                None => out!(self, "return FNValue::encodeUndefined()"),
            },
            Node::ExpressionStatement(ExpressionStatement {
                expression: exp, ..
            }) => {
                self.gen_expr(exp, scope, lock);
            }
            Node::WhileStatement(WhileStatement { test, body, .. }) => {
                self.gen_loop(Some(test), None, body, scope, lock);
            }
            Node::ForStatement(ForStatement {
                init,
                test,
                update,
                body,
                ..
            }) => {
                out!(self, "{{");
                let inner_scope = self.init_scope(node, scope, lock);
                if let Some(init) = init {
                    self.gen_stmt(init, inner_scope, lock);
                }
                self.gen_loop(*test, *update, body, inner_scope, lock);
                out!(self, "}}");
            }
            Node::IfStatement(IfStatement {
                test,
                consequent,
                alternate,
                ..
            }) => {
                let test = self.gen_expr(test, scope, lock);
                out!(self, "if({}.getBool()){{", test);
                self.gen_stmt(consequent, scope, lock);
                out!(self, "\n}}\nelse{{\n");
                if let Some(alt) = alternate {
                    self.gen_stmt(alt, scope, lock)
                }
                out!(self, "\n}}");
            }
            Node::TryStatement(TryStatement { block, handler, .. }) => {
                out!(self, "try {{");
                self.gen_stmt(block, scope, lock);
                out!(self, "}} catch (FNValue ex){{");
                let handler = if let Some(handler) = handler {
                    handler
                } else {
                    todo!("finally is not implemented");
                };
                let CatchClause { param, body, .. } = node_cast!(Node::CatchClause, handler);
                let new_scope = self.init_scope(handler, scope, lock);
                let BlockStatement { body, .. } = node_cast!(Node::BlockStatement, body);
                if let Some(param) = param {
                    // Initialize the catch parameter with the caught exception.
                    let lref = self.new_lref(param, new_scope, lock);
                    let ex_val = self.new_value();
                    out!(self, "FNValue {}=ex;", ex_val);
                    self.gen_store(lref, ex_val, scope);
                }
                for stmt in body.iter() {
                    self.gen_stmt(stmt, new_scope, lock);
                }
                out!(self, "}}");
            }
            Node::ThrowStatement(ThrowStatement { argument, .. }) => {
                let argument = self.gen_expr(argument, scope, lock);
                out!(self, "throw {};", argument);
            }
            _ => unimplemented!("Unimplemented statement: {:?}", node.variant()),
        }
    }
}

/// Read the specified file or stdin into a null terminated buffer.
fn read_file_or_stdin(input: &Path) -> anyhow::Result<NullTerminatedBuf> {
    if input == Path::new("-") {
        let stdin = std::io::stdin();
        let mut handle = stdin.lock();
        NullTerminatedBuf::from_reader(&mut handle).context("stdin")
    } else {
        let mut file = File::open(input).with_context(|| input.display().to_string())?;
        NullTerminatedBuf::from_file(&mut file).with_context(|| input.display().to_string())
    }
}

/// TransformStatus indicates whether there were parse or validation errors
/// when processing the input.
///
/// This may be a little confusing at first - why not just use std::Result like
/// a normal person??? The key is to understand that even though parse or
/// validation errors are called "errors", they are actually legitimately
/// expected output of a compiler. They are unlike, for example, a missing file
/// or an invalid URL. They have their own UI (that may display multiple errors,
/// colorize them, etc) and don't go through the std::error::Error flow.
enum TransformStatus {
    // Compilation completed successfully.
    Success,
    // There were parse or validation errors.
    Error,
}

fn run(opt: &Options) -> anyhow::Result<TransformStatus> {
    let mut ctx = ast::Context::new();
    let file_id = ctx.sm_mut().add_source(
        opt.input_path.display().to_string(),
        read_file_or_stdin(&*opt.input_path)?,
    );
    let buf = ctx.sm().source_buffer_rc(file_id);

    let parsed = hparser::ParsedJS::parse(
        hparser::ParserFlags {
            strict_mode: ctx.strict_mode(),
            enable_jsx: false,
            dialect: ParserDialect::JavaScript,
            store_doc_block: false,
        },
        &buf,
    );
    if let Some(e) = parsed.first_error() {
        ctx.sm().error(SourceRange::from_loc(file_id, e.0), e.1);
        return Ok(TransformStatus::Error);
    }

    // Convert to Juno AST.
    let (ast, sem) = {
        let resolver = resolve_dependency::DefaultResolver::new(ctx.sm());
        let lock = ast::GCLock::new(&mut ctx);
        let program = &node_cast!(ast::Node::Program, parsed.to_ast(&lock, file_id).unwrap());
        let module = ast::builder::Module::build_template(
            &lock,
            ast::template::Module {
                metadata: ast::TemplateMetadata {
                    phantom: Default::default(),
                    range: program.metadata.range,
                },
                body: program.body,
            },
        );
        (
            NodeRc::from_node(&lock, module),
            sema::resolve_module(&lock, module, SourceId(0), &resolver),
        )
    };

    if ctx.sm().num_errors() != 0 || ctx.sm().num_warnings() != 0 {
        eprintln!(
            "{} error(s), {} warning(s)",
            ctx.sm().num_errors(),
            ctx.sm().num_warnings()
        );
    }
    if ctx.sm().num_errors() != 0 {
        return Ok(TransformStatus::Error);
    }

    Compiler::compile(*opt.debug, &mut stdout(), ctx, ast, Rc::new(sem));
    Ok(TransformStatus::Success)
}

fn main() {
    let mut cl = CommandLine::new("Flow Native");
    let opt = Options::new(&mut cl);
    cl.parse_env_args();

    if let Err(e) = run(&opt) {
        eprintln!("{:#}", e);
        exit(1);
    }
}
