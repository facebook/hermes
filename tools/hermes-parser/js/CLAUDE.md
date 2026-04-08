# Hermes Parser JS Packages

This directory contains the JavaScript packages for the Hermes parser: `hermes-estree`, `hermes-parser`, `hermes-transform`, `hermes-eslint`, `flow-api-translator`, and `babel-plugin-syntax-hermes-parser`.

## Prerequisites

The WASM parser must be built before running tests. It is not checked in.

```bash
# 1. Build the WASM parser
(cd xplat/static_h && buck2 build --target-platforms ovr_config//platform/wasm:emcc-release :hermes-parser-wasm.js --show-full-output)

# 2. Copy it to dist/
mkdir -p xplat/static_h/tools/hermes-parser/js/hermes-parser/dist
cp <output_path_from_step_1> xplat/static_h/tools/hermes-parser/js/hermes-parser/dist/HermesParserWASM.js
```

## Running Tests

Use `js1 jest` from the fbsource root with `--config` pointing to this directory's jest config:

```bash
NODE_OPTIONS="--experimental-vm-modules" js1 jest \
  --config xplat/static_h/tools/hermes-parser/js/jest.config.js \
  "<test-pattern>"
```

### Common test patterns

| Pattern | What it runs |
|---------|-------------|
| `flowDefToTSDef-test` | flow-api-translator: Flow → TypeScript |
| `TSDefToFlowDef-test` | flow-api-translator: TypeScript → Flow |
| `flowToFlowDef-test` | flow-api-translator: Flow → Flow definitions |
| `TypeAnnotations-test` | hermes-parser: type annotation parsing |
| `ClassProperty-test` | hermes-parser: class property parsing |
| `ObjectProperty-test` | hermes-parser: object property parsing |

### Notes

- `NODE_OPTIONS="--experimental-vm-modules"` is required — without it, prettier's dynamic import fails.
- `--config` is required — without it, `js1 jest` searches from the repo root and finds nothing.
- If tests fail with "Cannot find module HermesParserWASM", the WASM parser hasn't been built (see Prerequisites).
