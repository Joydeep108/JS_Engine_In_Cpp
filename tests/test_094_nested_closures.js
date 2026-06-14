function outer(a) {
    return function(b) {
        return function(c) {
            return a + b + c;
        };
    };
}
console.log(outer(1)(2)(3));