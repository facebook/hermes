/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

use crate::cl::CommandLine;
use std::cell::Cell;
use std::cell::UnsafeCell;
use std::collections::HashSet;
use std::ops::Deref;
use std::rc::Rc;
use std::str::FromStr;

/// This enum controls whether an option can have a value.
#[derive(Debug, Clone, Copy, Eq, PartialEq)]
pub enum ExpectedValue {
    Optional,
    Required,
    Disallowed,
}

/// This enum controls whether an option is shown in the help text.
#[derive(Debug, Clone, Copy, Eq, PartialEq)]
pub enum Hidden {
    /// The option is always shown.
    No,
    /// The option is shown only with --help-hidden.
    Yes,
    /// The option is never shown.
    ReallyHidden,
}

/// Stores the value of a single option.
#[derive(Debug)]
pub struct OptValue<T> {
    value: UnsafeCell<Vec<T>>,
    /// This flag controls whether the parser is done with [`value`], meaning
    /// that it can no longer change, so we can give references to it.
    finished_parsing: Cell<bool>,
}

impl<T> Default for OptValue<T> {
    fn default() -> Self {
        // Self{value: RefCell::new(Vec::new())}
        Self {
            value: UnsafeCell::new(Default::default()),
            finished_parsing: Cell::new(false),
        }
    }
}

impl<T> OptValue<T> {
    /// Set the only value in the vector, or push a new value into the vector
    /// depending on `push`.
    /// This method may only be called before parsing has finished.
    fn update_value(&self, push: bool, value: T) {
        assert!(
            !self.finished_parsing.get(),
            "value cannot be modified after parsing"
        );
        let v = unsafe { &mut *self.value.get() };
        if push || v.is_empty() {
            v.push(value);
        } else {
            // We know this is in range, since we just checked that it isn't empty.
            unsafe {
                *v.get_unchecked_mut(0) = value;
            }
        }
    }

    /// Mark the value is no longer changeable, which enables borrowing.
    fn finish(&self) {
        self.finished_parsing.set(true);
    }

    fn borrow_vec(&self) -> &Vec<T> {
        assert!(
            self.finished_parsing.get(),
            "value cannot be borrowed during parsing"
        );
        unsafe { &*self.value.get() }
    }
    fn borrow_value(&self) -> &T {
        self.borrow_vec()
            .first()
            .expect("value must be set when borrowing non-list")
    }
}

/// Descriptor of a single enum value: name, value, description.
pub type EnumDesc<T, S> = (S, T, S);

/// A descriptor of a single command option.
#[derive(Debug, Clone)]
pub struct OptDesc<'a, T, S: Into<String>> {
    /// Long option name, e.g. "help". If both long and short names are omitted,
    /// the option is positional.
    pub long: Option<S>,
    /// Short option name, e.g. "h". If both long and short names are omitted,
    /// the option is positional.
    pub short: Option<S>,
    /// If specified, and [`long`] and [`short`] are missing, it is a list of
    /// mutually exclusive options.
    /// If specified, but [`long`] or [`short`] are present, it is a list of
    /// possible values.
    pub values: Option<&'a [EnumDesc<T, S>]>,
    /// Initialize the option with this value,
    pub init: Option<T>,
    /// A default value to use when the value is omitted.
    pub def_value: Option<T>,
    /// Whether a value is expected, allowed, etc.
    pub expected_value: Option<ExpectedValue>,
    /// Minimum number of times this option must be specified. Default 0.
    pub min_count: usize,
    /// Maximum number of times this option can be specified. For unlimted use
    /// [`usize::MAX`]. Default to 0, indicating that the value will be chosen
    /// automatically based on other parameters,
    pub max_count: usize,
    /// Whether this value will produce a Vec of values or just a single value.
    pub list: bool,
    /// Description of the option,
    pub desc: Option<S>,
    /// Description of the value.
    pub value_desc: Option<S>,
    /// Whether the option is hidden.
    pub hidden: Hidden,
    /// If this field is set, it allows several options to share a value.
    pub opt_value: Option<Rc<OptValue<T>>>,
    /// The category this option belongs to. The default 0 is "Generic Options".
    pub category: usize,
}

