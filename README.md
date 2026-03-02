VoLang
VoLang is an experimental, natively compiled programming language focusing on fast compilation times and straightforward syntax. It compiles directly to LLVM IR, bypassing intermediate C transpilation or heavy virtual machines.
⚡ Technical Goals
VoLang is built with a few core architectural principles:
 * Zero-Allocation Lexing: The lexer maps directly to memory without duplicating strings, minimizing overhead.
 * Arena-Based AST: Abstract Syntax Tree nodes are allocated in a contiguous memory arena in O(1) time, reducing memory fragmentation.
 * In-Place Optimizations: Basic constant folding and dead-code elimination are performed by mutating the AST in memory before code generation.
 * Direct LLVM Backend: Generates native .ll files for Clang/LLVM to compile into optimized machine code.
📦 Installation
To try out the VoLang compiler on Linux, macOS, or Termux:
curl -sSL [https://raw.githubusercontent.com/YOUR_GITHUB_NAME/VoLang/main/install.sh](https://raw.githubusercontent.com/YOUR_GITHUB_NAME/VoLang/main/install.sh) | bash

🚀 Usage
You can compile one or multiple .vo files together.
volang stdlib/math.vo main.vo

This generates an output.ll file. To build the final executable:
clang output.ll -O3 -o my_app
./my_app
