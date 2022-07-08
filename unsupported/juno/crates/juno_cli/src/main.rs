/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

use anyhow::ensure;
use anyhow::Context;
use anyhow::Error;
use command_line::CommandLine;
use command_line::Hidden;
use command_line::Opt;
use command_line::OptDesc;
use juno::ast;
use juno::ast::node_cast;
use juno::ast::validate_tree;
use juno::ast::NodeRc;
use juno::ast::SourceRange;
use juno::gen_js;
use juno::hparser;
use juno::hparser::MagicCommentKind;
use juno::hparser::ParsedJS;
use juno::hparser::ParserDialect;
use juno::resolve_dependency;
use juno::sema;
use juno::sema::SemContext;
use juno::sourcemap::merge_sourcemaps;
use juno_pass::PassManager;
use juno_support::fetchurl;
use juno_support::source_manager::SourceId;
use juno_support::HeapSize;
use juno_support::NullTerminatedBuf;
use juno_support::Timer;
use sourcemap::SourceMap;
use std::collections::HashMap;
use std::fs::File;
use std::io::Write;
use std::ops::DerefMut;
use std::path::Path;
use std::path::PathBuf;
use std::process::exit;
use std::rc::Rc;
use std::str::FromStr;
use url::Url;

#[derive(Debug, Copy, Clone, PartialEq)]
enum Gen {
    /// Dump the Semantic resolution information.
    Sema,
    /// Dump the AST as JSON.
    Ast,
    /// Generate JavaScript source.
    Js,
    /// Generate JavaScript source with annotations about variable resolution.
    ResolvedJs,
}

#[derive(Debug, Copy, Clone, PartialEq)]
enum InputSourceMap {
    Ignore,
    Auto,
}

struct Options {
    /// Enable pretty printing.
    pretty: Opt<bool>,

    /// Select what to emit.
    gen: Opt<Gen>,

    /// Perform AST validation.
    validate_ast: Opt<bool>,

    /// Perform semantic analysis.
    sema: Opt<bool>,

    /// Input file to parse.
    input_path: Opt<PathBuf>,

    /// Path to output to.
    /// Defaults to `-`, which is `stdout`.
    // #[structopt(long = "out", short = "o", default_value = "-", parse(from_os_str))]
    output_path: Opt<PathBuf>,

    /// Whether to output a source map.
    /// The source map will be merged with an input source map if provided.
    /// Can only be used when generating JS.
    sourcemap: Opt<bool>,

    /// Base URL to prepend to relative URLs.
    base_url: Opt<Option<Url>>,

    /// How to handle the source map directives.
    input_source_map: Opt<InputSourceMap>,

    /// Whether to run optimization passes.
    optimize: Opt<bool>,

    /// Whether to run strip flow types.
    strip_flow: Opt<bool>,

    /// Whether to force a space after the `async` keyword in arrow functions.
    force_async_arrow_space: Opt<bool>,

    /// Whether to emit the doc block when generating JS.
    /// The doc block contains every comment prior to the first non-directive token in the file.
    emit_doc_block: Opt<bool>,

    /// Whether to use double quotes on string literals.
    double_quote_strings: Opt<bool>,

    /// Whether to run the parsed AST.
    run: Opt<bool>,

    /// Control the recognized JavaScript dialect.
    dialect: Opt<ParserDialect>,

    /// Enable JSX parsing.
    jsx: Opt<bool>,

    /// Enable strict mode.
    strict_mode: Opt<bool>,

    /// Warn about undefined variables in strict mode functions.
    warn_undefined: Opt<bool>,

    /// Measure and print times.
    xtime: Opt<bool>,

    /// Measure and print memory.
    xmem: Opt<bool>,
}

