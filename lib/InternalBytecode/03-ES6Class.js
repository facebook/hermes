/* @nolint */ var initES6Class = function () {
  var objCreate = Object.create;
  var objDefineProperty = Object.defineProperty;
  var objectSetPrototypeOf = Object.setPrototypeOf;
  var objectGetOwnPropertyDescriptor = Object.getOwnPropertyDescriptor;

  function defineClass(ctor, superClass) {
    const resolvedSuperClass = superClass || Object;
    ctor.prototype = objCreate(resolvedSuperClass.prototype, {
      constructor: {
        value: ctor,
        writable: true,
        configurable: true,
      },
    });

    objDefineProperty(ctor, "prototype", {
      writable: false,
    });

    if (superClass) {
      objectSetPrototypeOf(ctor, superClass);
    }
  }

  function definePropertyGetterOrSetter(obj, name, getter, setter) {
    var existingDescriptor = objectGetOwnPropertyDescriptor(obj, name);
    if (!existingDescriptor) {
      existingDescriptor = {
        configurable: true,
        enumerable: false,
      }
    }

    if (getter) {
      existingDescriptor.get = getter;
    } else if (setter) {
      existingDescriptor.set = setter;
    }

    objDefineProperty(obj, name, existingDescriptor);
  }

  function defineClassPropertyGetter(cls, methodName, getter) {
    definePropertyGetterOrSetter(cls.prototype, methodName, getter, undefined);
  }

  function defineClassPropertySetter(cls, methodName, setter) {
    definePropertyGetterOrSetter(cls.prototype, methodName, undefined, setter);
  }

  function defineStaticClassPropertyGetter(cls, methodName, getter) {
    definePropertyGetterOrSetter(cls, methodName, getter, undefined);
  }

  function defineStaticClassPropertySetter(cls, methodName, setter) {
    definePropertyGetterOrSetter(cls, methodName, undefined, setter);
  }

  function doDefineMethod(obj, methodName, fn) {
    objDefineProperty(obj, methodName, {
      enumerable: false,
      configurable: true,
      writable: true,
      value: fn,
    });

    if (typeof methodName === 'string') {
      objDefineProperty(fn, 'name', {
        configurable: true,
        value: methodName,
      });
    }
  }

  function defineClassMethod(cls, methodName, fn) {
    doDefineMethod(cls.prototype, methodName, fn);
  }

  function defineStaticClassMethod(cls, methodName, fn) {
    doDefineMethod(cls, methodName, fn);
  }

  var hermesES6InternalObj = {
    defineClass,
    defineClassPropertyGetter,
    defineClassPropertySetter,
    defineStaticClassPropertyGetter,
    defineStaticClassPropertySetter,
    defineClassMethod,
    defineStaticClassMethod,
  };
  Object.freeze(hermesES6InternalObj);

  globalThis.HermesES6Internal = hermesES6InternalObj;
};

if (HermesInternal?.hasES6Class?.()) {
  initES6Class();
}