impl<T, S: Into<String>> Default for OptDesc<'_, T, S> {
    fn default() -> Self {
        Self {
            long: None,
            short: None,
            values: None,
            init: None,
            def_value: None,
            expected_value: None,
            min_count: 0,
            max_count: 0,
            list: false,
            desc: None,
            value_desc: None,
            hidden: Hidden::No,
            opt_value: None,
            category: 0,
        }
    }
}

impl<T, S: Into<String>> OptDesc<'_, T, S> {
    fn is_enum_option(&self) -> bool {
        self.long.is_none() && self.short.is_none() && self.values.is_some()
    }
}

#[derive(Clone)]
pub struct Opt<T>(pub Rc<OptHolder<T>>);

impl<T> Deref for Opt<T> {
    type Target = T;
    fn deref(&self) -> &Self::Target {
        Rc::deref(&self.0).opt_value.borrow_value()
    }
}

impl<T> std::ops::Index<usize> for Opt<T> {
    type Output = T;

    fn index(&self, index: usize) -> &Self::Output {
        Rc::deref(&self.0).opt_value.borrow_vec().index(index)
    }
}

impl<T> Opt<T> {
    /// Return a vec of all values.
    pub fn values(&self) -> &Vec<T> {
        self.0.opt_value.borrow_vec()
    }
    /// Return the number of values that have been stored. Note that this number
    /// is not necessarily the same as [`Self::occurrences()`].
    pub fn num_values(&self) -> usize {
        self.values().len()
    }
    /// Return how many times this option was specified in the command line.
    pub fn occurrences(&self) -> usize {
        self.0.count.get()
    }
}

pub struct OptHolder<T> {
    /// Long option name, e.g. "help". If both long and short names are omitted,
    /// the option is positional.
    long: Option<String>,
    /// Short option name, e.g. "h". If both long and short names are omitted,
    /// the option is positional.
    short: Option<String>,
    /// If specified, and [`long`] and [`short`] are missing, it is a list of
    /// mutually exclusive options.
    /// If specified, but [`long`] or [`short`] are present, it is a list of
    /// possible values.
    values: Option<Box<[T]>>,
    /// The names and descriptions of the [`values`].
    values_desc: Option<Box<[(String, String)]>>,
    /// A default value to use when the value is omitted.
    def_value: Option<T>,
    /// Whether a value is expected, allowed, etc.
    expected_value: ExpectedValue,
    /// Minimum number of times this option must be specified. Default 0.
    min_count: usize,
    /// Maximum number of times this option can be specified. Default 1.
    /// Use usize::MAX for unlimited.
    max_count: usize,
    /// Whether this value will produce a Vec of values or just a single value.
    list: bool,
    /// Description of the option,
    desc: Option<String>,
    /// Description of the value.
    value_desc: Option<String>,
    /// Whether the option is hidden.
    hidden: Hidden,
    /// The value. It can be shared between multiple options.
    opt_value: Rc<OptValue<T>>,
    /// The category this option belongs to. The default 0 is "Generic Options".
    pub category: usize,

    /// Callback to parse a string into T.
    parser: Box<dyn Fn(&str) -> Result<T, String>>,
    /// How many times has the option been specified.
    count: Cell<usize>,
}

pub(crate) struct OptInfo<'a> {
    pub long: Option<&'a str>,
    pub short: Option<&'a str>,
    pub values_desc: Option<&'a [(String, String)]>,
    pub expected_value: ExpectedValue,
    pub min_count: usize,
    #[allow(dead_code)]
    pub max_count: usize,
    pub list: bool,
    pub desc: Option<&'a str>,
    pub value_desc: Option<&'a str>,
    pub hidden: Hidden,
    pub category: usize,
}

impl OptInfo<'_> {
    /// Is this a positional argument?
    pub fn is_positional(&self) -> bool {
        self.long.is_none() && self.short.is_none() && self.values_desc.is_none()
    }
    /// An enum option is a set of mutually exclusive options encoded as an
    /// enum.
    pub fn is_enum_option(&self) -> bool {
        self.long.is_none() && self.short.is_none() && self.values_desc.is_some()
    }
    /// Is this an option whose value is an enum?
    pub fn is_enum_value(&self) -> bool {
        self.values_desc.is_some() && (self.long.is_some() || self.short.is_some())
    }
}

