# 🚀 CMinus-Compiler (C-- to LLVM IR)

![C++](https://img.shields.io/badge/Language-C++11/14/17-blue.svg)
![License](https://img.shields.io/badge/License-MIT-green.svg)
![Build](https://img.shields.io/badge/Build-Passing-brightgreen.svg)
![Course](https://img.shields.io/badge/Course-Compiler_Principles_2026-orange.svg)

[**英文版 (English Version)**](README_EN.md)

本项目是天津大学 2026 年春季《编译原理与技术》课程的期末大作业。

我们使用 **C++ 从零开始、纯手工打造**了一个 C-- 语言到 LLVM IR 的编译器前端与中间代码生成器。**本项目全程未借助任何自动化生成工具（如 Lex/Yacc/Flex/Bison 或 ANTLR）**，硬核复现了工业级编译器的底层运作机制。

## ✨ 核心特性

*   🔍 **纯手工词法分析器 Lexer**：
    *   基于双指针模拟确定性有穷自动机。
    *   支持**关键字大小写不敏感**与标识符区分大小写的哈希表精准映射。
    *   严格遵循**最长匹配原则**，具备多行/单行注释清洗与**精确到行号的 Panic 错误隔离定位**。
*   🧠 **SLR(1) 语法分析**：
    *   动态执行算法自动计算非终结符的 **FIRST 集与 FOLLOW 集**。
    *   自动构造 LR(0) 项目集规范族，解决 Shift/Reduce 冲突。
    *   基于二维分析表与状态栈/语义栈，完成移进与规约操作。
*   🌳 **抽象语法树与 Visitor 模式** ：
    *   采用严格的面向对象继承体系定义 AST 节点。
    *   应用 **Visitor 设计模式**，将 AST 的“数据结构”与“遍历语义”实现完美解耦。
*   ⚙️ **LLVM IR 生成器**：
    *   通过前序遍历 AST，自动生成人类可读的 LLVM IR 文本（`.ll` 格式）。
    *   内置单遍局部/全局符号表，实现变量作用域隔离与 SSA 寄存器分配。
*   🎨 **加分拓展：AST 可视化渲染**：
    *   额外实现 `GraphvizVisitor`，一键导出 AST 的 `.dot` 拓扑文件，支持渲染为高精度语法树图像。

## 🏗️ 整体架构

本编译器的流水线完全解耦，分为四大独立模块：

```text
[Source Code .sy] 
       ⬇️
(1) Lexical Analysis (词法分析)
       ⬇️  <Token Stream>
(2) SLR Table Generation (分析表自动构建)
       ⬇️  <SLR Action & Goto Tables>
(3) SLR Parsing (语法分析 & AST 构建)
       ⬇️  <Abstract Syntax Tree>
(4) IR Generation & Visualization (语义分析与 LLVM IR 生成)
       ⬇️
[LLVM IR Code .ll] & [AST Graph .dot]
```

## 📂 目录结构

```
CMinus-Compiler/
├── src/
│   ├── Lexer.h / Lexer.cpp                 # 词法分析器
│   ├── SLRTableGenerator.h / .cpp          # FIRST/FOLLOW集与SLR表生成器
│   ├── Parser.h / Parser.cpp               # SLR解析引擎与AST组装
│   ├── ASTNode.h                           # AST节点定义与Visitor接口
│   ├── IRGenerator.h / IRGenerator.cpp     # LLVM IR 生成器
│   ├── GraphvizVisitor.h                   # 【拓展】AST可视化生成器
│   └── main.cpp                            # 编译器主入口流水线
├── tests/
│   ├── test1.sy ... test_final.sy          # 官方与自建的高级压测用例
│   └── *.ref                               # 标准输出参考比对文件
├── docs/                                   # 实验报告与答辩PPT
└── README.md                               # 项目说明文档
```

## 🛠️ 编译与运行指南 (Build & Run)

### 1. 编译编译器本体

请确保系统已安装 GCC/Clang (支持 C++11 及以上)。在终端中执行：

```
# 建议在 Windows 环境下使用 CMD 而非 PowerShell 以防止流编码假死
g++ src/main.cpp src/Lexer.cpp src/SLRTableGenerator.cpp src/Parser.cpp src/IRGenerator.cpp -o mycompiler.exe
```

### 2. 运行编译器解析源代码

向编译器传入待编译的 .sy 文件路径：

```
.\mycompiler.exe tests/test_final.sy
```

**成功运行后，控制台将输出解析进度，并在当前目录下生成：**

1. output.ll：生成的 LLVM IR 中间代码。
2. ast_graph.dot：AST 的图形化拓扑文件。

### 3. 【拓展】渲染 AST 语法树图片

需提前安装 [Graphviz](https://www.google.com/url?sa=E&q=https%3A%2F%2Fgraphviz.org%2Fdownload%2F) 工具，并在终端执行：

```
dot -Tpng ast_graph.dot -o ast.png
```

*(打开 ast.png 即可看到极其精美的抽象语法树解析图！)*

### 4. 运行目标 LLVM IR 代码

需安装 [LLVM](https://www.google.com/url?sa=E&q=https%3A%2F%2Fllvm.org%2F) 工具链，使用 lli 直接执行生成的中间代码，并打印程序的返回值：

```
lli output.ll
# Windows CMD 查看返回值: echo %errorlevel%
# Linux/Mac 查看返回值: echo $?
```

## 👨‍💻 团队分工

本项目由天津大学 4 名成员在“极限 5 日”内接力攻坚完成：

- **👑 孙翌程 (组长)**：
  - 负责项目架构与接口设计、ASTNode 基类定义。
  - 负责词法分析器 Lexer 纯手工 DFA 开发。
  - **拓展任务**：开发 GraphvizVisitor 实现 AST 可视化，并主导四大模块的全线集成与系统级排错（解决 Windows I/O 流阻塞机制）。
- **🧑‍💻 张泽凡**：
  - 负责 SLRTableGenerator 开发。
  - 攻克 FIRST/FOLLOW 集自动推导算法，实现 LR(0) 状态机闭包与 SLR(1) 预测分析表生成。
- **🧑‍💻 郭昊霆**：
  - 负责 Parser 解析引擎开发。
  - 驱动状态栈与语义栈，实现 AST 节点的组装与规约树的向上级联。
- **🧑‍💻 李劭涵**：
  - 负责 IRGenerator 中端对接开发。
  - 实现 Visitor 模式，管理符号表作用域与 SSA 虚拟寄存器分配，最终生成合规的 LLVM IR 指令。

## 📝 踩坑记录与学术参考

我们在开发过程中遇到的最大挑战及解决方案，已详细记录在开发与测试报告.pdf 中。

**主要参考文献：**

1. Aho, A. V., Lam, M. S., Sethi, R., & Ullman, J. D. (2006). *Compilers: Principles, Techniques, and Tools (2nd Ed.)*.
2. Lattner, C., & Adve, V. (2004). *LLVM: A compilation framework for lifelong program analysis & transformation*.

------

*“我们不仅是在调用工具，我们正在亲手创造规则。”*