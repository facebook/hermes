# hermes_semantic_analysis

Implements semantic analysis of JavaScript name resolution and scope information over the `hermes_estree` AST. Eventually this
analysis will be extended to cover Flow and TypeScript in addition to the core JS and JSX language.

NOTE: this analsyis _assumes strict mode_ and does not support legacy non-strict semantics.
