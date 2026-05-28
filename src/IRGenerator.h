#ifndef IR_GENERATOR_H
#define IR_GENERATOR_H

#include "ASTNode.h"
#include <string>
#include <vector>
#include <map>
#include <sstream>

using namespace std;

/**
 * IRGeneratorVisitor — D 同学 Day 4 实现
 * 遍历 C 同学构建的 AST，直接生成 LLVM IR 文本 (.ll 格式)
 *
 * 采用 SSA 虚拟寄存器 + 作用域符号表管理，
 * 不依赖外部 LLVM 库，生成的 .ll 可直接用 lli / llc 运行。
 */
class IRGeneratorVisitor : public Visitor {
private:
    // ── 输出缓冲 ──
    ostringstream ir;

    // ── 符号表条目 ──
    struct SymEntry {
        string llvmName;   // LLVM 寄存器名或全局变量名
        string typeName;   // "int" / "float"
        bool   isConst;
        bool   isGlobal;
    };

    // ── 作用域符号表栈 ──
    vector<map<string, SymEntry>> scopeStack;

    // ── 函数签名表 ──
    map<string, string> funcRetTypes;
    map<string, vector<string>> funcParamTypes;

    // ── 寄存器/标号计数器 ──
    int regCounter   = 0;
    int labelCounter = 0;

    // ── 当前上下文 ──
    string curFuncRetType;           // 当前函数的返回类型
    string curFuncName;              // 当前函数名
    string curDeclType;              // 当前声明的基础类型
    bool   needTerminator = false;   // 当前 basic block 是否需要终结指令
    string lastBlockLabel;           // 当前 basic block 的标号（用于 br 合并）
    string sourceFilename;           // 源文件名（用于模块头）

    // ── 表达式求值结果 ──
    string lastResultVal;   // 上次 visit 表达式产生的 LLVM 值名
    string lastResultType;  // 上次 visit 表达式产生的类型 ("int"/"float"/"void")

public:
    IRGeneratorVisitor() { enterScope(); }  // 全局作用域

    // ── 工具函数 ──
    string newReg()        { return "%r" + to_string(++regCounter); }
    string newLabel()      { return "L" + to_string(++labelCounter); }
    string llvmType(const string& ct) {
        if (ct == "int")   return "i32";
        if (ct == "float") return "float";
        if (ct == "void")  return "void";
        return "i32";
    }
    string defaultValue(const string& ct) {
        if (ct == "int")   return "0";
        if (ct == "float") return "0.000000e+00";
        return "";
    }

    void emit(const string& s) { ir << s << "\n"; }

    // ── 符号表操作 ──
    void enterScope() { scopeStack.push_back({}); }
    void exitScope()  { scopeStack.pop_back(); }

    void addSym(const string& name, const string& llvmName,
                const string& typeName, bool isConst = false, bool isGlobal = false) {
        scopeStack.back()[name] = {llvmName, typeName, isConst, isGlobal};
    }

    SymEntry* lookupSym(const string& name) {
        for (int i = (int)scopeStack.size() - 1; i >= 0; --i) {
            auto it = scopeStack[i].find(name);
            if (it != scopeStack[i].end()) return &it->second;
        }
        return nullptr;
    }

    // ── 控制流辅助 ──
    void startBlock(const string& label) {
        if (needTerminator && !lastBlockLabel.empty()) {
            emit("  br label %" + label);
        }
        emit(label + ":");
        lastBlockLabel = label;
        needTerminator = false;
    }

    string getResult() const { return ir.str(); }
    void setSourceFilename(const string& name) { sourceFilename = name; }

    // ================================================================
    //  Visitor 实现（声明）
    // ================================================================
    void visit(ProgramNode* node)     override;
    void visit(VarDeclNode* node)     override;
    void visit(ConstDeclNode* node)   override;
    void visit(VarDefNode* node)      override;
    void visit(ConstDefNode* node)    override;
    void visit(FuncDefNode* node)     override;
    void visit(BlockNode* node)       override;
    void visit(BinaryExpNode* node)   override;
    void visit(NumberNode* node)      override;
    void visit(IdentNode* node)       override;
    void visit(IfStmtNode* node)      override;
    void visit(ReturnStmtNode* node)  override;
    void visit(AssignStmtNode* node)  override;
    void visit(UnaryExpNode* node)    override;
    void visit(FuncCallNode* node)    override;
    void visit(FuncFParamNode* node)  override;
    void visit(ExpStmtNode* node)     override;
    void visit(BTypeNode* node)       override;
    void visit(FuncTypeNode* node)    override;
    void visit(ListNode* node)        override;
    void visit(EmptyNode* node)       override;
    void visit(OpNode* node)          override;
};

#endif // IR_GENERATOR_H
