# 🚀 CMinus-Compiler (C-- to LLVM IR)

![C++](https://img.shields.io/badge/Language-C++11/14/17-blue.svg)
![License](https://img.shields.io/badge/License-MIT-green.svg)
![Build](https://img.shields.io/badge/Build-Passing-brightgreen.svg)
![Course](https://img.shields.io/badge/Course-Compiler_Principles_2026-orange.svg)

[**中文版 (Chinese Version)**](README.md)

This project is the final assignment for the Spring 2026 course *Compiler Principles and Techniques* at Tianjin University.

We have built a compiler front-end and Intermediate Representation (IR) generator from C-- (a subset of C language) to LLVM IR **from scratch using C++, entirely by hand**. **No automated code generation tools (such as Lex, Yacc, Flex, Bison, or ANTLR) were used**, hardcore replicating the underlying mechanisms of an industrial-grade compiler.

## ✨ Core Features

*   🔍 **Hand-written Lexical Analyzer (Lexer)**:
    *   Simulates a Deterministic Finite Automaton (DFA) using a two-pointer approach.
    *   Supports **case-insensitive keywords** and case-sensitive identifiers via precise Hash Table mapping.
    *   Strictly follows the **Maximal Munch principle**, features single/multi-line comment filtering, and **Panic-mode error isolation with exact line number tracking**.
*   🧠 **SLR(1) Syntax Parsing**:
    *   Dynamic execution algorithms to automatically calculate **FIRST and FOLLOW sets** for non-terminals.
    *   Automatically constructs the LR(0) canonical collection of item sets, resolving Shift/Reduce conflicts.
    *   Performs shift and reduce operations based on 2D parsing tables alongside state and semantic stacks.
*   🌳 **Abstract Syntax Tree (AST) & Visitor Pattern**:
    *   Defines AST nodes using a strict object-oriented inheritance hierarchy.
    *   Applies the **Visitor Design Pattern** to perfectly decouple the "data structure" of the AST from its "traversal semantics" (IR generation/visualization).
*   ⚙️ **LLVM IR Generator**:
    *   Automatically generates human-readable LLVM IR text (`.ll` format) via pre-order AST traversal.
    *   Built-in single-pass local/global symbol tables to isolate variable scopes and allocate SSA (Static Single Assignment) virtual registers.
*   🎨 **Bonus: AST Visualization Rendering**:
    *   Implemented an extra `GraphvizVisitor` to export the AST to a `.dot` topology file with one click, supporting high-fidelity syntax tree image rendering.

## 🏗️ Architecture

The pipeline of this compiler is completely decoupled into four independent modules:

```text
[Source Code .sy] 
       ⬇️
(1) Lexical Analysis
       ⬇️  <Token Stream>
(2) SLR Table Generation
       ⬇️  <SLR Action & Goto Tables>
(3) SLR Parsing & AST Building
       ⬇️  <Abstract Syntax Tree>
(4) IR Generation & Visualization
       ⬇️
[LLVM IR Code .ll] & [AST Graph .dot]
```

## 📂 Directory Structure

```
CMinus-Compiler/
├── src/
│   ├── Lexer.h / Lexer.cpp                 # Lexical Analyzer
│   ├── SLRTableGenerator.h / .cpp          # FIRST/FOLLOW Sets & SLR Table Generator
│   ├── Parser.h / Parser.cpp               # SLR Parsing Engine & AST Assembly
│   ├── ASTNode.h                           # AST Nodes Definition & Visitor Interface
│   ├── IRGenerator.h / IRGenerator.cpp     # LLVM IR Generator
│   ├── GraphvizVisitor.h                   # [Bonus] AST Visualization Generator
│   └── main.cpp                            # Compiler Main Pipeline
├── tests/
│   ├── test1.sy ... test_final.sy          # Official & Custom Advanced Test Cases
│   └── *.ref                               # Standard Output Reference Files
├── docs/                                   # Lab Reports & Presentation Slides
└── README_EN.md                            # Project Documentation
```

## 🛠️ Build & Run

### 1. Build the Compiler

Please ensure GCC/Clang (C++11 or later) is installed on your system. Execute the following command in your terminal:

```
# In Windows environments, using CMD instead of PowerShell is highly recommended to prevent I/O stream blocking (Fake Freeze).
g++ src/main.cpp src/Lexer.cpp src/SLRTableGenerator.cpp src/Parser.cpp src/IRGenerator.cpp -o mycompiler.exe
```

### 2. Parse Source Code

Pass the path of the .sy source file to the compiler:

```
.\mycompiler.exe tests/test_final.sy
```

**Upon successful execution, the console will print the parsing progress, and generate the following files in the current directory:**

1. output.ll: The generated LLVM IR code.
2. ast_graph.dot: The graphical topology file of the AST.

### 3. [Bonus] Render the AST Image

Requires the installation of [Graphviz](https://www.google.com/url?sa=E&q=https%3A%2F%2Fgraphviz.org%2Fdownload%2F). Execute the following command in the terminal:

```
dot -Tpng ast_graph.dot -o ast.png
```

*(Open ast.png to view an exquisite and highly-detailed Abstract Syntax Tree!)*

### 4. Run the Target LLVM IR Code

Requires the [LLVM](https://www.google.com/url?sa=E&q=https%3A%2F%2Fllvm.org%2F) toolchain. Use lli to execute the generated intermediate code directly and print the program's return value:

```
lli output.ll
# To check the return value in Windows CMD: echo %errorlevel%
# To check the return value in Linux/Mac: echo $?
```

## 👨‍💻 Team & Roles

This project was collaboratively completed by a team of 4 members from Tianjin University:

- **👑 Sun Yicheng (Team Lead)**:
  - Responsible for project architecture, interface design, and ASTNode base class definition.
  - Developed the hand-written DFA Lexer.
  - **Bonus Task**: Developed GraphvizVisitor for AST visualization. Led the full-pipeline integration and system-level debugging (resolved Windows I/O stream blocking mechanism).
- **🧑‍💻 Zhang Zefan**:
  - Responsible for SLRTableGenerator development.
  - Conquered the FIRST/FOLLOW sets automated deduction algorithm, implemented LR(0) state machine closures, and generated the SLR(1) parsing tables.
- **🧑‍💻 Guo Haoting**:
  - Responsible for Parser execution engine.
  - Drove the state and semantic stacks, implementing the assembly of AST nodes and the bottom-up cascading of the reduction tree.
- **🧑‍💻 Li Shaohan**:
  - Responsible for IRGenerator middle-end integration.
  - Implemented the Visitor pattern, managed symbol table scoping and SSA virtual register allocation, ultimately generating compliant LLVM IR instructions.

## 📝 Pitfalls & Academic References

The major challenges we encountered during development and our solutions (e.g., PowerShell redirection swallowing stderr causing output freezing, dealing with Dangling-Else conflicts in SLR reductions, etc.) are thoroughly documented in docs/Development_and_Testing_Report.pdf.

**Primary References:**

1. Aho, A. V., Lam, M. S., Sethi, R., & Ullman, J. D. (2006). *Compilers: Principles, Techniques, and Tools (2nd Ed.)*.
2. Lattner, C., & Adve, V. (2004). *LLVM: A compilation framework for lifelong program analysis & transformation*.

------

*"We are not merely calling tools; we are crafting the rules ourselves."*