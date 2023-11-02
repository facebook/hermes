/* @nolint */ var initES6Class = function () {
  var objCreate = Object.create;
  var objDefineProperty = Object.defineProperty;
  var objectSetPrototypeOf = Object.setPrototypeOf;

  function defineClass(ctor, superClass) {
    ctor.prototype = objCreate(superClass && superClass.prototype, {
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

  function defineClassProperty(cls, methodName, getter, setter) {
    objDefineProperty(cls.prototype, methodName, {
      enumerable: false,
      get: getter,
      set: setter,
    });
  }

  function defineStaticClassProperty(cls, methodName, getter, setter) {
    objDefineProperty(cls, methodName, {
      enumerable: false,
      get: getter,
      set: setter,
    });
  }

  function defineClassMethod(cls, methodName, fn) {
    objDefineProperty(cls.prototype, methodName, {
      enumerable: false,
      value: fn,
    });
  }

  function defineStaticClassMethod(cls, methodName, fn) {
    objDefineProperty(cls, methodName, {
      enumerable: false,
      value: fn,
    });
  }

  var hermesES6InternalObj = {
    defineClass,
    defineClassProperty,
    defineStaticClassProperty,
    defineClassMethod,
    defineStaticClassMethod,
  };
  Object.freeze(hermesES6InternalObj);

  globalThis.HermesES6Internal = hermesES6InternalObj;
};

if (HermesInternal?.hasES6Class?.()) {
  initES6Class();
}
