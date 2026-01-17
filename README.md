# Compiler

**Work in Progress** — not ready for production use.

An educational compiler for a simple **C-like custom language**, targeting **x86-64 assembly (NASM syntax) on Linux**. This project is intended for learning purposes and aims to understand the core concepts of compiler design, including lexing, parsing, code generation.

---

## Language Syntax

This compiler supports a minimal subset of C-like syntax:
- Integer variables
- Scopes (blocks)
- Integer arithmetic (+, -, *, /)
- Equality (==, !=)
- Comparison (>, >=, <, <=)
- If/else
- While loops
- Comments

Example:
```
{
    int x;
    x = (1 + 2) * 3;
    
    // comments

    int a;
    int b;
    int counter;

    counter = 0;

    a = 50;
    while (a) {
        b = 5;
        while (b) {
            b = b - 1;
            counter = counter + 1;
        }
        a = a - 1;
    }
}
```

## Build

1. Clone the repository:
```sh
git clone https://github.com/paul-csc/Compiler.git Compiler
cd Compiler
```

2. Configure CMake:
```sh
cmake -S . -B build 
```

3. Compile the project:
```sh
cmake --build build
```

4. Run the compiler:
```sh
./build/Compiler
```

5. Assemble and run the generated assembly (example for main program):
```sh
./test/assemble.sh main
```