impl Options {
    pub fn new(cl: &mut CommandLine) -> Options {
        let input_cat = cl.add_category("Input Options", None);
        let output_cat = cl.add_category("Output Options", None);

        Options {
            pretty: Opt::new_bool(
                cl,
                OptDesc {
                    long: Some("pretty"),
                    desc: Some("Enable pretty printing (default: on)."),
                    init: Some(true),
                    category: output_cat,
                    ..Default::default()
                },
            ),
            gen: Opt::new_enum(
                cl,
                OptDesc {
                    desc: Some("Choose generated output:"),
                    values: Some(&[
                        ("gen-sema", Gen::Sema, "Dump the Sema data."),
                        ("gen-ast", Gen::Ast, "Dump the AST as JSON."),
                        ("gen-js", Gen::Js, "Generate JavaScript source."),
                        (
                            "gen-resolved-js",
                            Gen::ResolvedJs,
                            "Generate resolution information.",
                        ),
                    ]),
                    category: output_cat,
                    ..Default::default()
                },
            ),
            validate_ast: Opt::new_bool(
                cl,
                OptDesc {
                    long: Some("validate-ast"),
                    desc: Some("Perform AST validation (default: on)."),
                    init: Some(true),
                    ..Default::default()
                },
            ),
            sema: Opt::new_bool(
                cl,
                OptDesc {
                    long: Some("sema"),
                    desc: Some("Perform semantic analysis (default: on)."),
                    init: Some(true),
                    ..Default::default()
                },
            ),
            input_path: Opt::<PathBuf>::new(
                cl,
                OptDesc {
                    desc: Some("'input-path'"),
                    min_count: 1,
                    list: true,
                    ..Default::default()
                },
            ),
            output_path: Opt::<PathBuf>::new(
                cl,
                OptDesc {
                    long: Some("out"),
                    short: Some("o"),
                    desc: Some("Path to output to. Defaults to `-`, which is `stdout`"),
                    init: Some(PathBuf::from_str("-").unwrap()),
                    value_desc: Some("output path"),
                    category: output_cat,
                    ..Default::default()
                },
            ),
            sourcemap: Opt::new_bool(
                cl,
                OptDesc {
                    long: Some("sourcemap"),
                    desc: Some("Whether to generate a source map."),
                    category: output_cat,
                    ..Default::default()
                },
            ),
            base_url: Opt::<Option<Url>>::new_optional(
                cl,
                OptDesc {
                    long: Some("base-url"),
                    desc: Some("Base URL to prepend to relative URLs."),
                    value_desc: Some("URL"),
                    category: input_cat,
                    ..Default::default()
                },
            ),
            input_source_map: Opt::new_enum(
                cl,
                OptDesc {
                    long: Some("input-source-map"),
                    desc: Some("How to handle the source map directives (default: auto)."),
                    values: Some(&[
                        (
                            "ignore",
                            InputSourceMap::Ignore,
                            "Ignore //# sourceMappingURL directives",
                        ),
                        (
                            "auto",
                            InputSourceMap::Auto,
                            "Recognize //# sourceMappingURL directives",
                        ),
                    ]),
                    category: input_cat,
                    ..Default::default()
                },
            ),
            optimize: Opt::new_flag(
                cl,
                OptDesc {
                    short: Some("O"),
                    desc: Some("Run optimization passes."),
                    ..Default::default()
                },
            ),
            strip_flow: Opt::new_bool(
                cl,
                OptDesc {
                    long: Some("strip-flow"),
                    desc: Some("Strip flow types"),
                    ..Default::default()
                },
            ),
            force_async_arrow_space: Opt::new_bool(
                cl,
                OptDesc {
                    long: Some("force-async-arrow-space"),
                    desc: Some("Force a space after the `async` keyword in arrow functions."),
                    ..Default::default()
                },
            ),
            emit_doc_block: Opt::new_bool(
                cl,
                OptDesc {
                    long: Some("emit-doc-block"),
                    desc: Some(
                        "Pass through the doc block from the original files when generating JS",
                    ),
                    ..Default::default()
                },
            ),
            double_quote_strings: Opt::new_bool(
                cl,
                OptDesc {
                    long: Some("double-quote-strings"),
                    desc: Some(
                        "When generating JS, use double quotes as the string literal delimiters",
                    ),
                    ..Default::default()
                },
            ),
            run: Opt::new_flag(
                cl,
                OptDesc {
                    long: Some("run"),
                    desc: Some("Run the parsed AST"),
                    ..Default::default()
                },
            ),
            dialect: Opt::new_enum(
                cl,
                OptDesc {
                    long: Some("dialect"),
                    desc: Some("Control the recognized JavaScript dialect (default: js)."),
                    values: Some(&[
                        ("js", ParserDialect::JavaScript, "JavaScript"),
                        ("flow", ParserDialect::Flow, "Flow"),
                        (
                            "flow-unambiguous",
                            ParserDialect::FlowUnambiguous,
                            "Flow unambiguous",
                        ),
                        (
                            "flow-detect",
                            ParserDialect::FlowDetect,
                            "Detect @flow pragma, otherwise only parse unambiguous Flow",
                        ),
                        ("ts", ParserDialect::TypeScript, "TypeScript"),
                    ]),
                    init: Some(ParserDialect::JavaScript),
                    category: input_cat,
                    ..Default::default()
                },
            ),
            jsx: Opt::new_bool(
                cl,
                OptDesc {
                    long: Some("jsx"),
                    desc: Some("Enable JSX parsing."),
                    category: input_cat,
                    ..Default::default()
                },
            ),
            strict_mode: Opt::new_bool(
                cl,
                OptDesc {
                    long: Some("strict-mode"),
                    desc: Some("Enable strict mode."),
                    category: input_cat,
                    ..Default::default()
                },
            ),
            warn_undefined: Opt::new_bool(
                cl,
                OptDesc {
                    long: Some("warn-undefined"),
                    desc: Some("Warn about undefined variables in strict mode functions."),
                    ..Default::default()
                },
            ),
            xtime: Opt::new_bool(
                cl,
                OptDesc {
                    long: Some("Xtime"),
                    desc: Some("Measure and print times."),
                    hidden: Hidden::Yes,
                    ..Default::default()
                },
            ),
            xmem: Opt::new_bool(
                cl,
                OptDesc {
                    long: Some("Xmem"),
                    desc: Some("Measure and print memory usage."),
                    hidden: Hidden::Yes,
                    ..Default::default()
                },
            ),
        }
    }

