/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes %s
// REQUIRES: intl
// UNSUPPORTED: apple

function assert(pred, str) {
  if (!pred) {
    throw new Error('assertion failed' + (str === undefined ? '' : (': ' + str)));
  }
}

function verifyResolvedOptions(testName, expected, actual) {
  for (const [option, value] of Object.entries(expected)) {
    assert(Object.hasOwn(actual, option), `${testName} failed, missing '${option}' option`);
    const actualValue = actual[option];
    assert(value === actualValue, `${testName} failed, for option='${option}', expected='${value}', actual='${actualValue}'`);
  }
}

function testResolvedOptions(testName, expected, locales, options) {
  let actual = new Intl.Collator(locales, options).resolvedOptions();
  verifyResolvedOptions(testName, expected, actual);
}

const DEFAULT_RESOLVED_OPTIONS = {
  locale: undefined,
  usage: 'sort',
  sensitivity: 'variant',
  ignorePunctuation: false,
  collation: 'default',
  numeric: false,
  caseFirst: 'false',
};
Object.freeze(DEFAULT_RESOLVED_OPTIONS);

let testName = 'Test-Locale-With-Extensions-Not-Supported-By-Locale'
let expected = Object.assign({}, DEFAULT_RESOLVED_OPTIONS);
expected.locale = 'ja-JP-u-kf-lower-kn-false';
expected.caseFirst = 'lower';
expected.numeric = false;
let locales = 'ja-JP-u-co-phonebk-kf-lower-kn-false';
let options = undefined;
testResolvedOptions(testName, expected, locales, options);

testName = 'Test-Locale-With-Co-Extension-Search'
expected = Object.assign({}, DEFAULT_RESOLVED_OPTIONS);
expected.locale = 'en-US-u-kn';
expected.numeric = true;
locales = 'en-US-u-co-search-kn';
options = {
  collation: 'search',
};
testResolvedOptions(testName, expected, locales, options);

testName = 'Test-Locale-With-Co-Extension-Standard'
expected = Object.assign({}, DEFAULT_RESOLVED_OPTIONS);
expected.locale = 'en-GB-u-kf-false';
expected.caseFirst = 'false';
locales = 'en-GB-u-co-standard-kf-false';
options = {
  collation: 'standard',
};
testResolvedOptions(testName, expected, locales, options);