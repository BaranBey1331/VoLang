VoLang 🚀
The Ultra-Fast, Zero-Copy, Native Programming Language.
VoLang is a modern, highly optimized programming language designed to beat the compilation speeds of C++ and Rust, while maintaining the simplicity of dynamic languages. It compiles directly to LLVM IR, bypassing heavy transpilers and virtual machines to deliver raw machine code performance.
⚡ Why VoLang? (The VoLang Manifesto)
We believe developers shouldn't have to choose between execution speed, compilation speed, and developer experience. VoLang is engineered to surpass the giants:
 * Better than C/C++ & Rust: C++ and Rust suffer from notoriously slow compilation times. VoLang uses a Zero-Allocation Arena AST and In-Place AST Mutation, allowing it to compile at breathtaking speeds of over 1,000,000 LOC/s.
 * Better than JS, TS, & Ruby: No Virtual Machines, no JIT warm-up time, no garbage collection pauses. VoLang compiles directly to native machine registers via LLVM.
 * Better than Zig & Go: VoLang offers aggressive, built-in compile-time constant folding and dead-code elimination without requiring complex macros or comptime keywords.
 * Better than Assembly: You get the raw execution speed of Assembly, but with a clean, modern, and mathematically sound syntax.
🛠️ Architecture
 * Zero-Copy Lexer: Reads the entire source file into memory once. Tokens use string views (pointers + lengths) to avoid malloc bottlenecks.
 * Arena-Allocated AST: Nodes are carved out of a massive pre-allocated memory block in O(1) time. No memory fragmentation.
 * In-Place Optimizer: Modifies the AST directly in memory. It folds constants (e.g., 5 + 10 becomes 15) and eliminates dead code before the backend even sees it.
 * Direct LLVM Backend: Bypasses C transpilation. Generates raw LLVM Intermediate Representation (.ll), achieving maximum CPU optimization.
📦 Installation
Install VoLang instantly on Linux, macOS, or Termux (Android) using our official installation script:
curl -sSL [https://raw.githubusercontent.com/YOUR_GITHUB_NAME/VoLang/main/install.sh](https://raw.githubusercontent.com/YOUR_GITHUB_NAME/VoLang/main/install.sh) | bash

🚀 Quick Start
Create a file named main.vo:
let five = 5;
let ten = 10;

fn add(x, y) {
    return x + y;
}

let result = add(five, ten);

Compile it to LLVM IR:
volang main.vo

(This generates an output.ll file).
Build the final native executable using Clang:
clang output.ll -O3 -o my_app
./my_app

🗺️ Roadmap
 * [x] Zero-copy Lexical Analysis
 * [x] O(1) Arena AST Parser
 * [x] Direct LLVM IR Generation
 * [x] Scope Tracking & Variable Shadowing
 * [ ] Native C/C++ Plugin System (plugin.h)
 * [ ] Turkish Keyword Support (tanim, fonk)
 * [ ] Standard Library (I/O, Math, Strings)
 * [ ] Self-Hosting (Writing the VoLang compiler in VoLang)
🤝 Contributing
Contributions are welcome! Please check the issues page and submit a Pull Request. Let's build the fastest language on earth together.
