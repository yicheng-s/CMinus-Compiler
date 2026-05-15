#ifndef AST_H
#define AST_H

#include <string>
#include <vector>
#include <iostream>

using namespace std;

// 前向声明
class ProgramNode; class VarDeclNode; class ConstDeclNode;
class VarDefNode; class ConstDefNode; class FuncDefNode;
class BlockNode; class BinaryExpNode; class NumberNode;
class IdentNode; class IfStmtNode; class ReturnStmtNode;
class AssignStmtNode; class UnaryExpNode; class FuncCallNode;
class FuncFParamNode; class ExpStmtNode; class ListNode;
class EmptyNode; class BTypeNode; class FuncTypeNode; class OpNode;

// ==========================================
// Visitor 接口 (D 同学 Day 4 实现 LLVM IR)
// ==========================================
class Visitor {
public:
    virtual ~Visitor() = default;
    virtual void visit(ProgramNode* node) = 0;
    virtual void visit(VarDeclNode* node) = 0;
    virtual void visit(ConstDeclNode* node) = 0;
    virtual void visit(VarDefNode* node) = 0;
    virtual void visit(ConstDefNode* node) = 0;
    virtual void visit(FuncDefNode* node) = 0;
    virtual void visit(BlockNode* node) = 0;
    virtual void visit(BinaryExpNode* node) = 0;
    virtual void visit(NumberNode* node) = 0;
    virtual void visit(IdentNode* node) = 0;
    virtual void visit(IfStmtNode* node) = 0;
    virtual void visit(ReturnStmtNode* node) = 0;
    virtual void visit(AssignStmtNode* node) = 0;
    virtual void visit(UnaryExpNode* node) = 0;
    virtual void visit(FuncCallNode* node) = 0;
    virtual void visit(FuncFParamNode* node) = 0;
    virtual void visit(ExpStmtNode* node) = 0;
    virtual void visit(BTypeNode* node) = 0;
    virtual void visit(FuncTypeNode* node) = 0;
    virtual void visit(ListNode* node) = 0;
    virtual void visit(EmptyNode* node) = 0;
    virtual void visit(OpNode* node) = 0;
};

// ==========================================
// AST 节点基类
// ==========================================
class ASTNode {
public:
    virtual ~ASTNode() = default;
    virtual void accept(Visitor* v) = 0;
};

// ==========================================
// 表达式节点
// ==========================================

class NumberNode : public ASTNode {
public:
    int int_val;
    bool is_float;
    float float_val;
    NumberNode(int v) : int_val(v), is_float(false), float_val(0) {}
    NumberNode(float v) : int_val(0), is_float(true), float_val(v) {}
    void accept(Visitor* v) override { v->visit(this); }
};

class IdentNode : public ASTNode {
public:
    string name;
    IdentNode(const string& n) : name(n) {}
    void accept(Visitor* v) override { v->visit(this); }
};

class BinaryExpNode : public ASTNode {
public:
    string op;
    ASTNode* lhs;
    ASTNode* rhs;
    BinaryExpNode(const string& o, ASTNode* l, ASTNode* r) : op(o), lhs(l), rhs(r) {}
    void accept(Visitor* v) override { v->visit(this); }
};

class UnaryExpNode : public ASTNode {
public:
    string op;
    ASTNode* operand;
    UnaryExpNode(const string& o, ASTNode* e) : op(o), operand(e) {}
    void accept(Visitor* v) override { v->visit(this); }
};

class FuncCallNode : public ASTNode {
public:
    string funcName;
    vector<ASTNode*> args;
    FuncCallNode(const string& name) : funcName(name) {}
    void accept(Visitor* v) override { v->visit(this); }
};

// ==========================================
// 语句节点
// ==========================================

class BlockNode : public ASTNode {
public:
    vector<ASTNode*> items;
    void accept(Visitor* v) override { v->visit(this); }
};

