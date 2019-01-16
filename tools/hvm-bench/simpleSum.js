function doSum() {
    var sum = 0;
    for (var i = 0; i < 100000; i++) {
        sum += i;
    }
    return sum;
}

function doSumNTimes(n) {
    var sum = 0;
    for (var i = 0; i < n; i++) {
        sum += doSum()
    }
    return sum;
}

print(doSumNTimes(10000));
