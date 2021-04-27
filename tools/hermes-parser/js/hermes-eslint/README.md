# hermes-eslint
A custom parser for [ESLint](https://eslint.org/) built from the Hermes engine's parser compiled to WebAssembly. The Hermes parser supports ES6, Flow, and JSX syntax, which are parsed into an ESTree AST and then analyzed to determine scope information in a format that can be consumed by ESLint.

## Usage
The `hermes-eslint` package is a [custom parser](https://eslint.org/docs/developer-guide/working-with-custom-parsers) for ESLint. To use `hermes-eslint` as the parser for ESLint in your project you must specify `"hermes-eslint"` as the `"parser"` in your ESLint configuration file:

**.eslintrc**
```js
{
  "parser": "hermes-parser"
}
```

The ESLint documentation provides more information about [how to configure ESLint](https://eslint.org/docs/user-guide/configuring/), including [how to specify a custom parser](https://eslint.org/docs/user-guide/configuring/plugins#specifying-parser).

### Options

You may provide additional configuration for `hermes-eslint` by passing an object containing configuration options as the `"parserOptions"` in your ESLint configuration file. This object may contain the following properties:
- **sourceType**: `"module"` or `"script"`, defaults to `"module"`

**.eslintrc**
```js
{
  "parser": "hermes-parser",
  "parserOptions": {
    "sourceType": "module"
  }
}
```
