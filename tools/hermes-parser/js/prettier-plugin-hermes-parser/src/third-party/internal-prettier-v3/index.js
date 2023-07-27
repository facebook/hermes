/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @flow
 * @format
 */

import type {Plugin} from 'prettier';
import type {Program} from 'hermes-estree';

import * as flowPluginUntyped from './plugins/flow';
import * as estreePluginUntyped from './plugins/estree';
import * as graphqlPluginUntyped from './plugins/graphql';
import * as postcssPluginUntyped from './plugins/postcss';
import * as htmlPluginUntyped from './plugins/html';
import * as markdownPluginUntyped from './plugins/markdown';
import {printAstToDoc as printAstToDocUntyped} from './ast-to-doc.js';

export const flowPlugin: Plugin<Program> = flowPluginUntyped;
export const estreePlugin: Plugin<Program> = estreePluginUntyped;
export const graphqlPlugin: Plugin<> = graphqlPluginUntyped;
export const postcssPlugin: Plugin<> = postcssPluginUntyped;
export const htmlPlugin: Plugin<> = htmlPluginUntyped;
export const markdownPlugin: Plugin<> = markdownPluginUntyped;

type Doc = mixed;
type PrintAstToDocType = (Program, mixed) => Doc;
export const printAstToDoc: PrintAstToDocType = printAstToDocUntyped;
