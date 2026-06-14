function apply(fn, x) {
    return fn(x);
}
function double(n) {
    return n * 2;
}
console.log(apply(double, 5));
console.log(apply(double, 10));
