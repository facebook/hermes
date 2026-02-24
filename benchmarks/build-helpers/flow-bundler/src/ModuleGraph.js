/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @flow strict-local
 * @format
 */

import type {
  ExportName,
  ImportBindingSource,
  ImportSource,
  LocalBinding,
  ModuleGraphNode,
  ReexportSource,
} from './ModuleGraphNode';
import type {
  ESNode,
  Identifier,
  JSXIdentifier,
  ModuleDeclaration,
  Statement,
  Program,
} from 'hermes-estree';

import {createModuleGraphNode} from './ModuleGraphNode';
import {transformModule} from './TransformModule';
import {parseFile} from './utils';
import * as path from 'path';

const PACKAGES_DIR_NAME = 'packages';

function getPathSourceWithJSExtension(source: string): string {
  return path.extname(source) === '.js' ? source : source + '.js';
}

function getPackageNameFromFile(projectRoot: string, file: string): ?string {
  const packagesDir = path.join(projectRoot, PACKAGES_DIR_NAME);
  if (!file.startsWith(packagesDir)) {
    return null;
  }

  const packageParts = file.substring(packagesDir.length + 1).split('/');
  if (packageParts[0].startsWith('@')) {
    return packageParts[0].substring(1) + '_' + packageParts[1];
  }

  return packageParts[0];
}

function resolveModuleLocation(
  moduleGraphNode: ModuleGraphNode,
  source: ImportSource,
): string {
  // Absolute path, leave as is
  if (path.isAbsolute(source)) {
    return source;
  }

  // Relative path, resolve to absolute path.
  if (source.startsWith('.')) {
    return path.join(
      path.dirname(moduleGraphNode.file),
      getPathSourceWithJSExtension(source),
    );
  }

  // Root relative path, resolve to project root
  if (source.startsWith('@/')) {
    const sourceWithoutAt = source.substring(2);
    const packageName = getPackageNameFromFile(
      moduleGraphNode.projectRoot,
      moduleGraphNode.file,
    );

    // If we are inside a package, then we need to resolve relative to that package.
    if (packageName != null) {
      return path.join(
        moduleGraphNode.projectRoot,
        PACKAGES_DIR_NAME,
        packageName,
        getPathSourceWithJSExtension(sourceWithoutAt),
      );
    }

    return path.join(
      moduleGraphNode.projectRoot,
      getPathSourceWithJSExtension(sourceWithoutAt),
    );
  }

  // Package path, resolve to packages directory
  const pathParts = source.split('/');
  if (
    pathParts.length === 1 ||
    // Scoped package path
    (pathParts[0].startsWith('@') && pathParts.length === 2)
  ) {
    pathParts.push('index.js');
  }
  return path.join(
    moduleGraphNode.projectRoot,
    PACKAGES_DIR_NAME,
    getPathSourceWithJSExtension(pathParts.join('/')),
  );
}

export type ModuleInfo = {
  exportToName: Map<ExportName, LocalBinding>,
  exportBindingToName: Map<LocalBinding, LocalBinding>,
  importBindingToName: Map<
    LocalBinding,
    // Named and default import
    | {type: 'name', binding: LocalBinding}
    // Namespace import
    | {type: 'namespace', bindings: Map<LocalBinding, LocalBinding>},
  >,
};

/**
 * Resolve our export bindings to unique identifier names.
 */
function resolveExportNames(
  moduleGraphNode: ModuleGraphNode,
  moduleInfo: ModuleInfo,
): void {
  for (const [local, {exported}] of moduleGraphNode.exportBindingToSource) {
    const exportedName = moduleGraphNode.moduleName + '$' + exported;
    moduleInfo.exportBindingToName.set(local, exportedName);
    moduleInfo.exportToName.set(exported, exportedName);
  }
}

async function resolveReexportName(
  reexportSource: ReexportSource,
  moduleGraphNode: ModuleGraphNode,
  moduleInfo: ModuleInfo,
  buildModuleGraph: string => Promise<ModuleInfo>,
): Promise<void> {
  const resolvedReexportModuleInfo = await buildModuleGraph(
    resolveModuleLocation(moduleGraphNode, reexportSource.source),
  );

  if (reexportSource.imported === '*') {
    if (reexportSource.exported !== '*') {
      // `export * as X from "Y"` is not yet supported.
      throw new Error(`Export all as name currently not supported.`);
    }

    // `export * from "Y"`
    // Copy over all export bindings from the re-exported module to this module's export map.
    for (const [
      exported,
      exportedName,
    ] of resolvedReexportModuleInfo.exportToName) {
      moduleInfo.exportToName.set(exported, exportedName);
    }
    return;
  }

  // `export X as * from "Y"` is invalid syntax.
  if (reexportSource.exported === '*') {
    throw new Error(`Invalid syntax.`);
  }

  // `export {X} from "Y"`
  // `export {X as Z} from "Y"`
  // Copy over a specific export from the re-exported module to this module's export map.
  const exportedName = resolvedReexportModuleInfo.exportToName.get(
    reexportSource.imported,
  );
  if (exportedName == null) {
    throw new Error(
      `Could not find export "${reexportSource.imported}" in module "${reexportSource.source}" from "${moduleGraphNode.file}".`,
    );
  }
  moduleInfo.exportToName.set(reexportSource.exported, exportedName);
}

async function resolveReexportNames(
  moduleGraphNode: ModuleGraphNode,
  moduleInfo: ModuleInfo,
  buildModuleGraph: string => Promise<ModuleInfo>,
): Promise<void> {
  for (const reexportSource of moduleGraphNode.reexportSources) {
    await resolveReexportName(
      reexportSource,
      moduleGraphNode,
      moduleInfo,
      buildModuleGraph,
    );
  }
}

