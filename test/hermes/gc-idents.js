// RUN: %hermes -target=HBC -O -gc-sanitize-handles=0 %s

function manyIdents(obj) {
    for(var i = 0; i < 10000; ++i) {
        delete obj["p" + i];
    }
}

manyIdents({});
print("Success!");
