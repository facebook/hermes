# hermes-eslint

`hermes-eslint` is a custom parser for [ESLint](https://eslint.org/). It is the recommended parser for use for linting with Flow code.

## Usage

### Flat config (`eslint.config.js`)

To use `hermes-eslint` with ESLint flat config, set it as
`languageOptions.parser`:

```js
module.exports = [
  {
    languageOptions: {
      parser: require('hermes-eslint'),
    },
  },
];
```

You can configure parser options with
`languageOptions.parserOptions`:

```js
module.exports = [
  {
    languageOptions: {
      parser: require('hermes-eslint'),
      parserOptions: {
        sourceType: 'module',
      },
    },
  },
];
```

### Legacy config (`.eslintrc`)

If you are still using legacy ESLint config, specify
`"hermes-eslint"` as the `"parser"` in your `.eslintrc` file:

```json
{
  "parser": "hermes-eslint"
}
```

The ESLint documentation provides more information about [how to configure ESLint](https://eslint.org/docs/latest/use/configure/), including [how to specify a custom parser](https://eslint.org/docs/latest/use/configure/parser).

### Options

You may provide additional configuration for `hermes-eslint` by passing an
object containing configuration options as `parserOptions` in your ESLint
configuration file. In flat config, this is `languageOptions.parserOptions`.
This object may contain the following properties:

```ts
type ParserOptions = {
  /**
   * The identifier that's used for JSX Element creation (after transpilation).
   * This should not be a member expression - just the root identifier (i.e. use "React" instead of "React.createElement").
   *
   * To use the new global JSX transform function, you can explicitly set this to `null`.
   *
   * Defaults to `"React"`.
   */
  jsxPragma?: string | null,

  /**
   * The identifier that's used for JSX fragment elements (after transpilation).
   * If `null`, assumes transpilation will always use a member on `jsxFactory` (i.e. React.Fragment).
   * This should not be a member expression - just the root identifier (i.e. use "h" instead of "h.Fragment").
   *
   * Defaults to `null`.
   */
  jsxFragmentName?: string | null,

  /**
   * The source type of the script.
   *
   * Defaults to `"module"`.
   */
  sourceType?: 'script' | 'module',

  /**
   * Ignore <fbt /> JSX elements when adding references to the module-level `React` variable.
   * FBT is JSX that's transformed to non-JSX and thus references differently
   *
   * https://facebook.github.io/fbt/
   */
  fbt?: boolean,
};
```

```json
{
  "parser": "hermes-eslint",
  "parserOptions": {
    "sourceType": "module"
  }
}
```
