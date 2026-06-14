let x = 1;
try {
    x = 2;
} finally {
    x = 3;
}
console.log(x);