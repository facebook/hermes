# prettier-plugin-hermes-parser

Hermes parser plugin for [Prettier](https://prettier.io/). Unless you want to be on the bleeding edge, you should use the official [`@prettier/plugin-hermes`](https://www.npmjs.com/package/@prettier/plugin-hermes) instead.

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
