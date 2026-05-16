#ifndef LLVM_IR_GEN_H
#define LLVM_IR_GEN_H

#include "ASTNode.h"
#include <string>
#include <vector>
#include <map>
#include <sstream>

using namespace std;

class LLVMIRGen : public Visitor {
private:
    ostringstream ir;           // IR 输出缓冲
    int regCounter;             // 虚拟寄存器编号
    int labelCounter;           // 基本块标签编号
    string currentFuncName;     // 当前函数名
    bool returnsValue;          // 当前函数是否有非 void 返回值
    string currentRetType;      // 当前函数返回类型 "i32" / "float" / "void"
    
    // 符号表：变量名 → LLVM 类型字符串 ("i32" / "float")
    map<string, string> symbolTypes;
    // 常量映射：常量名 → 值
    map<string, string> constValues;

    string newReg();
    string newLabel(const string& prefix = "label");
    string llvmType(const string& cType);   // C-- type → LLVM type
    string exprType(ASTNode* node);
    string promoteTo(ASTNode* node, const string& targetType);
    string getSymbolReg(const string& name);
    void emitGlobalVar(const string& name, const string& llvmTy, const string& initVal, bool isConst);
    void emitFunctionDecl(const string& name, const string& retTy, const vector<pair<string,string>>& params);

    // 生成表达式 IR，返回虚拟寄存器名
    string genExpr(ASTNode* node);

public:
    LLVMIRGen();
    string getIR() const { return ir.str(); }

    void visit(ProgramNode* node) override;
    void visit(VarDeclNode* node) override;
    void visit(ConstDeclNode* node) override;
    void visit(VarDefNode* node) override;
    void visit(ConstDefNode* node) override;
    void visit(FuncDefNode* node) override;
    void visit(BlockNode* node) override;
    void visit(BinaryExpNode* node) override;
    void visit(NumberNode* node) override;
    void visit(IdentNode* node) override;
    void visit(IfStmtNode* node) override;
    void visit(ReturnStmtNode* node) override;
    void visit(AssignStmtNode* node) override;
    void visit(UnaryExpNode* node) override;
    void visit(FuncCallNode* node) override;
    void visit(FuncFParamNode* node) override;
    void visit(ExpStmtNode* node) override;
    void visit(BTypeNode* node) override;
    void visit(FuncTypeNode* node) override;
    void visit(ListNode* node) override;
    void visit(EmptyNode* node) override;
    void visit(OpNode* node) override;

    // 保存 LLVM IR 到文件
    void saveToFile(const string& path);
};

#endif
