/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

use super::opt::*;
use crate::cl::CommandLine;
use std::path::Path;

#[derive(Debug, Clone, Eq, PartialEq)]
pub enum CommandLineIntent {
    /// Successful completion.
    Normal,
    /// Help was requested. The help text is attached.
    Help(String),
}

pub(crate) struct Parser<'a> {
    prog_name: String,
    opts: CommandLine,
    args: &'a [String],

    cur_arg: usize,
}

impl Parser<'_> {
    pub fn new(opts: CommandLine, args: &[String]) -> Parser {
        // Extract the name from the first arg.
        let name = args
            .first()
            .map(|s| AsRef::<Path>::as_ref(s.as_str()))
            .and_then(|p| p.file_name())
            .map(|os| os.to_string_lossy().to_string())
            .unwrap_or_else(|| String::from("<unknown>"));

        Parser {
            prog_name: name,
            opts,
            args,
            cur_arg: 0,
        }
    }

    pub(crate) fn parse(mut self) -> Result<CommandLineIntent, String> {
        self.cur_arg = 1;
        while self.cur_arg < self.args.len() {
            let arg = self.args[self.cur_arg].as_str();
            if arg == "--" {
                // End processing options at "--".
                self.cur_arg += 1;
                break;
            } else if arg.starts_with("--") {
                self.parse_long_arg(arg.split_at(2).1)?;
            } else if arg.starts_with('-') && arg.len() > 1 {
                self.parse_short_arg(arg.split_at(1).1)?;
            } else {
                self.parse_positional_arg(arg)?;
            }

            match self.opts.is_help_requested() {
                Some(Hidden::No) => {
                    return Ok(CommandLineIntent::Help(
                        self.opts.build_help(&self.prog_name, false),
                    ));
                }
                Some(_) => {
                    return Ok(CommandLineIntent::Help(
                        self.opts.build_help(&self.prog_name, true),
                    ));
                }
                None => {}
            }

            self.cur_arg += 1;
        }

        // Consume the remaining positional args.
        while self.cur_arg < self.args.len() {
            self.parse_positional_arg(&self.args[self.cur_arg])?;
            self.cur_arg += 1;
        }

        // Validate all options in the end.
        for opt in self.opts.as_slice() {
            if let Err(e) = opt.finish() {
                let info = opt.info();
                return if info.is_positional() {
                    if let Some(vd) = info.desc.or(info.value_desc) {
                        Err(format!(
                            "{}: for positional argument {}: {}",
                            self.prog_name, vd, e
                        ))
                    } else {
                        Err(format!(
                            "{}: for positional argument: {}",
                            self.prog_name, e
                        ))
                    }
                } else {
                    Err(format!(
                        "{}: for the {} option: {}",
                        self.prog_name,
                        opt.name(),
                        e
                    ))
                };
            }
        }

        Ok(CommandLineIntent::Normal)
    }

    fn parse_positional_arg(&mut self, arg: &str) -> Result<(), String> {
        let opt = if let Some(opt) = self.opts.next_positional() {
            opt
        } else {
            return Err(format!(
                "{}: too many positional arguments at argument '{}'",
                self.prog_name, arg
            ));
        };

        match opt.0.parse_value(Some(arg), opt.1) {
            Ok(_) => Ok(()),
            Err(e) => {
                let info = opt.0.info();
                if let Some(vd) = info.desc.or(info.value_desc) {
                    Err(format!(
                        "{}: for positional argument {}: {}",
                        self.prog_name, vd, e
                    ))
                } else {
                    Err(format!(
                        "{}: for argument nr {}: {}",
                        self.prog_name, self.cur_arg, e
                    ))
                }
            }
        }
    }

    fn parse_long_arg(&mut self, arg: &str) -> Result<(), String> {
        let (name, value) = if let Some((name, value)) = arg.split_once('=') {
            (name, Some(value))
        } else {
            (arg, None)
        };

        let opt = if let Some(opt) = self.opts.find_option(name, true) {
            opt
        } else {
            return Err(format!(
                "{}: Unknown command line argument --{}",
                self.prog_name, name
            ));
        };

        match opt.0.parse_value(value, opt.1) {
            Ok(_) => Ok(()),
            Err(e) => Err(format!(
                "{}: for the --{} option: {}",
                self.prog_name, name, e
            )),
        }
    }

    fn parse_short_arg(&mut self, mut arg: &str) -> Result<(), String> {
        debug_assert!(!arg.is_empty());

        /// Check if an argument is non-option. Options start with "--" or
        /// with "-" following with at least one more char.
        fn is_non_option(arg: &str) -> bool {
            !(arg.starts_with('-') && arg.len() > 1)
        }

        while !arg.is_empty() {
            let (name, mut rest) = arg.split_at(1);

            let opt = if let Some(opt) = self.opts.find_option(name, false) {
                opt
            } else {
                return Err(format!(
                    "{}: Unknown command line argument -{}",
                    self.prog_name, name
                ));
            };

            // Extract the value. If a value is possible, it either follows in
            // the current arg "-lFoo", or follows in the next one "-l Foo".
            let info = opt.0.info();
            let mut value = None;
            if info.expected_value != ExpectedValue::Disallowed {
                if !rest.is_empty() {
                    value = Some(rest);
                    rest = "";
                } else if self.cur_arg < self.args.len() - 1
                    && is_non_option(&self.args[self.cur_arg + 1])
                {
                    // "Steal" the next argument.
                    self.cur_arg += 1;
                    value = Some(&self.args[self.cur_arg]);
                }
            }

            if let Err(e) = opt.0.parse_value(value, opt.1) {
                return Err(format!(
                    "{}: for the -{} option: {}",
                    self.prog_name, name, e
                ));
            }

            arg = rest;
        }

        Ok(())
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_long() {
        let mut options = CommandLine::new("juno");

        let max_heap = Opt::<u32>::new(
            &mut options,
            OptDesc {
                long: Some("max-heap"),
                short: Some("m"),
                ..Default::default()
            },
        );

        let vec1: Vec<String> = "test --max-heap=10"
            .split_whitespace()
            .map(|s| s.to_string())
            .collect();
        let p = Parser::new(options, &vec1);
        p.parse().expect("parse should succeed");

        assert_eq!(*max_heap, 10);
    }

    #[test]
    fn test_short() {
        let mut options = CommandLine::new("juno");

        let m = Opt::<u32>::new(
            &mut options,
            OptDesc {
                short: Some("m"),
                ..Default::default()
            },
        );
        let i = Opt::<u32>::new(
            &mut options,
            OptDesc {
                short: Some("i"),
                ..Default::default()
            },
        );
        let o = Opt::<u32>::new(
            &mut options,
            OptDesc {
                short: Some("o"),
                def_value: Some(5),
                ..Default::default()
            },
        );

        let vec1: Vec<String> = "test -o -m 10 -i32"
            .split_whitespace()
            .map(|s| s.to_string())
            .collect();
        let p = Parser::new(options, &vec1);
        p.parse().expect("parse should succeed");

        assert_eq!(*m, 10);
        assert_eq!(*i, 32);
        assert_eq!(*o, 5);
    }

    #[test]
    fn test_positional() {
        let mut options = CommandLine::new("juno");

        let m = Opt::<u32>::new(
            &mut options,
            OptDesc {
                short: Some("m"),
                ..Default::default()
            },
        );
        let i = Opt::<u32>::new(
            &mut options,
            OptDesc {
                short: Some("i"),
                ..Default::default()
            },
        );
        let pos1 = Opt::<String>::new(
            &mut options,
            OptDesc {
                max_count: 2,
                list: true,
                desc: Some("pos1"),
                ..Default::default()
            },
        );
        let pos2 = Opt::<String>::new(
            &mut options,
            OptDesc {
                desc: Some("pos2"),
                ..Default::default()
            },
        );
        let extra = Opt::<String>::new(
            &mut options,
            OptDesc {
                max_count: usize::MAX,
                list: true,
                desc: Some("extra"),
                ..Default::default()
            },
        );

        let vec1: Vec<String> = "test file1 -m 10 file2 -i32 file3 extra1 -- --extra2"
            .split_whitespace()
            .map(|s| s.to_string())
            .collect();
        let p = Parser::new(options, &vec1);
        p.parse().expect("parse should succeed");

        assert_eq!(*m, 10);
        assert_eq!(*i, 32);

        assert_eq!(pos1.num_values(), 2);
        assert_eq!(pos1[0], "file1");
        assert_eq!(pos1[1], "file2");
        assert_eq!(pos2.num_values(), 1);
        assert_eq!(pos2[0], "file3");
        assert_eq!(extra.num_values(), 2);
        assert_eq!(extra[0], "extra1");
        assert_eq!(extra[1], "--extra2");
    }

    #[derive(Clone, Copy, PartialEq, Eq, Debug)]
    enum TestEnum {
        GenJS,
        GenAST,
        GenIR,
    }

    #[test]
    fn test_enum_option() {
        let mut options = CommandLine::new("juno");

        let max_heap = Opt::<u32>::new(
            &mut options,
            OptDesc {
                long: Some("max-heap"),
                short: Some("m"),
                ..Default::default()
            },
        );

        let gen = Opt::<TestEnum>::new_enum(
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

        let vec1: Vec<String> = "test --max-heap=10 --gen-ast"
            .split_whitespace()
            .map(|s| s.to_string())
            .collect();
        let p = Parser::new(options, &vec1);
        p.parse().expect("parse should succeed");

        assert_eq!(*max_heap, 10);
        assert_eq!(gen.num_values(), 1);
        assert_eq!(*gen, TestEnum::GenAST);
    }

    #[test]
    fn test_enum_option_err() {
        let mut options = CommandLine::new("juno");

        let _gen = Opt::<TestEnum>::new_enum(
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

        let vec1: Vec<String> = "test --gen-ast --gen-js"
            .split_whitespace()
            .map(|s| s.to_string())
            .collect();
        let p = Parser::new(options, &vec1);
        assert_eq!(
            p.parse().expect_err("parse should fail"),
            "test: for the --gen-js option: option may not occur more than 1 time"
        );
    }

    #[test]
    fn test_enum_value() {
        let mut options = CommandLine::new("juno");

        let gen = Opt::<TestEnum>::new_enum(
            &mut options,
            OptDesc {
                long: Some("gen"),
                values: Some(&[
                    ("js", TestEnum::GenJS, "Generate JS"),
                    ("ast", TestEnum::GenAST, "Generate AST"),
                    ("ir", TestEnum::GenIR, "Generate IR"),
                ]),
                desc: Some("Choose output"),
                list: true,
                ..Default::default()
            },
        );

        let vec1: Vec<String> = "test --gen=ast --gen=js"
            .split_whitespace()
            .map(|s| s.to_string())
            .collect();
        let p = Parser::new(options, &vec1);
        p.parse().expect("parse should succeed");

        assert_eq!(gen.num_values(), 2);
        assert_eq!(gen[0], TestEnum::GenAST);
        assert_eq!(gen[1], TestEnum::GenJS);
    }
}
