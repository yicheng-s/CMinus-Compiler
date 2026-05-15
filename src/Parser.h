#ifndef PARSER_H
#define PARSER_H

#include "ASTNode.h"
#include "Lexer.h"
#include "SLRTableGenerator.h"
#include <vector>
#include <string>

using namespace std;

class Parser {
private:
    SLRTableGenerator& slrGen;
    vector<Token>& tokens;
    size_t tokenIdx;

    vector<int> stateStack;
    vector<ASTNode*> semStack;

    const vector<map<Symbol, ActionEntry>>& actionTable;
    const vector<map<Symbol, int>>& gotoTable;
    const vector<Production>& productions;

    // 将 Token 转换为语义节点 (仅 IDN/INT/FLOAT 产生实际节点)
    static ASTNode* tokenToSemNode(const Token& token);

    // 执行单个产生式的规约动作，返回归约后新建的 AST 节点
    ASTNode* doReduce(int prodId, vector<ASTNode*>& poppedSem);

    // 打印 AST (用于调试验证)
    static void printIndent(int depth);
    static string escapeStr(const string& s);

public:
    Parser(vector<Token>& tokens, SLRTableGenerator& slrGen);

    // 主解析接口，返回 AST 根节点
    ASTNode* parse();

    // 打印整棵 AST
    static void printAST(ASTNode* node, int depth = 0);
};

#endif // PARSER_H
