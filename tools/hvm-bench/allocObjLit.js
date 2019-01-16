var x = null;

function doAlloc() {
    for (var i = 0; i < 100000; i++) {
        x = {a: 1, b: 'abc', c: null};
        x = {a: 1, b: 'abc', c: null};
        x = {a: 1, b: 'abc', c: null};
        x = {a: 1, b: 'abc', c: null};
        x = {a: 1, b: 'abc', c: null};
        x = {a: 1, b: 'abc', c: null};
        x = {a: 1, b: 'abc', c: null};
        x = {a: 1, b: 'abc', c: null};
        x = {a: 1, b: 'abc', c: null};
        x = {a: 1, b: 'abc', c: null};
    }
}

function allocNTimes(n) {
    for (var i = 0; i < n; i++) {
        doAlloc()
    }
}

print(allocNTimes(100));
