#ifndef AST_H
#define AST_H

#include <string>
#include <vector>
#include <iostream>

using namespace std;

// 前向声明所有 AST 节点类，供给 Visitor 使用
class ProgramNode;
class VarDeclNode;
class BinaryExpNode;
class NumberNode;
class IdentNode;
class IfStmtNode;
class ReturnStmtNode;

// ==========================================
// 核心接口：Visitor 模式访问者基类
// (D 同学将在 Day 4 继承这个类生成 LLVM IR)
// ==========================================
class Visitor {
public:
    virtual ~Visitor() = default;
    virtual void visit(ProgramNode* node) = 0;
    virtual void visit(VarDeclNode* node) = 0;
    virtual void visit(BinaryExpNode* node) = 0;
    virtual void visit(NumberNode* node) = 0;
    virtual void visit(IdentNode* node) = 0;
    virtual void visit(IfStmtNode* node) = 0;
    virtual void visit(ReturnStmtNode* node) = 0;
};

// ==========================================
// AST 节点基类
// ==========================================
class ASTNode {
public:
    virtual ~ASTNode() = default;
    // 强制所有子类实现 accept 方法
    virtual void accept(Visitor* v) = 0;
};

// ---------------- 具体节点定义示例 ----------------

class ProgramNode : public ASTNode {
public:
    vector<ASTNode*> compUnits;
    void accept(Visitor* v) override { v->visit(this); }
};

class BinaryExpNode : public ASTNode {
public:
    string op; // 操作符 "+", "-", "*", "/"
    ASTNode* lhs;
    ASTNode* rhs;
    BinaryExpNode(string o, ASTNode* l, ASTNode* r) : op(o), lhs(l), rhs(r) {}
    void accept(Visitor* v) override { v->visit(this); }
};

class NumberNode : public ASTNode {
public:
    int int_val;
    bool is_float;
    float float_val;
    void accept(Visitor* v) override { v->visit(this); }
};

// ... 其他节点 C 同学在建树时可依据文法在此处扩充
#endif // AST_H