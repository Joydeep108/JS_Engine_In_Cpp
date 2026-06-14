try {
    try {
        throw "inner";
    } catch(e) {
        throw e + " outer";
    }
} catch(e) {
    console.log(e);
}