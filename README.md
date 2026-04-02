# ⚡ Lumina Lang

> A lightweight scripting language built from scratch in C++17 —
> hand-written Lexer, recursive-descent Parser & tree-walk Interpreter.

---

## 🚀 Features

- Arithmetic operators: `+ - * / %`
- String concatenation
- Variables: `var x = 10;`
- If / Else conditions
- While loops
- User-defined functions with recursion
- Boolean logic: `&& || not`
- Built-in functions: `sqrt()` `abs()` `pow()` `str()`
- Interactive REPL mode

---

## 🔧 Build & Run
```bash
g++ -std=c++17 -O2 -o lumina main.cpp
./lumina            # Run all demos
./lumina --repl     # Interactive REPL mode
./lumina file.lm    # Run a source file
```

---

## 📖 Language Syntax
```js
// Variables
var x = 42;
var name = "Lumina";

// If / Else
if (x > 10) {
    print("Greater!");
} else {
    print("Smaller!");
}

// While loop
var i = 0;
while (i < 5) {
    print(i);
    i = i + 1;
}

// Functions & Recursion
fun factorial(n) {
    if (n <= 1) { return 1; }
    return n * factorial(n - 1);
}
print(factorial(10));   // 3628800

// Built-in math
print(sqrt(144));       // 12
print(pow(2, 10));      // 1024
```

---

## 🏗️ Architecture
```
Source Code
     ↓
 Lexer (lexer.h)          →  Characters into typed Tokens
     ↓
 Parser (parser.h)        →  Tokens into Abstract Syntax Tree (AST)
     ↓
 Interpreter (interp.h)   →  Tree-walk execution with scoped environments
     ↓
   Output
```

---

## 📁 Project Structure

| File | Role |
|------|------|
| `lexer.h` | Tokenizer — breaks source into tokens |
| `parser.h` | Recursive descent parser + AST node definitions |
| `interpreter.h` | Tree-walk interpreter, environments, values |
| `main.cpp` | Entry point, demo runner, REPL |

---

## 💡 Concepts Demonstrated

- Compiler pipeline (Lexer → Parser → Interpreter)
- Recursive descent parsing
- Abstract Syntax Tree (AST) construction
- Lexical scoping with linked environments
- First-class functions and recursion
- `std::shared_ptr`, `std::variant`, `std::unordered_map`

---

## 📌 Sample Output
```
factorial(10)  →  3628800
fibonacci(7)   →  13
sqrt(144)      →  12
pow(2, 10)     →  1024
FizzBuzz(15)   →  FizzBuzz
```

---
