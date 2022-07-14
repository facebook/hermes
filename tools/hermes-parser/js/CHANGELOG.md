## 0.9.0

### `hermes-transform`

- Fix unnecessary newlines being added during printing.
- Fix handling of arrays with nullable elements in `shallowCloneArray` (like an array's `.elements` property).

### `hermes-eslint`

- Add handling for FBT's `fbs` tags in scope analysis.
- Add handling for function type `this` param in scope analysis.

### `hermes-estree`

- Improve types for private brand checks (`#priv in this`)
- Add explicitly typed `parent` for nodes with small sets of known parent types