#[derive(Debug, Copy, Clone)]
pub(crate) enum EqName {
    /// The name didn't match.
    No,
    /// The name matched.
    Yes,
    /// The name matched an enum value with the following index.
    EnumValue(usize),
}

pub(crate) trait CLOption {
    /// Return true if this is a positional argument that can still accept
    /// values.
    fn is_accepting_positional(&self) -> bool;
    /// Parse a value, and if successful, store it.
    fn parse_value(&self, s: Option<&str>, eq: EqName) -> Result<(), String>;
    /// At the end of CLI parsing, perform finish actions like setting a init
    /// value and validate the option by checking [`Opt::min_count`].
    fn finish(&self) -> Result<(), String>;
    /// Compare the long or the short name for equality with the specified string.
    /// If this is an enum option, compare against all names.
    fn eq_name(&self, name: &str, long: bool) -> EqName;
    /// Return which ever name exist prefixed with '-' or '--'.
    fn name(&self) -> String;
    fn info(&self) -> OptInfo;
}

impl<T: Clone> OptHolder<T> {
    /// Is this a positional argument?
    fn is_positional(&self) -> bool {
        self.long.is_none() && self.short.is_none() && self.values.is_none()
    }

    /// An enum option is a set of mutually exclusive options encoded as an
    /// enum.
    fn is_enum_option(&self) -> bool {
        self.long.is_none() && self.short.is_none() && self.values.is_some()
    }

    /// Is this an option whose value is an enum?
    fn is_enum_value(&self) -> bool {
        self.values.is_some() && (self.long.is_some() || self.short.is_some())
    }

    /// Parse the input as an enum value described by [`Self::values_desc`].
    fn parse_enum_value(&self, input: &str) -> Result<T, String> {
        if let Some(vd) = &self.values_desc {
            if let Some(index) = vd.iter().position(|p| p.0.as_str() == input) {
                return Ok(self.values.as_ref().unwrap()[index].clone());
            }
        }
        Err(format!("cannot find option named '{}'", input))
    }

    //noinspection RsSelfConvention
    fn set_opt_value(&self, v: Option<T>) {
        if let Some(value) = v {
            self.opt_value.update_value(self.list, value);
        }
        self.count.set(self.count.get() + 1);
    }
}

impl<T: 'static + Clone> CLOption for OptHolder<T> {
    fn is_accepting_positional(&self) -> bool {
        self.is_positional() && self.count.get() < self.max_count
    }

    fn parse_value(&self, s: Option<&str>, eq: EqName) -> Result<(), String> {
        if self.count.get() >= self.max_count {
            return Err(format!(
                "option may not occur more than {} time{}",
                self.max_count,
                plural(self.max_count)
            ));
        }

        let enum_opt_value = if let EqName::EnumValue(index) = eq {
            Some(self.values.as_ref().unwrap()[index].clone())
        } else {
            None
        };

        match (
            self.expected_value,
            enum_opt_value.as_ref().or(self.def_value.as_ref()),
            s,
        ) {
            (ExpectedValue::Disallowed, _, Some(s)) => {
                Err(format!("option does not allow a value. '{}' specified", s))
            }

            (ExpectedValue::Disallowed, d, None) | (ExpectedValue::Optional, d, None) => {
                self.set_opt_value(d.cloned());
                Ok(())
            }

            (ExpectedValue::Required, _, None) => Err(String::from("option requires a value")),

            (ExpectedValue::Optional, _, Some(v)) | (ExpectedValue::Required, _, Some(v)) => {
                match cond!(
                    self.is_enum_value(),
                    self.parse_enum_value(v),
                    (self.parser)(v)
                ) {
                    Ok(o) => {
                        self.set_opt_value(Some(o));
                        Ok(())
                    }
                    Err(e) => Err(e),
                }
            }
        }
    }

    fn finish(&self) -> Result<(), String> {
        self.opt_value.finish();
        if self.count.get() < self.min_count {
            if self.min_count == self.max_count {
                if self.min_count == 1 {
                    Err(String::from("a value must be supplied"))
                } else {
                    // A weird requirement, but all right.
                    Err(format!(
                        "option must be specified exactly {} time{}",
                        self.min_count,
                        plural(self.min_count)
                    ))
                }
            } else {
                Err(format!(
                    "option must be specified at least {} time{}",
                    self.min_count,
                    plural(self.min_count)
                ))
            }
        } else {
            Ok(())
        }
    }

    fn eq_name(&self, name: &str, long: bool) -> EqName {
        if self.is_enum_option() {
            self.values_desc
                .as_ref()
                .unwrap()
                .iter()
                .position(|p| p.0 == name)
                .map(EqName::EnumValue)
                .unwrap_or(EqName::No)
        } else {
            cond!(
                cond!(long, self.long.as_ref(), self.short.as_ref()).map(|s| s.as_str())
                    == Some(name),
                EqName::Yes,
                EqName::No
            )
        }
    }

    fn name(&self) -> String {
        if let Some(ln) = &self.long {
            format!("--{}", ln)
        } else if let Some(sn) = &self.short {
            format!("-{}", sn)
        } else {
            String::from("<argument>")
        }
    }

    fn info(&self) -> OptInfo {
        OptInfo {
            long: self.long.as_deref(),
            short: self.short.as_deref(),
            values_desc: self.values_desc.as_deref(),
            expected_value: self.expected_value,
            min_count: self.min_count,
            max_count: self.max_count,
            list: self.list,
            desc: self.desc.as_deref(),
            value_desc: self.value_desc.as_deref(),
            hidden: self.hidden,
            category: self.category,
        }
    }
}

