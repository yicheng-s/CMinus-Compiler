#ifndef GRAPHVIZ_VISITOR_H
#define GRAPHVIZ_VISITOR_H

#include "ASTNode.h"
#include <fstream>
#include <string>
#include <map>

using namespace std;

class GraphvizVisitor : public Visitor {
private:
    ofstream dotFile;
    int nodeCounter = 0;
    int currentParentId = -1;

    int addNode(const string& label) {
        int id = ++nodeCounter;
        dotFile << "  node" << id << " [label=\"" << label << "\"];\n";
        if (currentParentId != -1) {
            dotFile << "  node" << currentParentId << " -> node" << id << ";\n";
        }
        return id;
    }

public:
    GraphvizVisitor(const string& filename) {
        dotFile.open(filename);
        dotFile << "digraph AST {\n";
        dotFile << "  node [shape=box, fontname=\"Consolas\", style=filled, fillcolor=lightblue];\n";
    }

    ~GraphvizVisitor() {
        dotFile << "}\n";
        dotFile.close();
    }

    void visit(ProgramNode* node) override {
        int id = addNode("Program");
        int saved = currentParentId; currentParentId = id;
        for (auto* u : node->compUnits) if (u) u->accept(this);
        currentParentId = saved;
    }
    void visit(FuncDefNode* node) override {
        int id = addNode("FuncDef: " + node->funcName);
        int saved = currentParentId; currentParentId = id;
        if (node->funcType) node->funcType->accept(this);
        for (auto* p : node->params) if (p) p->accept(this);
        if (node->body) node->body->accept(this);
        currentParentId = saved;
    }
    void visit(BlockNode* node) override {
        int id = addNode("Block");
        int saved = currentParentId; currentParentId = id;
        for (auto* item : node->items) if (item) item->accept(this);
        currentParentId = saved;
    }
    void visit(BinaryExpNode* node) override {
        int id = addNode("BinaryOp: " + node->op);
        int saved = currentParentId; currentParentId = id;
        if (node->lhs) node->lhs->accept(this);
        if (node->rhs) node->rhs->accept(this);
        currentParentId = saved;
    }
    void visit(NumberNode* node) override {
        if (node->is_float) addNode("Float: " + to_string(node->float_val));
        else addNode("Int: " + to_string(node->int_val));
    }
    void visit(IdentNode* node) override { addNode("Ident: " + node->name); }
    void visit(AssignStmtNode* node) override {
        int id = addNode("Assign");
        int saved = currentParentId; currentParentId = id;
        if (node->lVal) node->lVal->accept(this);
        if (node->rVal) node->rVal->accept(this);
        currentParentId = saved;
    }
    void visit(ReturnStmtNode* node) override {
        int id = addNode("Return");
        int saved = currentParentId; currentParentId = id;
        if (node->retValue) node->retValue->accept(this);
        currentParentId = saved;
    }
    void visit(BTypeNode* node) override { addNode("Type: " + node->typeName); }
    void visit(VarDeclNode* node) override {
        int id = addNode("VarDecl");
        int saved = currentParentId; currentParentId = id;
        if (node->bType) node->bType->accept(this);
        for (auto* v : node->varDefs) if(v) v->accept(this);
        currentParentId = saved;
    }
    void visit(VarDefNode* node) override {
        int id = addNode("VarDef: " + node->varName);
        int saved = currentParentId; currentParentId = id;
        if (node->hasInit && node->initVal) node->initVal->accept(this);
        currentParentId = saved;
    }
    
    void visit(ConstDeclNode* node) override {}
    void visit(ConstDefNode* node) override {}
    void visit(IfStmtNode* node) override {
        int id = addNode("IfStmt");
        int saved = currentParentId; currentParentId = id;
        if(node->condition) node->condition->accept(this);
        if(node->thenStmt) node->thenStmt->accept(this);
        if(node->elseStmt) node->elseStmt->accept(this);
        currentParentId = saved;
    }
    void visit(UnaryExpNode* node) override {}
    void visit(FuncCallNode* node) override {}
    void visit(FuncFParamNode* node) override {}
    void visit(ExpStmtNode* node) override {}
    void visit(FuncTypeNode* node) override {}
    void visit(ListNode* node) override {}
    void visit(EmptyNode* node) override {}
    void visit(OpNode* node) override {}
};
#endif