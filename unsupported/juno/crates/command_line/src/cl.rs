/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

use crate::opt::*;
use crate::parser::CommandLineIntent;
use crate::parser::Parser;
use std::fmt::Write;
use std::process::exit;
use std::rc::Rc;

#[derive(Debug)]
struct OptionCategory {
    name: String,
    desc: Option<String>,
}

#[derive(Default)]
pub struct CommandLine {
    desc: String,
    opts: Vec<Rc<dyn CLOption>>,
    categories: Vec<OptionCategory>,

    help: Option<Opt<bool>>,
    help_hidden: Option<Opt<bool>>,
}

impl CommandLine {
    pub fn new<S: Into<String>>(desc: S) -> Self {
        let mut res = CommandLine {
            desc: desc.into(),
            opts: Default::default(),
            categories: Default::default(),
            help: None,
            help_hidden: None,
        };

        res.add_category("Generic Options", None);

        res.help = Some(Opt::new_flag(
            &mut res,
            OptDesc {
                long: Some("help"),
                short: Some("h"),
                desc: Some("Display available options (-help-hidden for more)"),
                ..Default::default()
            },
        ));
        res.help_hidden = Some(Opt::new_flag(
            &mut res,
            OptDesc {
                long: Some("help-hidden"),
                hidden: Hidden::ReallyHidden,
                ..Default::default()
            },
        ));

        res
    }

    pub fn add_category<S: Into<String>>(&mut self, name: S, description: Option<S>) -> usize {
        self.categories.push(OptionCategory {
            name: name.into(),
            desc: description.map(|s| s.into()),
        });
        self.categories.len() - 1
    }

    /// Parse the specified arguments.
    pub fn parse(self, args: &[String]) -> Result<CommandLineIntent, String> {
        let parser = Parser::new(self, args);
        parser.parse()
    }

    /// Parse the program command line arguments.
    pub fn parse_env_args(self) {
        let args: Vec<String> = std::env::args().collect();
        match self.parse(&args) {
            Err(e) => {
                eprintln!("{}", e);
                exit(0);
            }
            Ok(CommandLineIntent::Help(s)) => {
                println!("{}", s);
                exit(0);
            }
            Ok(CommandLineIntent::Normal) => {}
        }
    }

    pub(crate) fn num_categories(&self) -> usize {
        self.categories.len()
    }

    /// Add a new option.
    pub(crate) fn add_option(&mut self, opt: Rc<dyn CLOption>) {
        self.opts.push(opt)
    }

    /// Find an option by name.
    pub(crate) fn find_option(
        &self,
        name: &str,
        long: bool,
    ) -> Option<(&Rc<dyn CLOption>, EqName)> {
        for opt in &self.opts {
            let eq = opt.eq_name(name, long);
            if let EqName::Yes | EqName::EnumValue(_) = &eq {
                return Some((opt, eq));
            }
        }
        None
    }

    /// Find the next positional option that can still accept values.
    pub(crate) fn next_positional(&self) -> Option<(&Rc<dyn CLOption>, EqName)> {
        self.opts
            .iter()
            .find(|p| p.is_accepting_positional())
            .map(|p| (p, EqName::Yes))
    }

    pub(crate) fn as_slice(&self) -> &[Rc<dyn CLOption>] {
        &self.opts
    }

    /// Check whether help has been requested. Also indicate whether the hidden
    /// options should be shown.
    pub(crate) fn is_help_requested(&self) -> Option<Hidden> {
        if let Some(h) = &self.help {
            if self.help_hidden.as_ref().unwrap().occurrences() != 0 {
                return Some(Hidden::Yes);
            }
            if h.occurrences() != 0 {
                return Some(Hidden::No);
            }
        }
        None
    }

    /// Build and return the help string.
    pub(crate) fn build_help(&self, prog_name: &str, show_hidden: bool) -> String {
        self.build_help_impl(prog_name, show_hidden).unwrap()
    }

