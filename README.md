# JS_Engine - JavaScript Interpreter in C++

A custom-built, lightweight JavaScript interpreter written in C++17. Developed as a solution for the hackathon project, this engine parses JavaScript source code into an AST and executes it in a custom runtime environment, fully supporting advanced features like closures, exceptions, and prototype methods.

---

## Features Supported

- **Variables & Scoping**: `let` and `const` declarations, proper block scoping, and dynamic re-assignment.
- **Data Types**: Numbers (float/integer), Strings, Booleans, `null`, and `undefined`.
- **Operators**: 
  - Arithmetic: `+` (with string concatenation coercion), `-`, `*`, `/`, `%`, `**` (exponentiation).
  - Update: Prefix/Postfix `++` and `--`.
  - Compound Assignment: `+=` and `-=`.
  - Comparison & Equality: `==`, `===`, `!=`, `!==`, `<`, `<=`, `>`, `>=`.
  - Logical: `&&` and `||` (with short-circuit evaluation), `!`.
- **Control Flow**: `if/else` (including `else if` chains), `while` loops, `do-while` loops, `for` loops, and `switch/case/default` statements with break fallthrough.
- **Functions & Closures**: First-class function declarations, function expressions, single/multi-parameter arrow functions, callbacks, recursion, and lexical scoping (closures).
- **Spread Operator**: Evaluates `...` spread syntax in both array literals and function call arguments.
- **Exception Handling**: Fully supports `try-catch-finally` statements and `throw` expressions.
- **Built-in Standard Library**:
  - **Arrays**: `push`, `pop`, `shift`, `unshift`, `slice`, `splice`, `concat`, `includes`, `indexOf`, `sort`, `reverse`, `join`, `map`, `filter`, `reduce`, `find`, `some`, `every`, `forEach`, and `length`.
  - **Strings**: `split`, `replace`, `replaceAll`, `substring`, `slice`, `trim`, `toUpperCase`, `toLowerCase`, `includes`, `startsWith`, `endsWith`, `indexOf`, `charAt`, and `length`.
  - **Global Functions**: `Number()`, `String()`, `Boolean()`.
  - **Globals**: `console.log`, `Math` (`floor`, `ceil`, `round`, `max`, `min`, `pow`, `sqrt`, `abs`, `random`), `Date` (`Date()`, `Date.now()`).

---

## How to Build & Run

### Prerequisites
Make sure you have a C++ compiler supporting **C++17** (like `g++` or `clang++`).

### 1. Compile the Project
Open your terminal/command line in the project root directory and run:

```bash
g++ -std=c++17 -o js_engine src/main.cpp src/lexer/Lexer.cpp src/parser/Parser.cpp src/runtime/Environment.cpp src/interpreter/Interpreter.cpp
```
On Windows, this will generate `js_engine.exe`. On macOS/Linux, it generates `js_engine`.

### 2. Run a JavaScript File
Execute any JavaScript file by passing it as an argument to the compiler binary:

```bash
./js_engine path/to/script.js
```

### 3. Run a JavaScript Snippet directly
You can execute inline JavaScript using the `-e` flag:

```bash
./js_engine -e "console.log(10 + 20 * 2);"
```

### 4. Run the Test Suite
The JS_Engine binary has a **natively built-in test runner**. To run all 105 automated test cases:

```bash
./js_engine tests/
```

---

## Hackathon Required Test Cases

This engine passes all 5 required test cases out-of-the-box:

1. **Odd / Even Checker**:
   ```js
   let num = 7; 
   if (num % 2 === 0) { 
       console.log(num + " is Even"); 
   } else { 
       console.log(num + " is Odd"); 
   }
   ```
   *Output:* `7 is Odd`

2. **Triangle Pattern**:
   ```js
   for (let i = 1; i <= 5; i++) { 
       let row = ""; 
       for (let j = 1; j <= i; j++) { 
           row += "*"; 
       } 
       console.log(row); 
   }
   ```
   *Output:*
   ```txt
   *
   **
   ***
   ****
   *****
   ```

3. **Armstrong Number**:
   Checks 153 and 123 using nested loops and exponentiation.
   *Output:* `true` / `false`

4. **Array Reverse**:
   Reverses an array using the spread operator, `.reverse()`, and `.join()`.
   *Output:*
   ```txt
   Original: 1, 2, 3, 4, 5
   Reversed: 5, 4, 3, 2, 1
   ```

5. **String Palindrome Check**:
   Reverses a string using `.split()`, `.reverse()`, and `.join()`.
   *Output:* `racecar is a Palindrome`
