## Day 3 完成情况 — SLR(1) 解析引擎与 AST 构建

### 交付状态：✅ 全部完成

### 核心文件

| 文件 | 说明 |
|------|------|
| `src/Parser.h` | Parser 类声明，含状态栈、语义栈、parse/doReduce/printAST 接口 |
| `src/Parser.cpp` | 解析引擎实现：移进/规约主循环 + 全部 83 条产生式的 AST 构建逻辑 |

### 对 D 同学的接口

`Parser::parse()` 返回 **`ASTNode*`**，实际类型为 **`ProgramNode*`**。顶层结构：

```
ProgramNode
  └─ compUnits: vector<ASTNode*>   // 包含 VarDeclNode / ConstDeclNode / FuncDefNode
```

D 同学只需实现 Visitor 子类（接口已定义在 `ASTNode.h`），对每种 AST 节点生成对应的 LLVM IR，然后调用：

```cpp
ASTNode* astRoot = parser.parse();
Visitor* visitor = new YourLLVMVisitor();
astRoot->accept(visitor);
```

`printAST()` 可随时调用来查看任意节点的完整树结构。

### 支持的语法特性

- 变量声明（int / float，支持初始化）
- 常量声明（const int / const float）
- 函数定义（多参数、int / float / void 返回类型）
- 语句：赋值、表达式语句、if / if-else、return、block
- 表达式：算术（+ - * / %）、关系（< > <= >=）、相等（== !=）、逻辑（&& || !）
- 函数调用

### 编译与验证

```bash
# 编译
g++ src/Lexer.cpp src/SLRTableGenerator.cpp src/Parser.cpp src/main.cpp -o compiler.exe -std=c++17

# 验证（输出 "Parse succeeded!" + 完整 AST 即为通过）
./compiler.exe tests/test_day3.sy
./compiler.exe tests/test_c_comprehensive.sy
```