async function resolveImportName(
  local: LocalBinding,
  importSource: ImportBindingSource,
  moduleGraphNode: ModuleGraphNode,
  moduleInfo: ModuleInfo,
  buildModuleGraph: string => Promise<ModuleInfo>,
): Promise<void> {
  const resolvedImportModuleInfo = await buildModuleGraph(
    resolveModuleLocation(moduleGraphNode, importSource.source),
  );

  if (importSource.imported === '*') {
    // `import * as X from "Y"`
    moduleInfo.importBindingToName.set(local, {
      type: 'namespace',
      bindings: resolvedImportModuleInfo.exportToName,
    });
    return;
  }

  // `import X from "Y"`
  // `import {X} from "Y"`
  // `import {X as Z} from "Y"`
  const exportedName = resolvedImportModuleInfo.exportToName.get(
    importSource.imported,
  );
  if (exportedName == null) {
    throw new Error(
      `Could not find import "${importSource.imported}" in module "${importSource.source}" from "${moduleGraphNode.file}".`,
    );
  }
  moduleInfo.importBindingToName.set(local, {
    type: 'name',
    binding: exportedName,
  });
}

async function resolveImportNames(
  moduleGraphNode: ModuleGraphNode,
  moduleInfo: ModuleInfo,
  buildModuleGraph: string => Promise<ModuleInfo>,
): Promise<void> {
  for (const [local, importSource] of moduleGraphNode.importBindingToSource) {
    await resolveImportName(
      local,
      importSource,
      moduleGraphNode,
      moduleInfo,
      buildModuleGraph,
    );
  }
}

export type ModuleOverride = {
  target: string,
  override: string,
};

export async function createModuleGraph(
  projectRoot: string,
  entrypoints: Array<string>,
  moduleOverrides: Array<ModuleOverride>,
): Promise<Array<{file: string, ast: Program, code: string}>> {
  const moduleNameCounter = new Map<string, number>();
  const moduleStack: Array<string> = [];
  const moduleInfoMap = new Map<string, ModuleInfo>();
  const bundleSources = [];

  const overrideModuleMap = new Map<string, string>();
  const overrideModules = new Set<string>();
  for (const {target, override} of moduleOverrides) {
    overrideModuleMap.set(target, override);
    overrideModules.add(override);
  }

  function getModuleName(fileName: string): string {
    let moduleName = path.basename(fileName, '.js');

    // Prefix module with package name if within package.
    const packageName = getPackageNameFromFile(projectRoot, fileName);
    if (packageName != null) {
      moduleName = packageName + '_' + moduleName;
    }

    // This prefix being uppercase is required for
    // the JSX transform as it considers lower case JSXIdentifiers as host elements
    // and transforms them differently.
    const moduleNamePrefix = 'M$';

    // Dedupe file name conflicts by adding index suffix to module name.
    const moduleIndex = moduleNameCounter.get(moduleName) ?? 0;
    moduleNameCounter.set(moduleName, moduleIndex + 1);
    const sanitizedModuleName = moduleName.replace(/[^a-zA-Z0-9_$]/g, '_');
    if (moduleIndex === 0) {
      return moduleNamePrefix + sanitizedModuleName;
    }
    return moduleNamePrefix + sanitizedModuleName + '$' + moduleIndex;
  }

  async function buildModuleGraph(fileName: string): Promise<ModuleInfo> {
    const resolvedModuleInfo = moduleInfoMap.get(fileName);
    if (resolvedModuleInfo != null) {
      return resolvedModuleInfo;
    }

    // Mark module as being built
    if (moduleStack.includes(fileName)) {
      throw new Error(
        `Circular dependency detected, stack:\n ${moduleStack.join(
          '\n -> ',
        )}\n ---> ${fileName}`,
      );
    }
    moduleStack.push(fileName);

    if (overrideModules.has(fileName)) {
      throw new Error(
        `Module overrides can be directly imported, override module "${fileName}".`,
      );
    }

    // Update the source location if there is an override.
    let sourceLocation = fileName;
    const overrideModule = overrideModuleMap.get(fileName);
    if (overrideModule != null) {
      sourceLocation = overrideModule;
    }

    // Extract module import/export information from AST.
    const moduleGraphNode = await createModuleGraphNode(
      fileName,
      sourceLocation,
      projectRoot,
      getModuleName(fileName),
    );

    const moduleInfo: ModuleInfo = {
      exportToName: new Map(),
      exportBindingToName: new Map(),
      importBindingToName: new Map(),
    };
    moduleInfoMap.set(fileName, moduleInfo);

    // Resolve export names
    resolveExportNames(moduleGraphNode, moduleInfo);

    // Resolve reexports (and their dependent modules)
    await resolveReexportNames(moduleGraphNode, moduleInfo, buildModuleGraph);

    // Resolve imports (and any needed dependent modules)
    await resolveImportNames(moduleGraphNode, moduleInfo, buildModuleGraph);

    const builtModule = await transformModule(moduleGraphNode, moduleInfo);

    bundleSources.push({
      file: sourceLocation,
      ast: builtModule.ast,
      code: builtModule.code,
    });

    // Pop module from stack
    moduleStack.pop();

    return moduleInfo;
  }

  for (const entrypoint of entrypoints) {
    await buildModuleGraph(entrypoint);
  }

  return bundleSources;
}
