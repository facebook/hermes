# prettier-plugin-hermes-parser

Hermes parser plugin for [Prettier](https://prettier.io/). This plugin enables Prettier to use `hermes-parser` as it's parser. Since Hermes parser uses C++ compiled to WASM it is significantly faster than alternatives such as `flow` or `babel-flow` by as much as 10x.

## Usage

More details on using Prettier plugins: https://prettier.io/docs/en/plugins.html#using-plugins

To then use the parser you will need to instruct Prettier to use `hermes` as the parser for your required files:

```
// .prettierrc
{
  "plugins": ["prettier-plugin-hermes-parser"],
  "overrides": [
    {
      "files": ["*.js", "*.jsx", "*.flow"],
      "options": {
        "parser": "hermes"
      }
    }
  ]
}
```
More details on configuring Prettier parsers: https://prettier.io/docs/en/configuration.html#setting-the-parserdocsenoptionshtmlparser-option
