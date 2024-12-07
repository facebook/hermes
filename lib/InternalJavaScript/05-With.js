(function() {
    function _containsField(obj, field) {
        // We use __containsField to handle multiple scenarios:
        // 1. checking for properties in primitive types (e.g. _containsField(1, 'toString') should return true)
        // 2. checking for properties in objects and functions, including undefined properties (e.g. _containsField({a:undefined}, 'a') should return true)
        if (obj instanceof Object) {
            if (obj.hasOwnProperty(Symbol.unscopables)) {
                const unscopables = obj[Symbol.unscopables];
                if (unscopables && unscopables[field]) {
                    return false;
                }
            }
            return obj[field] !== undefined || obj.hasOwnProperty(field);
        } else {
            return obj[field] !== undefined;
        }
    }

    var HermesWithInternal = {
        _containsField
    };
    Object.freeze(HermesWithInternal);
    globalThis.HermesWithInternal = HermesWithInternal;
})();