impl<T: 'static + Clone> Opt<T> {
    pub fn with_parser<S: Into<String> + Clone>(
        opts: &mut CommandLine,
        desc: OptDesc<T, S>,
        parser: impl Fn(&str) -> Result<T, String> + 'static,
    ) -> Opt<T> {
        Self::with_parser_impl(opts, desc, Box::new(parser))
    }

    /// An implementation of [`Opt::with_parser()`] avoiding the parser generic.
    fn with_parser_impl<S: Into<String> + Clone>(
        opts: &mut CommandLine,
        mut desc: OptDesc<T, S>,
        parser: Box<dyn Fn(&str) -> Result<T, String>>,
    ) -> Opt<T> {
        // Need to record this before fields start moving out.
        let is_enum_option = desc.is_enum_option();

        // Convert names to string.
        let long = desc.long.map(|s| s.into());
        let short = desc.short.map(|s| s.into());
        // Check for duplicate option names.
        if let Some(ln) = &long {
            assert!(ln.len() > 1, "Long options must be at least two characters");
            debug_assert!(
                opts.find_option(ln.as_str(), true).is_none(),
                "Duplicated option '--{}'",
                ln
            );
        }
        if let Some(sn) = &short {
            assert_eq!(sn.len(), 1, "Short options must be a single character");
            debug_assert!(
                opts.find_option(sn.as_str(), false).is_none(),
                "Duplicated option '-{}'",
                sn
            );
        }

        // Handle enums.
        let (values, values_desc) = if let Some(enum_desc) = desc.values {
            let (v, d) = Self::copy_enum_desc(enum_desc);
            // Is this an enum option?
            if is_enum_option {
                // Values are disallowed in enum options, because the enum value is the value.
                desc.expected_value = desc.expected_value.or(Some(ExpectedValue::Disallowed));
                assert_eq!(
                    desc.expected_value,
                    Some(ExpectedValue::Disallowed),
                    "Enum options can't have a value"
                );
                assert!(
                    desc.def_value.is_none(),
                    "Enum option cannot have a default value"
                );

                // Check for duplicate option names.
                if cfg!(debug_assertions) {
                    for nd in d.iter() {
                        let name = nd.0.as_str();
                        debug_assert!(
                            opts.find_option(name, name.len() > 1).is_none(),
                            "Duplicated option '{}{}'",
                            cond!(name.len() > 1, "--", "-"),
                            name
                        );
                    }
                }
            } else {
                // The value cannot be disallowed in options of the form "--opt=enum_val".
                assert_ne!(
                    desc.expected_value,
                    Some(ExpectedValue::Disallowed),
                    "Values cannot be disallowed in enum values"
                );
            }
            // If these are unique option names, we must validate them.
            (Some(v), Some(d))
        } else {
            (None, None)
        };

        // If expected_value is not set, determine it based on the presence of a
        // default value.
        desc.expected_value = desc.expected_value.or(cond!(
            desc.def_value.is_some(),
            Some(ExpectedValue::Optional),
            Some(ExpectedValue::Required)
        ));

        // If needed, automatically determined max count.
        if desc.max_count == 0 {
            if is_enum_option {
                desc.max_count = 1;
            } else if desc.list || desc.expected_value.unwrap() == ExpectedValue::Disallowed {
                desc.max_count = usize::MAX;
            } else {
                desc.max_count = 1;
            }
        }

        // Validate counts.
        assert!(
            desc.min_count <= desc.max_count,
            "min_count cannot exceed max_count"
        );
        // Validate category.
        assert!(desc.category < opts.num_categories());

        let opt = Self(Rc::<OptHolder<T>>::new(OptHolder {
            long,
            short,
            values,
            values_desc,
            def_value: desc.def_value.clone(),
            expected_value: desc.expected_value.unwrap(),
            min_count: desc.min_count,
            max_count: desc.max_count,
            list: desc.list,
            desc: desc.desc.map(|s| s.into()),
            value_desc: desc.value_desc.map(|s| s.into()),
            hidden: desc.hidden,
            opt_value: desc
                .opt_value
                .unwrap_or_else(|| Rc::new(Default::default())),
            category: desc.category,
            parser,
            count: Default::default(),
        }));

        if !desc.list {
            opt.0
                .opt_value
                .update_value(false, desc.init.expect("init value must be specified"));
        }

        opts.add_option(opt.0.clone());
        opt
    }

    /// Convert the list of enum descriptors into the internal form: two boxed
    /// slices: one of values, the other of names and descriptions.
    /// Validates that the names are not empty and in debug mode validates that
    /// there are no duplicates.
    #[allow(clippy::type_complexity)]
    fn copy_enum_desc<S: Into<String> + Clone>(
        enum_desc: &[EnumDesc<T, S>],
    ) -> (Box<[T]>, Box<[(String, String)]>) {
        assert!(
            !enum_desc.is_empty(),
            "There must be at least one enum value"
        );

        let mut values = Vec::with_capacity(enum_desc.len());
        let mut values_desc = Vec::<(String, String)>::with_capacity(enum_desc.len());
        for ed in enum_desc {
            values.push(ed.1.clone());
            values_desc.push((ed.0.clone().into(), ed.2.clone().into()));
            assert!(
                !values_desc.last().as_ref().unwrap().0.is_empty(),
                "Empty enum name"
            );
        }

        // Check for duplicates.
        if cfg!(debug_assertions) {
            let mut name_set = HashSet::new();
            values_desc.iter().for_each(|p| {
                debug_assert!(
                    name_set.insert(p.0.as_str()),
                    "Duplicated enum value {}",
                    p.0
                )
            });
        }

        (values.into_boxed_slice(), values_desc.into_boxed_slice())
    }

    /// Create a new option with type T, where T: Default.
    pub fn new<S: Into<String> + Clone>(opts: &mut CommandLine, mut desc: OptDesc<T, S>) -> Opt<T>
    where
        T: Default + FromStr,
        <T as FromStr>::Err: std::fmt::Display,
    {
        // Set the init value if not specified.
        desc.init = desc.init.or_else(|| Some(T::default()));
        Self::with_parser(opts, desc, |s| match T::from_str(s) {
            Ok(o) => Ok(o),
            Err(e) => Err(format!("{}", e)),
        })
    }

    /// Create a new option containing a list of T values.
    pub fn new_list<S: Into<String> + Clone>(
        opts: &mut CommandLine,
        mut desc: OptDesc<T, S>,
    ) -> Opt<T>
    where
        T: FromStr,
        <T as FromStr>::Err: std::fmt::Display,
    {
        desc.list = true;
        Self::with_parser(opts, desc, |s| match T::from_str(s) {
            Ok(o) => Ok(o),
            Err(e) => Err(format!("{}", e)),
        })
    }

    /// Create an enum option. The enum should have at least one element. If
    /// init value is not specified, the first enum value is used.
    pub fn new_enum<S: Into<String> + Clone>(
        opts: &mut CommandLine,
        mut desc: OptDesc<T, S>,
    ) -> Opt<T> {
        // Obtain a reference to the first enum value. We may end up not needing it,
        // but we are validating that it exists.
        let enum_desc = desc
            .values
            .expect("values must be supplied for enum")
            .first()
            .expect("enum must have at least one value");
        // If the init value is missing, use the first enum value.
        if desc.init.is_none() {
            desc.init = Some(enum_desc.1.clone());
        }
        Self::with_parser(opts, desc, parse_disallowed::<T>)
    }
}

