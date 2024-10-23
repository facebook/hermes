/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: LC_ALL=fr_FR _HERMES_TEST_LOCALE=fr_FR %hermes %s
// REQUIRES: intl

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

let testName = 'Test-Default-Construct';
let expected = Object.assign({}, DEFAULT_RESOLVED_OPTIONS);
// See run instruction at the top of the file in specifying fr-FR as default locale.
expected.locale = 'fr-FR';
let actual = new Intl.Collator().resolvedOptions();
verifyResolvedOptions(testName, expected, actual);

testName = 'Test-No-Options';
expected = Object.assign({}, DEFAULT_RESOLVED_OPTIONS);
expected.locale = 'en-GB';
let locales = 'en-GB';
actual = new Intl.Collator(locales).resolvedOptions();
verifyResolvedOptions(testName, expected, actual);

testName = 'Test-Usage-Sort'
expected = Object.assign({}, DEFAULT_RESOLVED_OPTIONS);
expected.locale = 'pt-BR';
locales = ['pt-br'];
let options = {
  usage: 'sort',
};
testResolvedOptions(testName, expected, locales, options);

testName = 'Test-Usage-Search'
expected = Object.assign({}, DEFAULT_RESOLVED_OPTIONS);
expected.locale = 'en-US';
expected.usage = 'search';
locales = ['en-US', 'pt-BR'];
options = {
  usage: 'search',
};
testResolvedOptions(testName, expected, locales, options);

testName = 'Test-Locale-With-Extensions'
expected = Object.assign({}, DEFAULT_RESOLVED_OPTIONS);
expected.locale = 'de-DE-u-co-phonebk-kf-upper-kn';
expected.collation = 'phonebk';
expected.caseFirst = 'upper';
expected.numeric = true;
locales = ['zz', 'de-DE-u-Co-phoneBK-kf-Upper-kn-TRUE'];
options = undefined;
testResolvedOptions(testName, expected, locales, options);

testName = 'Test-Options-Overrides'
expected = Object.assign({}, DEFAULT_RESOLVED_OPTIONS);
expected.locale = 'es-ES';
expected.collation = 'eor';
expected.caseFirst = 'lower';
expected.numeric = false;
locales = 'es-ES-u-co-emoji-kf-upper-kn-true';
options = {
  collation: 'eor',
  caseFirst: 'lower',
  numeric: false,
};
testResolvedOptions(testName, expected, locales, options);

testName = 'Test-Usage-Search-Overrides-Collation-Extension'
expected = Object.assign({}, DEFAULT_RESOLVED_OPTIONS);
expected.locale = 'it-IT';
expected.usage = 'search';
locales = 'it-IT-u-co-eor';
options = {
  usage: 'search',
};
testResolvedOptions(testName, expected, locales, options);

testName = 'Test-Usage-Search-Overrides-Collation-Option'
expected = Object.assign({}, DEFAULT_RESOLVED_OPTIONS);
expected.locale = 'it-IT';
expected.usage = 'search';
locales = 'it-IT-u-co-emoji';
options = {
  usage: 'search',
  collation: 'eor',
};
testResolvedOptions(testName, expected, locales, options);

testName = 'Test-Options'
expected = Object.assign({}, DEFAULT_RESOLVED_OPTIONS);
expected.locale = 'si';
expected.sensitivity = 'base';
expected.ignorePunctuation = true;
expected.collation = 'dict';
expected.caseFirst = 'upper';
expected.numeric = true;
locales = ['zz', 'Si'];
options = {
  localeMatcher: 'lookup',
  sensitivity: 'base',
  ignorePunctuation: true,
  numeric: true,
  caseFirst: 'upper',
  collation: 'dict',
};
testResolvedOptions(testName, expected, locales, options);