# Mini Compiler / Interpreter — C++17

Ek complete compiler pipeline jo ek custom programming language ko execute karta hai.

## Features
- Arithmetic: `+ - * / %`
- Strings with concatenation
- Variables: `var x = 10;`
- If / Else conditions
- While loops
- User-defined functions with recursion
- Boolean logic: `&&  ||  not`
- Built-ins: `sqrt()  abs()  pow()  str()`
- REPL (interactive mode)

## Build & Run

```bash
g++ -std=c++17 -O2 -o minicompiler main.cpp
./minicompiler          # Run demos
./minicompiler --repl   # Interactive REPL
./minicompiler file.ml  # Run a file
```

## Language Syntax

```
// Variables
var x = 42;
var name = "Piyush";

// If / Else
if (x > 10) { print("bada"); } else { print("chota"); }

// While
var i = 0;
while (i < 5) { print(i); i = i + 1; }

// Functions
fun factorial(n) {
    if (n <= 1) { return 1; }
    return n * factorial(n - 1);
}
print(factorial(10));
```

## Architecture

```
Source Code
    ↓
Lexer (lexer.h)        → Tokens
    ↓
Parser (parser.h)      → AST (Abstract Syntax Tree)
    ↓
Interpreter (interp.h) → Output
```

## Files
| File | Role |
|------|------|
| `lexer.h` | Source → Tokens |
| `parser.h` | Tokens → AST + AST Node definitions |
| `interpreter.h` | AST → Execute (Environment, Values, Functions) |
| `main.cpp` | Entry point, demos, REPL |
