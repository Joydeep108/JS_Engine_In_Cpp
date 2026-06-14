try {
    throw "err";
} catch(e) {
    console.log(e);
} finally {
    console.log("finally");
}