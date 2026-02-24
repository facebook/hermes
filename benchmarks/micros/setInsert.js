let log = typeof print === "undefined"
    ? console.log
    : print;

const N = 10_000_000;

const setSolution = arr => {
  const tMinus1 = Date.now()
  const set = new Set(arr)
  const t0 = Date.now()
  log(`Time: ${t0 - tMinus1}`)
}
let linearValueArray = Array.from({ length: N }, (_, i) => i);
setSolution(linearValueArray);