    /// Ensure the arguments are valid.
    /// Return `Err` if there are any conflicts.
    fn validate(&self) -> anyhow::Result<()> {
        if *self.sourcemap {
            ensure!(
                *self.output_path != Path::new("-"),
                "Source map requires an output path",
            );
            ensure!(*self.gen == Gen::Js, "Source map requires JS output");
        }
        Ok(())
    }
}

/// Read the specified file or stdin into a null terminated buffer.
fn read_file_or_stdin(input: &Path) -> anyhow::Result<NullTerminatedBuf> {
    if input == Path::new("-") {
        let stdin = std::io::stdin();
        let mut handle = stdin.lock();
        Ok(NullTerminatedBuf::from_reader(&mut handle).context("stdin")?)
    } else {
        let mut file = File::open(input).with_context(|| input.display().to_string())?;
        Ok(NullTerminatedBuf::from_file(&mut file).with_context(|| input.display().to_string())?)
    }
}

/// If there is a magic comment of the specified type, attempt to parse it as an URL.
fn parse_magic_url(
    parsed: &ParsedJS,
    kind: MagicCommentKind,
    opt: &Options,
) -> Result<Option<Url>, Error> {
    parsed
        .magic_comment(kind)
        .map(|s| Url::options().base_url(opt.base_url.as_ref()).parse(s))
        .transpose()
        .with_context(|| format!("Error parsing {}", kind.name()))
}

/// Consume the URL and fetch and parse the source map from it.
/// The URL is consumed to ensure that it is freed immediately after it is no
/// longer needed. Data URLs can potentially contain megabytes of data.
fn load_source_map(url: Url) -> anyhow::Result<SourceMap> {
    fetchurl::fetch_url(&url, Default::default())
        .with_context(|| format!("Source map: {}", &url))
        .and_then(|data| {
            SourceMap::from_slice(data.as_ref())
                .with_context(|| format!("Source map: {}: error parsing", &url))
        })
}

/// Convert a `Program` (script) AST node to a `Module` with an identical body.
fn script_to_module<'gc>(
    lock: &'gc ast::GCLock,
    program: &'gc ast::Program<'gc>,
) -> &'gc ast::Node<'gc> {
    ast::builder::Module::build_template(
        lock,
        ast::template::Module {
            metadata: ast::TemplateMetadata {
                phantom: Default::default(),
                range: program.metadata.range,
            },
            body: program.body,
        },
    )
}

