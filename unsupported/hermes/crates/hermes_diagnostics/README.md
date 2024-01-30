# hermes_diagnostics

Types for representing compiler diagnostics. Includes a general-purpose representation
of diagnostics with related information which can be converted into `miette::Diagnostic` to exploit miette's pretty printing of errors.

Unlike miette, lsp_types, and other diagnostic libraries, the error severities are categorized
to allow different tools to report them at different levels. For example, a tool may choose to
report todo errors or ignore them.
