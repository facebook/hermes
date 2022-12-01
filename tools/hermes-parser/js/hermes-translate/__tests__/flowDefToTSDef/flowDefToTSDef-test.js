/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @flow strict-local
 * @format
 */

// $FlowExpectedError[cannot-resolve-module]
import prettierConfig from '../../../.prettierrc.json';
import {translateFlowDefToTSDef} from '../../src';
import {ExpectedTranslationError} from '../../src/utils/ErrorUtils';
import fs from 'fs';
import glob from 'glob';
import path from 'path';
import {} from 'hermes-parser';

import 'jest-specific-snapshot';

const FIXTURES_DIR = path.resolve(__dirname, 'fixtures');
const FIXTURES = glob
  .sync(`${FIXTURES_DIR}/**/spec.js`)
  .map(file => {
    const parsed = path.parse(file);
    const contents = fs.readFileSync(file, 'utf8');
    const fixtureName = path.relative(FIXTURES_DIR, parsed.dir);
    return {
      contents,
      name: fixtureName,
      fixturePath: file,
      translateSnapshotPath: path.join(parsed.dir, `translate-result.shot`),
    };
  })
  .filter(Boolean);

// set this to the path of the test to only run that fixture
const ONLY = '';

describe('flowDefToTSDef', () => {
  if (FIXTURES.length === 0) {
    it.skip('no fixtures found', () => {});
  }

  for (const fixture of FIXTURES) {
    (ONLY === fixture.name ? it.only : it)(fixture.name, () => {
      // ensure we don't accidentally have an empty fixture
      expect(fixture.contents).not.toBe('');

      try {
        const printedResult = translateFlowDefToTSDef(
          fixture.contents,
          prettierConfig,
        );
        expect(printedResult).toMatchSpecificSnapshot(
          fixture.translateSnapshotPath,
        );

        // TODO - parse the printed result back with TS and assert there aren't any errors
      } catch (e) {
        if (e instanceof ExpectedTranslationError) {
          // errors of type TranslationError are expected, allowed and should be logged
          expect(e).toMatchSpecificSnapshot(fixture.translateSnapshotPath);
        } else {
          // unexpected errors should crash the test
          // this ensures we don't accidentally write a snapshot for a test that's unexpectedly crashing
          throw e;
        }
      }
    });
  }
});
