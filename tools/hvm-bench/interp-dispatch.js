"use strict";

var logger = typeof print === "undefined"
    ? console.log
    : print;

function bench (lc, fc) {
    var n, fact;
    var res = 0;
    while (--lc >= 0) {
        n = fc;
        fact = n;
        while (--n > 1)
            fact *= n;
        res += fact;
    }
    return res;
}

logger(bench(4e6, 100))
