/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @flow
 * @format
 */

import {
  jsTypeToCppType,
  toCppNamespace,
  toCppType,
  type JsTypeString,
} from './Converters';

export class Property {
  domain: string;
  name: string;
  description: ?string;
  exported: ?boolean;
  experimental: ?boolean;
  optional: ?boolean;

  static create(domain: string, obj: any): Property {
    if (obj.$ref) {
      return new RefProperty(domain, obj);
    } else if (obj.type === 'array') {
      return new ArrayProperty(domain, obj);
    }
    return new PrimitiveProperty(domain, obj);
  }

  static createArray(
    domain: string,
    parent: string,
    elements: Array<any>,
    ignoreExperimental: boolean,
    includeExperimental: Set<string>,
  ): Array<Property> {
    let props = elements.map(elem => Property.create(domain, elem));
    if (ignoreExperimental) {
      props = props.filter(prop => {
        return (
          !prop.experimental ||
          includeExperimental.has(domain + '.' + parent + '.' + prop.name)
        );
      });
    }
    return props;
  }

  constructor(domain: string, obj: any) {
    this.domain = domain;
    this.name = obj.name;
    this.description = obj.description;
    this.exported = obj.exported;
    this.experimental = obj.experimental;
    this.optional = obj.optional;
  }

  getRefDebuggerName(): ?string {
    throw new Error('subclass must implement');
  }

  getFullCppType(): string {
    throw new Error('subclass must implement');
  }

  getCppIdentifier(): string {
    // need to munge identifier if it matches a C++ keyword like "this"
    if (this.name === 'this') {
      return 'thisObj';
    }
    return this.name;
  }

  getInitializer(): string {
    throw new Error('subclass must implement');
  }
}

function maybeWrapOptional(
  type: string,
  optional: ?boolean,
  recursive: ?boolean,
  cyclical: ?boolean,
) {
  if (cyclical) {
    // n.b. need to use unique_ptr with a custom deleter since
    // due to the type cycle we only have the fwd def here and
    // the type is incomplete
    return `std::unique_ptr<${type}, std::function<void(${type}*)>>`;
  } else if (optional) {
    return recursive ? `std::unique_ptr<${type}>` : `std::optional<${type}>`;
  }
  return type;
}

function toDomainAndId(
  curDomain: string,
  absOrRelRef: string,
): [string, string] {
  let [domain, id] = ['', ''];

  // absOrRelRef can be:
  // 1) absolute ref with a "." referencing a type from another namespace, like
  //    "Runtime.ExceptionDetails"
  // 2) relative ref without a "." referencing a type in current domain, like
  //    "Domain"
  const i = absOrRelRef.indexOf('.');
  if (i === -1) {
    domain = curDomain;
    id = absOrRelRef;
  } else {
    domain = absOrRelRef.slice(0, i);
    id = absOrRelRef.slice(i + 1);
  }

  return [domain, id];
}

function toFullCppType(curDomain: string, absOrRelRef: string) {
  const [domain, id] = toDomainAndId(curDomain, absOrRelRef);
  return `${toCppNamespace(domain)}::${toCppType(id)}`;
}

class PrimitiveProperty extends Property {
  type: JsTypeString;

  constructor(domain: string, obj: any) {
    super(domain, obj);
    this.type = obj.type;
  }

  getRefDebuggerName(): ?string {
    return undefined;
  }

  getFullCppType(): string {
    return maybeWrapOptional(jsTypeToCppType(this.type), this.optional);
  }

  getInitializer(): string {
    // std::optional doesn't need to be explicitly zero-init
    if (this.optional) {
      return '';
    }

    // we want to explicitly zero-init bool, int, and double
    const type = this.type;
    if (type === 'boolean' || type === 'integer' || type === 'number') {
      return '{}';
    }

    // std::string has sensible default constructor, no need to explicitly
    // zero-init
    return '';
  }
}

class RefProperty extends Property {
  $ref: string;
  recursive: ?boolean;
  cyclical: ?boolean;

  constructor(domain: string, obj: any) {
    super(domain, obj);
    this.$ref = obj.$ref;
    this.recursive = obj.recursive;
    this.cyclical = obj.cyclical;
  }

  getRefDebuggerName(): ?string {
    const [domain, id] = toDomainAndId(this.domain, this.$ref);
    return `${domain}.${id}`;
  }

  getFullCppType(): string {
    const fullCppType = toFullCppType(this.domain, this.$ref);
    return maybeWrapOptional(
      `${fullCppType}`,
      this.optional,
      this.recursive,
      this.cyclical,
    );
  }

  getInitializer(): string {
    // must zero-init non-optional ref props since the ref could just be an
    // alias to a C++ primitive type like int which we always want to zero-init
    const fullCppType = toFullCppType(this.domain, this.$ref);
    return this.cyclical
      ? `{nullptr, deleter<${fullCppType}>}`
      : this.optional
      ? ''
      : '{}';
  }
}

class ArrayProperty extends Property {
  type: 'array';
  items:
    | {|type: JsTypeString, recursive: false|}
    | {|$ref: string, recursive: ?boolean|};

  constructor(domain: string, obj: any) {
    super(domain, obj);
    this.type = obj.type;
    this.items = obj.items;
  }

  getRefDebuggerName(): ?string {
    if (this.items && this.items.$ref && !this.items.recursive) {
      const [domain, id] = toDomainAndId(this.domain, this.items.$ref);
      return `${domain}.${id}`;
    }
  }

  getFullCppType(): string {
    let elemType: string = 'JSONValue *';
    let recursive: ?(false | boolean) = false;

    if (this.items) {
      if (this.items.type) {
        elemType = jsTypeToCppType(this.items.type);
      } else if (this.items.$ref) {
        elemType = toFullCppType(this.domain, this.items.$ref);
        recursive = this.items.recursive;
      }
    }

    return maybeWrapOptional(
      `std::vector<${elemType}>`,
      this.optional,
      recursive,
    );
  }

  getInitializer(): string {
    return '';
  }
}
