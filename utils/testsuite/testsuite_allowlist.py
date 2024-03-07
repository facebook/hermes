# Tests which use `const` but are not broken because
# they don't specifically depend on the defined behavior
# of `const`
CONST_ALLOWLIST = [
    # Allow all tests in intl402
    "test262/test/intl402/"
]