/// Generate the specified output, if any.
/// Returns whether any output was generated.
fn gen_output(
    opt: &Options,
    ctx: &mut ast::Context,
    sem: Option<&SemContext>,
    js_module: &ParsedJSModule,
) -> anyhow::Result<bool> {
    let output_path = &*opt.output_path;
    let mut out: Box<dyn Write> = if output_path == Path::new("-") {
        Box::new(std::io::stdout())
    } else {
        Box::new(File::create(output_path).with_context(|| output_path.display().to_string())?)
    };

    let final_ast = if *opt.strip_flow {
        PassManager::strip_flow().run(ctx, js_module.ast.clone())
    } else {
        js_module.ast.clone()
    };

    let final_ast = if *opt.optimize {
        PassManager::standard().run(ctx, final_ast)
    } else {
        final_ast
    };

    if *opt.run {
        juno_eval::run(&final_ast);
        return Ok(true);
    }

    match *opt.gen {
        Gen::Ast => {
            ast::dump_json(
                out,
                ctx,
                &final_ast,
                if !*opt.pretty {
                    ast::Pretty::No
                } else {
                    ast::Pretty::Yes
                },
            )?;
            Ok(true)
        }
        Gen::Js | Gen::ResolvedJs => {
            let generated_map = gen_js::generate(
                out.deref_mut(),
                ctx,
                &final_ast,
                gen_js::Opt {
                    pretty: if *opt.pretty {
                        gen_js::Pretty::Yes
                    } else {
                        gen_js::Pretty::No
                    },
                    annotation: match sem {
                        Some(sem) if *opt.gen == Gen::ResolvedJs => gen_js::Annotation::Sem(sem),
                        _ => gen_js::Annotation::No,
                    },
                    force_async_arrow_space: *opt.force_async_arrow_space,
                    doc_block: js_module.doc_block.clone(),
                    quote: if *opt.double_quote_strings {
                        gen_js::QuoteChar::Double
                    } else {
                        gen_js::QuoteChar::Single
                    },
                },
            )?;
            if *opt.sourcemap {
                // Workaround because `PathBuf` doesn't have a way to append an extension,
                // only to replace the existing one.
                let mut path = output_path.clone().into_os_string();
                path.push(".map");
                let sourcemap_file = File::create(PathBuf::from(&path))?;
                let merged_map = match &js_module.source_map {
                    None => generated_map,
                    Some(input_map) => merge_sourcemaps(input_map, &generated_map),
                };
                merged_map.to_writer(sourcemap_file)?;
                write!(out, "\n//# sourceMappingURL={}", path.to_str().unwrap())?;
            }
            Ok(true)
        }
        Gen::Sema => {
            if let Some(sem) = sem {
                sem.dump(&ast::GCLock::new(ctx));
            }
            Ok(true)
        }
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
/// colorize them, etc) and don't go throw the std::error::Error flow.
enum TransformStatus {
    // Compilation completed successfully.
    Success,
    // There were parse or validation errors.
    Error,
}

/// Parsed JS file with its associated sourcemap.
struct ParsedJSModule {
    id: SourceId,
    /// AST node, may be either `Program` or `Module`.
    ast: NodeRc,
    source_map: Option<SourceMap>,
    /// Doc block for the file if it exists.
    doc_block: Option<Rc<String>>,
}

fn run(opt: &Options) -> anyhow::Result<TransformStatus> {
    opt.validate()?;

    let mut ctx = ast::Context::new();

    // Propagate flags.
    if *opt.strict_mode {
        ctx.enable_strict_mode();
    }
    ctx.warn_undefined = *opt.warn_undefined;

    // Start measuring time.
    let mut timer = Timer::new();

    // Read the input into memory.
    let input_paths = opt.input_path.values();

    let mut js_modules = HashMap::<SourceId, ParsedJSModule>::new();

    for path in input_paths {
        let input = path.as_path();
        let file_id = ctx
            .sm_mut()
            .add_source(input.display().to_string(), read_file_or_stdin(input)?);
        let buf = ctx.sm().source_buffer_rc(file_id);

        // Parse.
        let parsed = hparser::ParsedJS::parse(
            hparser::ParserFlags {
                strict_mode: ctx.strict_mode(),
                enable_jsx: *opt.jsx,
                dialect: *opt.dialect,
                store_doc_block: *opt.emit_doc_block,
            },
            &buf,
        );
        timer.mark("Parse");
        if let Some(e) = parsed.first_error() {
            ctx.sm().error(SourceRange::from_loc(file_id, e.0), e.1);
            return Ok(TransformStatus::Error);
        }

        // Extract the optional source mapping URL.
        let sm_url = if *opt.input_source_map != InputSourceMap::Ignore {
            parse_magic_url(&parsed, MagicCommentKind::SourceMappingUrl, opt)?
        } else {
            None
        };

        let ast = {
            // Convert to Juno AST.
            let lock = ast::GCLock::new(&mut ctx);
            match parsed.to_ast(&lock, file_id) {
                None => return Ok(TransformStatus::Error),
                Some(program) => {
                    if input_paths.len() > 1 {
                        NodeRc::from_node(
                            &lock,
                            script_to_module(&lock, node_cast!(ast::Node::Program, program)),
                        )
                    } else {
                        NodeRc::from_node(&lock, program)
                    }
                }
            }
        };
        let doc_block = parsed.get_doc_block().map(|s| Rc::new(s.to_string()));
        // We don't need the original parser anymore.
        drop(parsed);
        timer.mark("Cvt");

        if *opt.validate_ast {
            validate_tree(&mut ctx, &ast).with_context(|| input.display().to_string())?;
            timer.mark("Validate AST");
        }

        // Fetch and parse the source map before we generate the output.
        let source_map = sm_url.map(load_source_map).transpose()?;

        js_modules.insert(
            file_id,
            ParsedJSModule {
                id: file_id,
                ast,
                source_map,
                doc_block,
            },
        );
    }

    if js_modules.len() == 1 {
        let js_module = js_modules.into_values().next().unwrap();
        let sem = if *opt.sema {
            let lock = ast::GCLock::new(&mut ctx);
            let sem = sema::resolve_program(&lock, js_module.id, js_module.ast.node(&lock));
            if lock.sm().num_errors() != 0 || lock.sm().num_warnings() != 0 {
                eprintln!(
                    "{} error(s), {} warning(s)",
                    lock.sm().num_errors(),
                    lock.sm().num_warnings()
                );
            }
            if lock.sm().num_errors() != 0 {
                return Ok(TransformStatus::Error);
            }

            timer.mark("Sema");
            Some(sem)
        } else {
            None
        };

        // Generate output.
        if gen_output(opt, &mut ctx, sem.as_ref(), &js_module)? {
            timer.mark("Gen");
        }
    } else {
        // Show information about semantic resolution for all modules if requested.
        if *opt.sema {
            println!("{} modules", js_modules.len());
            let mut sems = Vec::new();
            let resolver = resolve_dependency::DefaultResolver::new(ctx.sm());
            for module in js_modules.into_values() {
                let sem;
                {
                    let lock = ast::GCLock::new(&mut ctx);
                    sem = sema::resolve_module(&lock, module.ast.node(&lock), module.id, &resolver);

                    let source_name = lock.sm().source_name(module.id);
                    println!("Module: {}", source_name);
                    println!(
                        "{} error(s), {} warning(s)",
                        lock.sm().num_errors(),
                        lock.sm().num_warnings()
                    );
                    if lock.sm().num_errors() != 0 {
                        return Ok(TransformStatus::Error);
                    }
                }
                // Generate output.
                if gen_output(opt, &mut ctx, Some(&sem), &module)? {
                    timer.mark("Gen");
                }
                sems.push(sem);
            }
            timer.mark("Sema");

            drop(sems);
            timer.mark("Drop Sema");
        }
    }

    // Optionally print memory usage.
    if *opt.xmem {
        println!("Context size:  {} MB", ctx.heap_size() / 1_000_000);
        println!("Storage size:  {} MB", ctx.storage_size() / 1_000_000);
        println!("# nodes:       {}", ctx.num_nodes());
    }

    // Drop the AST. We are doing it explicitly just to measure the time.
    drop(ctx);
    timer.mark("Drop");

    // Optionally print elapsed times.
    if *opt.xtime {
        print!("{:#}", timer);
    }

    Ok(TransformStatus::Success)
}

fn main() {
    let mut cl = CommandLine::new("A JavaScript compiler");
    let opt = Options::new(&mut cl);
    cl.parse_env_args();

    match run(&opt) {
        Ok(TransformStatus::Success) => {}
        Ok(TransformStatus::Error) => exit(1),
        Err(e) => {
            eprintln!("{:#}", e);
            exit(1);
        }
    }
}
