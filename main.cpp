#include "interpreter.h"
#include <iostream>
#include <fstream>
#include <string>

void runCode(const std::string& source, bool showSource = false) {
    try {
        if (showSource) {
            std::cout << "--- Source ---\n" << source << "\n--- Output ---\n";
        }
        Lexer       lexer(source);
        auto        tokens = lexer.tokenize();
        Parser      parser(std::move(tokens));
        auto        ast    = parser.parse();
        Interpreter interp;
        interp.run(ast);
    } catch (const std::exception& e) {
        std::cerr << "[Error] " << e.what() << "\n";
    }
}

void startREPL() {
    std::cout << "====================================\n";
    std::cout << "   Mini Compiler REPL v1.0\n";
    std::cout << "   Type 'exit' to quit\n";
    std::cout << "====================================\n\n";

    Interpreter interp;
    std::string line;
    std::string accumulated;

    while (true) {
        std::cout << (accumulated.empty() ? ">>> " : "... ");
        if (!std::getline(std::cin, line)) break;
        if (line == "exit" || line == "quit") break;

        accumulated += line + "\n";

        int braces = 0;
        for (char c : accumulated) {
            if (c == '{') braces++;
            if (c == '}') braces--;
        }

        if (braces <= 0) {
            try {
                Lexer  lexer(accumulated);
                auto   tokens = lexer.tokenize();
                Parser parser(std::move(tokens));
                auto   ast    = parser.parse();
                interp.run(ast);
            } catch (const std::exception& e) {
                std::cerr << "[Error] " << e.what() << "\n";
            }
            accumulated.clear();
        }
    }
    std::cout << "\nBye!\n";
}

void runDemos() {
    std::cout << "\n========================================\n";
    std::cout << "        MINI COMPILER DEMO\n";
    std::cout << "========================================\n\n";

    std::cout << "[1] Arithmetic Operations:\n";
    runCode("var a = 10; var b = 3; print(a + b); print(a - b); print(a * b); print(a / b); print(a % b);");

    std::cout << "\n[2] String Concatenation:\n";
    runCode(R"(var name = "Piyush"; print("Hello, " + name + "!"); print("Sum = " + str(5 + 3));)");

    std::cout << "\n[3] If / Else:\n";
    runCode("var x = 15; if (x > 10) { print(\"x bada hai 10 se\"); } else { print(\"x chota hai\"); }");

    std::cout << "\n[4] While Loop (1 to 5):\n";
    runCode("var i = 1; while (i <= 5) { print(i); i = i + 1; }");

    std::cout << "\n[5] Functions:\n";
    runCode(R"(fun greet(name) { print("Namaste, " + name + "!"); } greet("Piyush"); greet("World");)");

    std::cout << "\n[6] Recursive Factorial:\n";
    runCode("fun factorial(n) { if (n <= 1) { return 1; } return n * factorial(n - 1); } print(factorial(5)); print(factorial(10));");

    std::cout << "\n[7] Fibonacci Series (first 8 terms):\n";
    runCode("fun fib(n) { if (n <= 1) { return n; } return fib(n-1) + fib(n-2); } var i = 0; while (i < 8) { print(fib(i)); i = i + 1; }");

    std::cout << "\n[8] Built-in Math:\n";
    runCode("print(sqrt(144)); print(abs(-42)); print(pow(2, 10));");

    std::cout << "\n[9] Boolean Logic:\n";
    runCode("var a = true; var b = false; if (a && not b) { print(\"Logic works!\"); } print(a == true); print(b != true);");

    std::cout << "\n[10] FizzBuzz (1-15):\n";
    runCode(R"(
        var i = 1;
        while (i <= 15) {
            if (i % 15 == 0) { print("FizzBuzz"); }
            else { if (i % 3 == 0) { print("Fizz"); }
                   else { if (i % 5 == 0) { print("Buzz"); }
                          else { print(i); } } }
            i = i + 1;
        }
    )");

    std::cout << "\n========================================\n";
    std::cout << "        ALL DEMOS COMPLETE!\n";
    std::cout << "========================================\n";
}

int main(int argc, char* argv[]) {
    if (argc == 1) {
        runDemos();
    } else if (argc == 2) {
        std::string arg = argv[1];
        if (arg == "--demo") {
            runDemos();
        } else if (arg == "--repl") {
            startREPL();
        } else {
            std::ifstream file(arg);
            if (!file) { std::cerr << "Cannot open: " << arg << "\n"; return 1; }
            std::string source((std::istreambuf_iterator<char>(file)),
                                std::istreambuf_iterator<char>());
            runCode(source, true);
        }
    }
    return 0;
}
