# CMinus Compiler (C-- 编译器)

本仓库为《编译原理与技术》期末大作业代码库。项目目标为实现一个 C-- 语言的编译器前端与中端，最终将 C-- 源代码编译为 LLVM IR 中间代码。

## 👥 团队成员与分工 (极限5日冲刺计划)
*   **【组长】A同学**：项目基建、手工词法分析器 (Lexer)、AST 节点体系定义、Visitor 接口定义 (Day 1)；最终项目整合与可视化拓展 (Day 5)。
*   **【组员】B同学**：SLR(1) 核心算法，自动计算 First/Follow 集，构造 LR(0) 状态机与 SLR 分析表 (Day 2)。
*   **【组员】C同学**：SLR(1) 解析引擎驱动，实现移进/规约逻辑，并利用状态栈拼接 AST 抽象语法树 (Day 3)。
*   **【组员】D同学**：对接官方中端 API，实现 Visitor 模式遍历 AST，完成 LLVM IR 中间代码生成 (Day 4)。

## 🚀 编译与运行 (Day 1 进度)
目前已完成纯手工 C++ 词法分析器的编写，脱离了 Flex 工具，支持精确定位行号与列号。

**编译指令 (基于 g++)：**
```bash
g++ src/Lexer.cpp src/main.cpp -o compiler.exe
./compiler.exe tests/test_day1.sy

## ✅ D 同学工作说明（LLVM IR 中间代码生成）

本部分实现 Visitor 模式遍历 AST，直接输出 LLVM IR 文本（.ll），不依赖外部 LLVM 库。

### 交付内容
- 新增 IR 生成器实现：[src/IRGenerator.cpp](src/IRGenerator.cpp)
- 访问器声明与工具函数：[src/IRGenerator.h](src/IRGenerator.h)
- 在主流程中接入 IR 生成并写出文件：[src/main.cpp](src/main.cpp)

### 主要功能
- 全局/局部变量与常量声明（含初始化）
- 函数定义与参数处理（含返回类型）
- 语句：赋值、表达式语句、if / if-else、return、block
- 表达式：算术、关系、相等、逻辑、单目运算
- 基础类型自动转换（int ↔ float）
- 生成 LLVM IR 输出文件 `output.ll`

### 运行与输出
```bash
g++ src/Lexer.cpp src/SLRTableGenerator.cpp src/Parser.cpp src/IRGenerator.cpp src/main.cpp -o compiler.exe -std=c++17
./compiler.exe tests/test_day3.sy
./compiler.exe tests/test_c_comprehensive.sy
```
成功运行后会输出 AST，并在当前目录生成 `output.ll`。

### 说明与限制
- 逻辑与关系表达式以 `i32` 作为结果类型（0/1）。
- 常量初始化支持简单常量表达式求值。
- 目前函数返回类型与参数类型由 AST 推断，跨文件声明不处理。