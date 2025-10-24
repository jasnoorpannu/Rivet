# Rivet — A Tiny Programming Language in C++

Rivet is a small interpreted programming language built entirely in modern C++17, designed as a learning project to explore how compilers and interpreters work from the ground up.

It includes:
- A Lexer (tokenizer)
- A Recursive Descent Parser
- An Abstract Syntax Tree (AST)
- An Interpreter that executes AST nodes directly

Rivet is inspired by the simplicity of Python and the strictness of C++ — easy to write, but with robust typing and structure.

## Features

- Variables (`let` and `var` for immutability/mutability)
- Numbers, Booleans, Strings, and Arrays
- Arithmetic and logical expressions (`+ - * / && ||`)
- If / Else conditionals
- While loops
- C-style For loops (`for (var i = 0; i < 10; i = i + 1)`)
- For-in loops for arrays and strings (`for x in arr { ... }`)
- Functions and return values
- Print statement
- Nested scopes and lexical environments

## Project Structure

```
Rivet/
├── include/
│   └── rivet/
│       └── token.hpp
├── src/
│   ├── lexer.cpp
│   ├── lexer.hpp
│   ├── parser.cpp
│   ├── parser.hpp
│   ├── eval.cpp
│   ├── eval.hpp
│   └── main.cpp
├── CMakeLists.txt
└── test.rvt
```

## Build Instructions

### Requirements
- CMake ≥ 3.15
- C++17 compiler (tested on g++ 13+ / clang++)
- Ninja (optional, for faster builds)

### Build & Run

```bash
# Configure
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug

# Compile
cmake --build build

# Run interactive REPL
./build/rvt

# Run a .rvt script
./build/rvt run test.rvt
```

## Example Program

`test.rvt`

```rivet
let x = 5;
var y = 3;

print x + y;

let greet = "Hello";
let name = "Rivet";
print greet + ", " + name;

if (x > 2 && y < 5) {
  print "Condition works!";
}

var count = 0;
while (count < 3) {
  print "While count = " + count;
  count = count + 1;
}

for (var i = 0; i < 5; i = i + 1) {
  print "for i = " + i;
}

let nums = [10, 20, 30];
for n in nums {
  print "n = " + n;
}

fn add(a, b) {
  return a + b;
}

print "add(7, 8) = " + add(7, 8);
```

**Output:**
```
8
Hello, Rivet
Condition works!
While count = 0
While count = 1
While count = 2
for i = 0
for i = 1
for i = 2
for i = 3
for i = 4
n = 10
n = 20
n = 30
add(7, 8) = 15
```

## How Rivet Works

1. Lexer breaks the input text into tokens (`if`, `+`, `(`, `123`, etc.)  
2. Parser consumes tokens and builds an AST representing expressions and statements.  
3. Interpreter walks the AST and executes code node by node.  
4. Environment tracks variables, scopes, and functions.

## What I Learned

- Building a compiler pipeline (lexer → parser → interpreter)
- Working with recursive descent parsing and AST design
- Managing scope and variables in nested environments
- Implementing type checking and runtime errors
- Using modern C++17 features: std::variant, std::optional, lambdas, and smart pointers
- Debugging memory issues with AddressSanitizer

## Future Plans

- Add a static type checker  
- Implement file I/O and standard library  
- Compile to bytecode or LLVM IR  
- Create a VS Code syntax highlighter  
- Add importable modules and functions

## Author

Jasnoor Pannu  
Built on Fedora Linux using C++17 + CMake

Every 'rvt>' prompt feels like a small step toward building my own world.

## License
MIT License — free to use, modify, and learn from.
