/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @flow strict-local
 * @format
 */

'use strict';

import type {Variable, ScopeManager, Scope} from 'hermes-eslint';
import type {
  ESNode,
  Identifier,
  JSXIdentifier,
  ModuleDeclaration,
  Statement,
  Program,
} from 'hermes-estree';
import type {ModuleInfo} from './ModuleGraph';
import type {LocalBinding, ModuleGraphNode} from './ModuleGraphNode';

import {transformAST} from 'hermes-transform/dist/transform/transformAST';
import {print, t} from 'hermes-transform';
import * as path from 'path';

export async function transformModule(
  moduleGraphNode: ModuleGraphNode,
  moduleInfo: ModuleInfo,
): Promise<string> {
  const {ast, astWasMutated, mutatedCode} = transformAST(
    moduleGraphNode.parseResult,
    context => {
      function getVariableByName(scope: Scope, name: string): ?Variable {
        for (const innerVariable of scope.variables) {
          if (innerVariable.name === name) {
            return innerVariable;
          }
        }

        return null;
      }

      function rewriteBindingReferences(variable: Variable, name: string) {
        const refs = new Set<Identifier | JSXIdentifier>();
        for (const def of variable.defs) {
          switch (def.type) {
            // Ensure the def name is also renamed
            case 'Variable':
            case 'Parameter':
            case 'Type':
            case 'TypeParameter':
            case 'Enum':
            case 'ComponentName':
            case 'FunctionName': {
              refs.add(def.name);
              break;
            }

            // Classes create an inner scope which contains the references to the class name
            case 'ClassName': {
              const innerScope = context.getScope(def.name);
              const innerVariable = getVariableByName(
                innerScope,
                variable.name,
              );
              if (innerVariable == null) {
                throw new Error(
                  context.buildCodeFrame(
                    def.name,
                    `Could not find inner scope name for "${variable.name}"`,
                  ),
                );
              }
              for (const ref of innerVariable.references) {
                refs.add(ref.identifier);
              }
              refs.add(def.name);
              break;
            }

            // Do nothing, these get removed anyway.
            case 'ImportBinding': {
              break;
            }

            // Should not be possible.
            case 'CatchClause': {
              throw new Error(
                context.buildCodeFrame(
                  def.name,
                  `Tried to rename CatchClause which is not supported.`,
                ),
              );
            }
          }
        }

        for (const ref of variable.references) {
          const id = ref.identifier;
          refs.add(id);
          /**
           * Current version of hermes-eslint does not find `JSXClosingElement` `JSXIdentifiers`,
           * so we need to manually add them. 0.17.0 should include this logic.
           *
           * e.g.
           * <Text>
           *  ^^^^ From here
           *   foo
           * </Text>
           *   ^^^^ Also add this
           */
          if (
            id.type === 'JSXIdentifier' &&
            id.parent.type === 'JSXOpeningElement' &&
            id.parent.name === id
          ) {
            const closingElem = id.parent.parent.closingElement;
            if (
              closingElem != null &&
              closingElem.name.type === 'JSXIdentifier'
            ) {
              refs.add(closingElem.name);
            }
          }
        }

        for (const ref of refs) {
          context.modifyNodeInPlace(ref, {
            name,
          });
        }
      }

      function rewriteNamespaceBindingReferences(
        variable: Variable,
        bindings: Map<LocalBinding, LocalBinding>,
      ) {
        for (const ref of variable.references) {
          const container = ref.identifier.parent;
          switch (container.type) {
            case 'MemberExpression': {
              switch (container.property.type) {
                case 'Identifier': {
                  const propertyName = container.property.name;
                  const importNamespaceName = bindings.get(propertyName);
                  if (importNamespaceName == null) {
                    throw new Error(
                      context.buildCodeFrame(
                        container.property,
                        `Namespace referenced non existent property "${propertyName}"`,
                      ),
                    );
                  }
                  context.replaceNode(
                    container,
                    t.Identifier({name: importNamespaceName}),
                    {keepComments: true},
                  );
                  break;
                }
                default: {
                  throw new Error(
                    context.buildCodeFrame(
                      container.property,
                      `Non identifier reference of namespace object, type "${container.type}"`,
                    ),
                  );
                }
              }
              break;
            }
            default: {
              throw new Error(
                context.buildCodeFrame(
                  container,
                  `Non member expression reference of type "${container.type}"`,
                ),
              );
            }
          }
        }
      }

      function rewriteModuleVariable(variable: Variable) {
        const exportBinding = moduleInfo.exportBindingToName.get(variable.name);
        if (exportBinding != null) {
          rewriteBindingReferences(variable, exportBinding);
          return;
        }

        const importBinding = moduleInfo.importBindingToName.get(variable.name);
        if (importBinding != null) {
          switch (importBinding.type) {
            case 'name': {
              rewriteBindingReferences(variable, importBinding.binding);
              return;
            }
            case 'namespace': {
              rewriteNamespaceBindingReferences(
                variable,
                importBinding.bindings,
              );
              return;
            }
          }
        }

        rewriteBindingReferences(
          variable,
          moduleGraphNode.moduleName + '$INTERNAL$' + variable.name,
        );
      }

      return {
        Program(node) {
          const globalScope = context.getScope();
          const moduleScope = globalScope.childScopes[0];

          // $FlowExpectedError[cannot-write] Yes yes, i'm bad.
          node.docblock = null;

          for (const stmt of node.body) {
            switch (stmt.type) {
              case 'ImportDeclaration':
              case 'ExportAllDeclaration': {
                context.removeStatement(stmt);
                break;
              }
              case 'ExportNamedDeclaration': {
                if (stmt.declaration != null) {
                  context.replaceNode(stmt, stmt.declaration, {
                    keepComments: true,
                  });
                  break;
                }

                context.removeStatement(stmt);
                break;
              }
              case 'ExportDefaultDeclaration': {
                switch (stmt.declaration.type) {
                  case 'FunctionDeclaration':
                  case 'ClassDeclaration':
                  case 'ComponentDeclaration': {
                    context.replaceNode(stmt, stmt.declaration, {
                      keepComments: true,
                    });
                    break;
                  }
                  default: {
                    // Expression
                    throw new Error(
                      context.buildCodeFrame(
                        stmt,
                        `Export default expressions (type: "${stmt.declaration.type}") not supported`,
                      ),
                    );
                  }
                }
                break;
              }
            }
          }

          for (const variable of moduleScope.variables) {
            rewriteModuleVariable(variable);
          }
        },
      };
    },
  );
  if (!astWasMutated) {
    return moduleGraphNode.parseResult.code;
  }

  return print(ast, mutatedCode, {});
}