    /// Build and return the help string.
    /// Note that this call can't fail, but the compiler keeps warning about
    /// unused Result in the `write!()` calls. So, we wrap this in another function
    /// to ignore it.
    fn build_help_impl(
        &self,
        prog_name: &str,
        show_hidden: bool,
    ) -> Result<String, std::fmt::Error> {
        // Collect information about the options.
        let opts = self.as_slice();
        let mut have_options = false;

        // An entry per category, containing all text for that category
        let mut cats: Vec<Vec<(String, Option<String>)>> = Vec::new();
        // Positional argument description.
        let mut pos_descs = String::new();

        cats.resize(self.categories.len(), Vec::new());

        fn add_value_desc(buf: &mut String, desc: Option<&str>) {
            buf.push('<');
            if let Some(vd) = desc {
                buf.push_str(vd);
            } else {
                buf.push_str("value");
            }
            buf.push('>');
        }

        for opt in opts {
            let info = opt.info();

            if info.is_positional() {
                if !pos_descs.is_empty() {
                    pos_descs.push(' ');
                }
                if let Some(d) = info.desc {
                    pos_descs.push_str(d);
                } else {
                    add_value_desc(&mut pos_descs, info.value_desc);
                    if info.list {
                        pos_descs.push(cond!(info.min_count == 0, '*', '+'));
                    } else if info.min_count == 0 {
                        pos_descs.push('?');
                    }
                }
                continue;
            }
            have_options = true;

            // Skip hidden options.
            if info.hidden == Hidden::ReallyHidden || info.hidden == Hidden::Yes && !show_hidden {
                continue;
            }

            let cat_index = info.category;

            // If there is more than one category, the first time we append to it,
            // we must generate its name and description.
            if self.categories.len() > 1 && cats[cat_index].is_empty() {
                // An empty line before every category except the first.
                if cat_index != 0 {
                    cats[cat_index].push(("".into(), None));
                }
                // Name.
                cats[cat_index].push((
                    format!("{}:", self.categories[cat_index].name.clone()),
                    None,
                ));
                // Optional description.
                if let Some(desc) = &self.categories[cat_index].desc {
                    cats[cat_index].push((desc.clone(), None));
                }
                // Another empty line.
                cats[cat_index].push(("".into(), None));
            }

            let mut left = String::from("  ");
            if let Some(sn) = info.short {
                left.push('-');
                left.push_str(sn);
                if info.long.is_none()
                    && info.expected_value != ExpectedValue::Disallowed
                    && !info.is_enum_value()
                {
                    left.push(' ');
                    if info.expected_value == ExpectedValue::Optional {
                        left.push('[');
                    }
                    add_value_desc(&mut left, info.value_desc);
                    if info.expected_value == ExpectedValue::Optional {
                        left.push(']');
                    }
                }
            }
            if let Some(ln) = info.long {
                if info.short.is_some() {
                    left.push_str(", ");
                }
                left.push_str("--");
                left.push_str(ln);
                if info.expected_value != ExpectedValue::Disallowed && !info.is_enum_value() {
                    if info.expected_value == ExpectedValue::Optional {
                        left.push('[');
                    }
                    left.push('=');
                    add_value_desc(&mut left, info.value_desc);
                    if info.expected_value == ExpectedValue::Optional {
                        left.push(']');
                    }
                }
            }
            if info.is_enum_value() {
                // An enum value generates descriptions for all alternatives.
                cats[cat_index].push((left, Some(info.desc.unwrap_or("").to_string())));
                for alt in info.values_desc.unwrap() {
                    cats[cat_index].push((format!("    ={}", alt.0), Some(format!("  {}", alt.1))));
                }
            } else if info.is_enum_option() {
                // An enum option generates descriptions for all alternatives.
                left.push_str(if let Some(d) = info.desc {
                    d
                } else {
                    "Choose one of:"
                });
                cats[cat_index].push((left, None));
                for alt in info.values_desc.unwrap() {
                    cats[cat_index].push((
                        format!("    {}{}", cond!(alt.0.len() > 1, "--", "-"), alt.0),
                        Some(alt.1.clone()),
                    ));
                }
            } else {
                cats[cat_index].push((left, Some(info.desc.unwrap_or("").to_string())));
            }
        }

        let descs: Vec<(String, Option<String>)> = cats.iter().flatten().cloned().collect();
        drop(cats);

        // Calc the width of the left column.
        let left_width = descs.iter().fold(0, |left_w, p| {
            cond!(p.1.is_none(), left_w, left_w.max(p.0.len()))
        });

        let mut out = String::new();
        writeln!(out, "OVERVIEW: {}\n", self.desc)?;
        write!(out, "USAGE: {}", prog_name)?;
        if have_options {
            write!(out, " [options]")?;
        }
        if !pos_descs.is_empty() {
            write!(out, " {}", pos_descs)?;
        }
        writeln!(out, "\n")?;
        writeln!(out, "OPTIONS:\n")?;

        for (left, right) in &descs {
            match right {
                Some(r) if !r.is_empty() => writeln!(out, "{0:1$} - {2}", left, left_width, r)?,
                _ => writeln!(out, "{}", left)?,
            }
        }

        Ok(out)
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[derive(Clone, Copy, PartialEq, Eq, Debug)]
    enum TestEnum {
        GenJS,
        GenAST,
        GenIR,
    }

    #[test]
    fn test_help() {
        let mut options = CommandLine::new("juno");

        Opt::<u32>::new(
            &mut options,
            OptDesc {
                long: Some("memory"),
                short: Some("m"),
                desc: Some("Set maximum memory amount"),
                value_desc: Some("number"),
                ..Default::default()
            },
        );
        Opt::<TestEnum>::new_enum(
            &mut options,
            OptDesc {
                values: Some(&[
                    ("gen-js", TestEnum::GenJS, "Generate JS"),
                    ("gen-ast", TestEnum::GenAST, "Generate AST"),
                    ("gen-ir", TestEnum::GenIR, "Generate IR"),
                ]),
                desc: Some("Choose output"),
                ..Default::default()
            },
        );
        Opt::<u32>::new(
            &mut options,
            OptDesc {
                short: Some("i"),
                desc: Some("Increment value."),
                ..Default::default()
            },
        );
        Opt::<TestEnum>::new_enum(
            &mut options,
            OptDesc {
                long: Some("gen"),
                values: Some(&[
                    ("js", TestEnum::GenJS, "Generate JS"),
                    ("ast", TestEnum::GenAST, "Generate AST"),
                    ("ir", TestEnum::GenIR, "Generate IR"),
                ]),
                desc: Some("Generate output of the following kind"),
                list: true,
                ..Default::default()
            },
        );
        Opt::<String>::new(
            &mut options,
            OptDesc {
                max_count: 2,
                list: true,
                value_desc: Some("file"),
                ..Default::default()
            },
        );
        Opt::<String>::new(
            &mut options,
            OptDesc {
                value_desc: Some("port"),
                ..Default::default()
            },
        );
        Opt::<String>::new(
            &mut options,
            OptDesc {
                max_count: usize::MAX,
                list: true,
                desc: Some("<extra arguments>"),
                ..Default::default()
            },
        );

        static OUTPUT: &str = r"OVERVIEW: juno

USAGE: test [options] <file>* <port>? <extra arguments>

OPTIONS:

  -h, --help            - Display available options (-help-hidden for more)
  -m, --memory=<number> - Set maximum memory amount
  Choose output
    --gen-js            - Generate JS
    --gen-ast           - Generate AST
    --gen-ir            - Generate IR
  -i <value>            - Increment value.
  --gen                 - Generate output of the following kind
    =js                 -   Generate JS
    =ast                -   Generate AST
    =ir                 -   Generate IR
";
        assert_eq!(options.build_help("test", false), OUTPUT);
    }
    #[test]
    fn test_categories_help() {
        let mut options = CommandLine::new("juno");

        let output_cat = options.add_category("Output options", Some("These control the output."));
        let special_cat = options.add_category("Special options", None);

        Opt::<u32>::new(
            &mut options,
            OptDesc {
                long: Some("memory"),
                short: Some("m"),
                desc: Some("Set maximum memory amount"),
                value_desc: Some("number"),
                category: special_cat,
                ..Default::default()
            },
        );
        Opt::<u32>::new(
            &mut options,
            OptDesc {
                short: Some("i"),
                desc: Some("Increment value."),
                category: output_cat,
                ..Default::default()
            },
        );
        Opt::<TestEnum>::new_enum(
            &mut options,
            OptDesc {
                values: Some(&[
                    ("gen-js", TestEnum::GenJS, "Generate JS"),
                    ("gen-ast", TestEnum::GenAST, "Generate AST"),
                    ("gen-ir", TestEnum::GenIR, "Generate IR"),
                ]),
                desc: Some("Choose output"),
                category: output_cat,
                ..Default::default()
            },
        );
        Opt::<TestEnum>::new_enum(
            &mut options,
            OptDesc {
                long: Some("gen"),
                values: Some(&[
                    ("js", TestEnum::GenJS, "Generate JS"),
                    ("ast", TestEnum::GenAST, "Generate AST"),
                    ("ir", TestEnum::GenIR, "Generate IR"),
                ]),
                desc: Some("Generate output of the following kind"),
                list: true,
                ..Default::default()
            },
        );
        Opt::<String>::new(
            &mut options,
            OptDesc {
                list: true,
                value_desc: Some("file"),
                ..Default::default()
            },
        );
        Opt::<String>::new(
            &mut options,
            OptDesc {
                long: Some("port"),
                desc: Some("The port"),
                value_desc: Some("port number"),
                category: special_cat,
                ..Default::default()
            },
        );

        static OUTPUT: &str = r"OVERVIEW: juno

USAGE: test [options] <file>*

OPTIONS:

Generic Options:

  -h, --help            - Display available options (-help-hidden for more)
  --gen                 - Generate output of the following kind
    =js                 -   Generate JS
    =ast                -   Generate AST
    =ir                 -   Generate IR

Output options:
These control the output.

  -i <value>            - Increment value.
  Choose output
    --gen-js            - Generate JS
    --gen-ast           - Generate AST
    --gen-ir            - Generate IR

Special options:

  -m, --memory=<number> - Set maximum memory amount
  --port=<port number>  - The port
";
        assert_eq!(options.build_help("test", false), OUTPUT);
    }
}