impl<U: 'static + Clone> Opt<Option<U>> {
    /// Create a new option with type Option<U>.
    pub fn new_optional<S: Into<String> + Clone>(
        opts: &mut CommandLine,
        mut desc: OptDesc<Option<U>, S>,
    ) -> Opt<Option<U>>
    where
        U: FromStr,
        <U as FromStr>::Err: std::fmt::Display,
    {
        // Set the init value if not specified.
        desc.init = desc.init.or(Some(None));
        Self::with_parser(opts, desc, |s| match U::from_str(s) {
            Ok(o) => Ok(Some(o)),
            Err(e) => Err(format!("{}", e)),
        })
    }
}

impl Opt<bool> {
    /// Create a boolean option that accepts forms like "--pretty=on" or just "--pretty".
    /// If [`OptDesc::max_count`] is 0, it is set to 1.
    /// If [`OptDesc::def_value`] is not set, it is set to true.
    /// If ['OptDesc::expected_value`] is not set, it is set to [`ExpectedValue::Optional`].
    pub fn new_bool<S: Into<String> + From<&'static str> + Clone>(
        opts: &mut CommandLine,
        mut desc: OptDesc<bool, S>,
    ) -> Opt<bool> {
        // By default, disable specifying a bool more than once.
        if desc.max_count == 0 {
            desc.max_count = 1;
        }
        // Set default value to true, so "--flag" will function as "--flag=true".
        desc.def_value = desc.def_value.or(Some(true));
        // Set the init value to false if not specified.
        desc.init = desc.init.or(Some(false));
        // Ordinarily the value should be optional.
        desc.expected_value = desc.expected_value.or(Some(ExpectedValue::Optional));
        // Value description.
        if desc.expected_value != Some(ExpectedValue::Disallowed) {
            desc.value_desc = desc.value_desc.or_else(|| Some(S::from("bool")));
        }
        Self::with_parser(opts, desc, parse_bool)
    }

    /// Create a boolean flag without arguments like "--strict".
    /// If [`OptDesc::max_count`] is 0, it is set to 1.
    /// If [`OptDesc::def_value`] is not set, it is set to true.
    /// If ['OptDesc::expected_value`] is not set, it is set to [`ExpectedValue::Disallowed`].
    pub fn new_flag<S: Into<String> + From<&'static str> + Clone>(
        opts: &mut CommandLine,
        mut desc: OptDesc<bool, S>,
    ) -> Opt<bool> {
        // Prevent a value.
        desc.expected_value = desc.expected_value.or(Some(ExpectedValue::Disallowed));
        Self::new_bool(opts, desc)
    }
}

/// Return "s" if he value is 1, "" otherwise. For use in string formatting.
fn plural(count: usize) -> &'static str {
    cond!(count != 1, "s", "")
}

/// Parse a string into a boolean, accepting various common forms in any case:
/// 1, on, yes, true, y, etc.
pub fn parse_bool(input: &str) -> Result<bool, String> {
    match input.to_ascii_lowercase().as_str() {
        "1" | "on" | "true" | "t" | "yes" | "y" => Ok(true),
        "0" | "off" | "false" | "f" | "no" | "n" => Ok(false),
        _ => Err(format!(
            "'{}' is invalid value for boolean argument. Try 0 or 1",
            input
        )),
    }
}

/// A parser for options that do not take a value. Ordinarily it should not be
/// invoked because such options should have [`ExpectedValue::Disallowed`] set/
pub fn parse_disallowed<T>(_input: &str) -> Result<T, String> {
    Err(String::from("option does not take a value"))
}
