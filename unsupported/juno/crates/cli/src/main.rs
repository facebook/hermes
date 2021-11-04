/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

use anyhow::{self, ensure, Context, Error};
use juno::ast::{self, validate_tree, NodePtr, SourceRange};
use juno::gen_js;
use juno::hparser::{self, MagicCommentKind, ParsedJS};
use juno::sourcemap::merge_sourcemaps;
use pass::PassManager;
use sourcemap::SourceMap;
use std::fs::File;
use std::io::Write;
use std::path::{Path, PathBuf};
use std::process::exit;
use structopt::{clap::arg_enum, clap::AppSettings, clap::ArgGroup, StructOpt};
use support::NullTerminatedBuf;
use support::{fetchurl, Timer};
use url::{self, Url};

#[derive(StructOpt, Debug)]
#[structopt(group = ArgGroup::with_name("gen"))]
struct Gen {
    /// Dump the AST as JSON.
    #[structopt(long = "gen-ast", group = "gen")]
    ast: bool,
    /// Generate JavaScript source.
    #[structopt(long = "gen-js", group = "gen")]
    js: bool,
}

arg_enum! {
    #[derive(Debug, Copy, Clone, PartialEq)]
    enum InputSourceMap {
        Ignore,
        Auto,
    }
}

#[derive(Debug, StructOpt)]
#[structopt(name = "juno", about = "A JavaScript Compiler", setting = AppSettings::DeriveDisplayOrder)]
struct Opt {
    /// Disable pretty printing.
    #[structopt(long)]
    no_pretty: bool,

    /// Select what to emit.
    #[structopt(flatten)]
    gen: Gen,

    /// Input file to parse.
    #[structopt(parse(from_os_str))]
    input_path: PathBuf,

    /// Path to output to.
    /// Defaults to `-`, which is `stdout`.
    #[structopt(long = "out", short = "o", default_value = "-", parse(from_os_str))]
    output_path: PathBuf,

    /// Whether to output a source map.
    /// The source map will be merged with an input source map if provided.
    /// Can only be used when generating JS.
    #[structopt(long)]
    sourcemap: bool,

    /// Base URL to prepend to relative URLs.
    #[structopt(long)]
    base_url: Option<Url>,

    /// How to handle the source map directives.
    #[structopt(long, possible_values = &InputSourceMap::variants(),
                case_insensitive = true, default_value="Auto")]
    input_source_map: InputSourceMap,

    /// Whether to run optimization passes.
    #[structopt(short = "O")]
    optimize: bool,

    /// Enable strict mode.
    #[structopt(long)]
    strict_mode: bool,

    /// Warn about undefined variables in strict mode functions.
    #[structopt(long)]
    warn_undefined: bool,

    /// Measure and print times.
    #[structopt(long = "Xtime")]
    xtime: bool,
}

impl Opt {
    /// Ensure the arguments are valid.
    /// Return `Err` if there are any conflicts.
    fn validate(&self) -> anyhow::Result<()> {
        if self.sourcemap {
            ensure!(
                self.output_path != Path::new("-"),
                "Source map requires an output path",
            );
            ensure!(self.gen.js, "Source map requires JS output",);
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
    opt: &Opt,
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

/// Generate the specified output, if any.
/// Returns whether any output was generated.
fn gen_output(
    opt: &Opt,
    ctx: &mut ast::Context,
    root: NodePtr,
    input_map: &Option<SourceMap>,
) -> anyhow::Result<bool> {
    let out: Box<dyn Write> = if opt.output_path == Path::new("-") {
        Box::new(std::io::stdout())
    } else {
        Box::new(
            File::create(&opt.output_path)
                .with_context(|| opt.output_path.display().to_string())?,
        )
    };

    let final_ast = if opt.optimize {
        let pm = PassManager::standard();
        pm.run(ctx, root)
    } else {
        root
    };

    if opt.gen.ast {
        ast::dump_json(
            out,
            ctx,
            &final_ast,
            if opt.no_pretty {
                ast::Pretty::No
            } else {
                ast::Pretty::Yes
            },
        )?;
        Ok(true)
    } else if opt.gen.js {
        let generated_map = gen_js::generate(
            out,
            ctx,
            &final_ast,
            if opt.no_pretty {
                gen_js::Pretty::No
            } else {
                gen_js::Pretty::Yes
            },
        )?;
        if opt.sourcemap {
            // Workaround because `PathBuf` doesn't have a way to append an extension,
            // only to replace the existing one.
            let mut path = opt.output_path.clone().into_os_string();
            path.push(".map");
            let sourcemap_file = File::create(PathBuf::from(path))?;
            let merged_map = match input_map {
                None => generated_map,
                Some(input_map) => merge_sourcemaps(input_map, &generated_map),
            };
            merged_map.to_writer(sourcemap_file)?;
        }
        Ok(true)
    } else {
        Ok(false)
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

fn run(opt: &Opt) -> anyhow::Result<TransformStatus> {
    opt.validate()?;

    let mut ctx = ast::Context::new();

    // Propagate flags.
    if opt.strict_mode {
        ctx.enable_strict_mode();
    }
    ctx.warn_undefined = opt.warn_undefined;

    // Read the input into memory.
    let input = opt.input_path.as_path();
    let file_id = ctx
        .sm_mut()
        .add_source(input.display().to_string(), read_file_or_stdin(input)?);
    let buf = ctx.sm().source_buffer_rc(file_id);

    // Start measuring time.
    let mut timer = Timer::new();

    // Parse.
    let parsed = hparser::ParsedJS::parse(
        hparser::ParserFlags {
            strict_mode: ctx.strict_mode(),
            ..Default::default()
        },
        &buf,
    );
    timer.mark("Parse");
    if let Some(e) = parsed.first_error() {
        ctx.sm().error(SourceRange::from_loc(file_id, e.0), e.1);
        return Ok(TransformStatus::Error);
    }

    // Extract the optional source mapping URL.
    let sm_url = if opt.input_source_map != InputSourceMap::Ignore {
        parse_magic_url(&parsed, MagicCommentKind::SourceMappingUrl, opt)?
    } else {
        None
    };

    let ast = {
        // Convert to Juno AST.
        let gc = ast::GCContext::new(&mut ctx);
        NodePtr::from_node(&gc, parsed.to_ast(&gc, file_id).unwrap())
    };
    // We don't need the original parser anymore.
    drop(parsed);
    timer.mark("Cvt");

    validate_tree(&mut ctx, &ast).with_context(|| input.display().to_string())?;
    timer.mark("Validate AST");

    // Fetch and parse the source map before we generate the output.
    let source_map = sm_url.map(load_source_map).transpose()?;

    // Generate output.
    if gen_output(opt, &mut ctx, ast, &source_map)? {
        timer.mark("Gen");
    }

    // Drop the AST. We are doing it explicitly just to measure the time.
    drop(ctx);
    timer.mark("Drop");

    // Optionally print elapsed times.
    if opt.xtime {
        print!("{:#}", timer);
    }

    Ok(TransformStatus::Success)
}

fn main() {
    let opt: Opt = Opt::from_args();

    match run(&opt) {
        Ok(TransformStatus::Success) => {}
        Ok(TransformStatus::Error) => exit(1),
        Err(e) => {
            eprintln!("{:#}", e);
            exit(1);
        }
    }
}