class IfStmtNode : public ASTNode {
public:
    ASTNode* condition;
    ASTNode* thenStmt;
    ASTNode* elseStmt; // nullptr 表示无 else 分支
    IfStmtNode(ASTNode* c, ASTNode* t, ASTNode* e = nullptr)
        : condition(c), thenStmt(t), elseStmt(e) {}
    void accept(Visitor* v) override { v->visit(this); }
};

class ReturnStmtNode : public ASTNode {
public:
    ASTNode* retValue; // nullptr 表示无返回值
    ReturnStmtNode(ASTNode* v = nullptr) : retValue(v) {}
    void accept(Visitor* v) override { v->visit(this); }
};

class AssignStmtNode : public ASTNode {
public:
    ASTNode* lVal;
    ASTNode* rVal;
    AssignStmtNode(ASTNode* l, ASTNode* r) : lVal(l), rVal(r) {}
    void accept(Visitor* v) override { v->visit(this); }
};

class ExpStmtNode : public ASTNode {
public:
    ASTNode* expr; // nullptr 表示空表达式语句 ";"
    ExpStmtNode(ASTNode* e = nullptr) : expr(e) {}
    void accept(Visitor* v) override { v->visit(this); }
};

// ==========================================
// 声明节点
// ==========================================

class BTypeNode : public ASTNode {
public:
    string typeName; // "int" 或 "float"
    BTypeNode(const string& t) : typeName(t) {}
    void accept(Visitor* v) override { v->visit(this); }
};

class FuncTypeNode : public ASTNode {
public:
    string typeName; // "void", "int" 或 "float"
    FuncTypeNode(const string& t) : typeName(t) {}
    void accept(Visitor* v) override { v->visit(this); }
};

class VarDefNode : public ASTNode {
public:
    string varName;
    bool hasInit;
    ASTNode* initVal;
    VarDefNode(const string& name, bool init, ASTNode* val = nullptr)
        : varName(name), hasInit(init), initVal(val) {}
    void accept(Visitor* v) override { v->visit(this); }
};

class ConstDefNode : public ASTNode {
public:
    string constName;
    ASTNode* initVal;
    ConstDefNode(const string& name, ASTNode* val)
        : constName(name), initVal(val) {}
    void accept(Visitor* v) override { v->visit(this); }
};

class VarDeclNode : public ASTNode {
public:
    ASTNode* bType;
    vector<VarDefNode*> varDefs;
    void accept(Visitor* v) override { v->visit(this); }
};

class ConstDeclNode : public ASTNode {
public:
    ASTNode* bType;
    vector<ConstDefNode*> constDefs;
    void accept(Visitor* v) override { v->visit(this); }
};

class FuncFParamNode : public ASTNode {
public:
    string paramName;
    ASTNode* bType;
    FuncFParamNode(ASTNode* type, const string& name)
        : paramName(name), bType(type) {}
    void accept(Visitor* v) override { v->visit(this); }
};

class FuncDefNode : public ASTNode {
public:
    ASTNode* funcType;
    string funcName;
    vector<FuncFParamNode*> params;
    ASTNode* body;
    FuncDefNode(ASTNode* type, const string& name)
        : funcType(type), funcName(name), body(nullptr) {}
    void accept(Visitor* v) override { v->visit(this); }
};

// ==========================================
// 辅助节点
// ==========================================

class ProgramNode : public ASTNode {
public:
    vector<ASTNode*> compUnits;
    void accept(Visitor* v) override { v->visit(this); }
};

// 中间链表聚合节点，用于文法中 * 列表的非终结符
class ListNode : public ASTNode {
public:
    vector<ASTNode*> items;
    void prepend(ASTNode* item) {
        items.insert(items.begin(), item);
    }
    void accept(Visitor* v) override { v->visit(this); }
};

// ε 空产生式节点
class EmptyNode : public ASTNode {
public:
    void accept(Visitor* v) override { v->visit(this); }
};

// 运算符字符串传递节点（用于 unaryOp 等）
class OpNode : public ASTNode {
public:
    string op;
    OpNode(const string& o) : op(o) {}
    void accept(Visitor* v) override { v->visit(this); }
};

#endif // AST_H
