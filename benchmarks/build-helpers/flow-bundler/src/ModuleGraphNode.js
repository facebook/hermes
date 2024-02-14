/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @flow strict-local
 * @format
 */

import type {ParseResult} from './utils';
import type {
  ESNode,
  Identifier,
  JSXIdentifier,
  ModuleDeclaration,
  Statement,
  Program,
} from 'hermes-estree';

import {parseFile} from './utils';
import * as path from 'path';

export type LocalBinding = string;
export type ImportSource = string;
export type ImportName = string | 'default' | '*';
export type ExportName = string | 'default';
export type ImportBindingSource = {
  source: ImportSource,
  imported: ImportName,
};
export type ReexportSource = {
  source: ImportSource,
  imported: ImportName,
  exported: ExportName | '*',
};
export type ModuleGraphNode = {
  file: string,
  projectRoot: string,
  parseResult: ParseResult,
  moduleName: string,
  importBindingToSource: Map<LocalBinding, ImportBindingSource>,
  exportBindingToSource: Map<LocalBinding, {exported: ExportName}>,
  // Array since order is significant
  reexportSources: Array<ReexportSource>,
};

function processModuleStatement(
  stmt: ModuleDeclaration | Statement,
  moduleGraphNode: ModuleGraphNode,
): void {
  switch (stmt.type) {
    case 'ImportDeclaration': {
      if (stmt.importKind === 'typeof') {
        throw new Error(
          `"typeof" imports not supported yet in "${moduleGraphNode.file}"`,
        );
      }

      // Record source
      const importSource = stmt.source.value;

      // Add mapping for each specifier
      for (const specifier of stmt.specifiers) {
        switch (specifier.type) {
          case 'ImportSpecifier': {
            moduleGraphNode.importBindingToSource.set(specifier.local.name, {
              source: importSource,
              imported: specifier.imported.name,
            });
            break;
          }
          case 'ImportDefaultSpecifier': {
            moduleGraphNode.importBindingToSource.set(specifier.local.name, {
              source: importSource,
              imported: 'default',
            });
            break;
          }
          case 'ImportNamespaceSpecifier': {
            moduleGraphNode.importBindingToSource.set(specifier.local.name, {
              source: importSource,
              imported: '*',
            });
          }
        }
      }

      return;
    }
    case 'ExportNamedDeclaration': {
      if (stmt.source != null) {
        // Record source
        const importSource = stmt.source.value;

        // Add mapping for each specifier
        for (const specifier of stmt.specifiers) {
          moduleGraphNode.reexportSources.push({
            source: importSource,
            imported: specifier.local.name,
            exported: specifier.exported.name,
          });
        }
        return;
      }

      if (stmt.declaration != null) {
        const decl = stmt.declaration;
        switch (decl.type) {
          case 'VariableDeclaration': {
            for (const declarator of decl.declarations) {
              const id = declarator.id;
              switch (id.type) {
                case 'Identifier': {
                  moduleGraphNode.exportBindingToSource.set(id.name, {
                    exported: id.name,
                  });
                  break;
                }
                default: {
                  throw new Error(
                    `Unsupported VariableDeclaration export binding name of type "${id.type}"`,
                  );
                }
              }
            }
            break;
          }
          case 'TypeAlias':
          case 'OpaqueType':
          case 'EnumDeclaration':
          case 'FunctionDeclaration':
          case 'ComponentDeclaration':
          case 'HookDeclaration':
          case 'InterfaceDeclaration':
          case 'ClassDeclaration': {
            const id = decl.id;
            if (id == null) {
              throw new Error(
                `Invalid program, id missing on exported declaration at line: ${decl.loc.start.line}`,
              );
            }
            moduleGraphNode.exportBindingToSource.set(id.name, {
              exported: id.name,
            });
            break;
          }
          default: {
            throw new Error(
              `Unsupported named export declaration type "${decl.type}"`,
            );
          }
        }
        return;
      }

      for (const specifier of stmt.specifiers) {
        moduleGraphNode.exportBindingToSource.set(specifier.local.name, {
          exported: specifier.exported.name,
        });
      }
      return;
    }
    case 'ExportDefaultDeclaration': {
      const decl = stmt.declaration;
      switch (decl.type) {
        case 'FunctionDeclaration':
        case 'ComponentDeclaration':
        case 'ClassDeclaration': {
          const id = decl.id;
          if (id == null) {
            throw new Error(
              `Invalid program, id missing on default exported declaration at line: ${decl.loc.start.line}`,
            );
          }
          moduleGraphNode.exportBindingToSource.set(id.name, {
            exported: 'default',
          });
          break;
        }
        case 'Identifier': {
          moduleGraphNode.exportBindingToSource.set(decl.name, {
            exported: 'default',
          });
          break;
        }
        case 'HookDeclaration': {
          throw new Error(
            `export default hook declarations not supported as they lose their hook specific name of "${decl.id.name}" in "${moduleGraphNode.file}"`,
          );
        }
        default: {
          throw new Error(
            `Non declaration export default expressions not supported. Got expression of type "${decl.type}" in "${moduleGraphNode.file}"`,
          );
        }
      }
      return;
    }
    case 'ExportAllDeclaration': {
      // Record source
      const importSource = stmt.source.value;

      moduleGraphNode.reexportSources.push({
        source: importSource,
        imported: '*',
        exported: stmt.exported != null ? stmt.exported.name : '*',
      });
      return;
    }
    default: {
      return;
    }
  }
}

export async function createModuleGraphNode(
  file: string,
  sourceLocation: string,
  projectRoot: string,
  moduleName: string,
): Promise<ModuleGraphNode> {
  const parseResult = await parseFile(sourceLocation);

  const moduleGraphNode: ModuleGraphNode = {
    file,
    projectRoot,
    moduleName,
    parseResult,
    importBindingToSource: new Map(),
    exportBindingToSource: new Map(),
    reexportSources: [],
  };

  for (const stmt of parseResult.ast.body) {
    processModuleStatement(stmt, moduleGraphNode);
  }

  return moduleGraphNode;
}
