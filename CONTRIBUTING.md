# Contributing to Hermes
We want to make contributing to this project simple and convenient.

## Code of Conduct
Facebook has adopted a Code of Conduct that we expect project participants to
adhere to. Please [read the full text](https://code.fb.com/codeofconduct/) so
that you can understand what actions will and will not be tolerated.

## Our Development Process
Facebook's internal repository remains the source of truth. It is
automatically synchronized with GitHub. Contributions can be made through
regular GitHub pull requests.

## Pull Requests
We actively welcome your pull requests. If you are planning on doing a larger
chunk of work or want to change an external facing API, make sure to file an
issue first to get feedback on your idea.

1. Fork the repo and create your branch from `main`.
2. If you've added code that should be tested, add tests.
3. Ensure the test suite passes and your code lints.
4. Consider squashing your commits (`git rebase -i`). One intent alongside one
   commit makes it clearer for people to review and easier to understand your
   intention.
5. If you haven't already, complete the Contributor License Agreement ("CLA").

## Copyright Notice for files
Copy and paste this to the top of your new file(s):

```
/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */
```

## Contributor License Agreement ("CLA")
In order to accept your pull request, we need you to submit a CLA. You only need
to do this once to work on any of Facebook's open source projects.

Complete your CLA here: <https://code.facebook.com/cla>

## Issues
We use GitHub issues to track public bugs. Please ensure your description is
clear and has sufficient instructions to be able to reproduce the issue.

Facebook has a [bounty program](https://www.facebook.com/whitehat/) for the
safe disclosure of security bugs. In those cases, please go through the process
outlined on that page and do not file a public issue.

## Coding Style
* The Hermes coding style is generally based on the
  [LLVM Coding Standards](https://llvm.org/docs/CodingStandards.html).
* Match the style you see used in the rest of the project. This includes
  formatting, naming things in code, naming things in documentation.
* Run `clang-format`, using the provided `.clang-format` configuration.

## License
By contributing to Hermes, you agree that your contributions will be licensed
under the LICENSE file in the root directory of this source tree.
