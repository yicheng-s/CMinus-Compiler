#include "Parser.h"
#include <stdexcept>
#include <sstream>

using namespace std;

Parser::Parser(vector<Token>& tokens, SLRTableGenerator& slrGen, ostream* trace)
    : slrGen(slrGen), tokens(tokens), tokenIdx(0), traceOut(trace),
      actionTable(slrGen.getActionTable()),
      gotoTable(slrGen.getGotoTable()),
      productions(slrGen.getProductions())
{
}

ASTNode* Parser::tokenToSemNode(const Token& token) {
    switch (token.type) {
        case TokenType::IDN:
            return new IdentNode(token.value);
        case TokenType::INT:
            return new NumberNode(stoi(token.value));
        case TokenType::FLOAT:
            return new NumberNode(stof(token.value));
        case TokenType::KW:
            // "main" 在文法中被当作标识符 (IDN) 处理
            if (token.value == "main")
                return new IdentNode(token.value);
            return nullptr;
        default:
            return nullptr;
    }
}

ASTNode* Parser::doReduce(int prodId, vector<ASTNode*>& popped) {
    // popped[0] = 最右侧符号, popped[n-1] = 最左侧符号
    auto rhs = [&](int i) -> ASTNode* {
        return popped[popped.size() - 1 - i];
    };

    switch (prodId) {
        // ============================================================
        // 0: Program' → Program
        // ============================================================
        case 0:
            return rhs(0);

        // ============================================================
        // 1: Program → compUnit
        // ============================================================
        case 1:
            return rhs(0);

        // ============================================================
        // 2: compUnit → CompUnitItems EOF_TOKEN
        // ============================================================
        case 2: {
            auto* listNode = dynamic_cast<ListNode*>(rhs(0));
            auto* prog = new ProgramNode();
            if (listNode) prog->compUnits = listNode->items;
            return prog;
        }

        // ============================================================
        // 3: CompUnitItems → decl CompUnitItems
        // 4: CompUnitItems → funcDef CompUnitItems
        // ============================================================
        case 3:
        case 4: {
            auto* tailList = dynamic_cast<ListNode*>(rhs(1));
            if (tailList) tailList->prepend(rhs(0));
            return tailList ? tailList : new ListNode();
        }

        // ============================================================
        // 5: CompUnitItems → ε
        // ============================================================
        case 5:
            return new ListNode();

        // ============================================================
        // 6: decl → constDecl
        // 7: decl → varDecl
        // ============================================================
        case 6:
        case 7:
            return rhs(0);

        // ============================================================
        // 8: constDecl → const type constDef ConstDefTail ;
        // ============================================================
        case 8: {
            auto* type = dynamic_cast<BTypeNode*>(rhs(1));
            auto* constDef = dynamic_cast<ConstDefNode*>(rhs(2));
            auto* tailList = dynamic_cast<ListNode*>(rhs(3));
            auto* decl = new ConstDeclNode();
            decl->bType = type;
            if (constDef) decl->constDefs.push_back(constDef);
            if (tailList) {
                for (auto* item : tailList->items)
                    if (auto* cd = dynamic_cast<ConstDefNode*>(item))
                        decl->constDefs.push_back(cd);
            }
            return decl;
        }

        // ============================================================
        // 9: ConstDefTail → , constDef ConstDefTail
        // ============================================================
        case 9: {
            auto* tailList = dynamic_cast<ListNode*>(rhs(2));
            if (tailList) tailList->prepend(rhs(1));
            return tailList ? tailList : new ListNode();
        }

        // ============================================================
        // 10: ConstDefTail → ε
        // ============================================================
        case 10:
            return new ListNode();

        // ============================================================
        // 11: type → int
        // 12: type → float
        // 13: type → void
        // ============================================================
        case 11: return new BTypeNode("int");
        case 12: return new BTypeNode("float");
        case 13: return new BTypeNode("void");

        // ============================================================
        // 14: constDef → IDN = constInitVal
        // ============================================================
        case 14: {
            auto* ident = dynamic_cast<IdentNode*>(rhs(0));
            return new ConstDefNode(ident ? ident->name : "", rhs(2));
        }

        // ============================================================
        // 15: constInitVal → constExp
        // ============================================================
        case 15:
            return rhs(0);

        // ============================================================
        // 16: varDecl → type varDef VarDefTail ;
        // ============================================================
        case 16: {
            auto* type = dynamic_cast<BTypeNode*>(rhs(0));
            auto* varDef = dynamic_cast<VarDefNode*>(rhs(1));
            auto* tailList = dynamic_cast<ListNode*>(rhs(2));
            auto* decl = new VarDeclNode();
            decl->bType = type;
            if (varDef) decl->varDefs.push_back(varDef);
            if (tailList) {
                for (auto* item : tailList->items)
                    if (auto* vd = dynamic_cast<VarDefNode*>(item))
                        decl->varDefs.push_back(vd);
            }
            return decl;
        }

        // ============================================================
        // 17: VarDefTail → , varDef VarDefTail
        // ============================================================
        case 17: {
            auto* tailList = dynamic_cast<ListNode*>(rhs(2));
            if (tailList) tailList->prepend(rhs(1));
            return tailList ? tailList : new ListNode();
        }

        // ============================================================
        // 18: VarDefTail → ε
        // ============================================================
        case 18:
            return new ListNode();

        // ============================================================
        // 19: varDef → IDN
        // 20: varDef → IDN = initVal
        // ============================================================
        case 19: {
            auto* ident = dynamic_cast<IdentNode*>(rhs(0));
            return new VarDefNode(ident ? ident->name : "", false);
        }
        case 20: {
            auto* ident = dynamic_cast<IdentNode*>(rhs(0));
            return new VarDefNode(ident ? ident->name : "", true, rhs(2));
        }

        // ============================================================
        // 21: initVal → exp
        // ============================================================
        case 21:
            return rhs(0);

        // ============================================================
        // 22: funcDef → type IDN ( FuncFParamsOpt ) block
        // ============================================================
        case 22: {
            auto* funcType = rhs(0); // type (leftmost)
            auto* ident = dynamic_cast<IdentNode*>(rhs(1));
            auto* paramsLN = dynamic_cast<ListNode*>(rhs(3));
            auto* body = dynamic_cast<BlockNode*>(rhs(5));
            auto* func = new FuncDefNode(funcType, ident ? ident->name : "");
            if (paramsLN) {
                for (auto* item : paramsLN->items)
                    if (auto* fp = dynamic_cast<FuncFParamNode*>(item))
                        func->params.push_back(fp);
            }
            func->body = body;
            return func;
        }

        // ============================================================
        // 23: FuncFParamsOpt → funcFParams
        // 24: FuncFParamsOpt → ε
        // ============================================================
        case 23:
            return rhs(0);
        case 24:
            return new ListNode();

        // ============================================================
        // 25: funcFParams → funcFParam FuncFParamTail
        // ============================================================
        case 25: {
            auto* tailList = dynamic_cast<ListNode*>(rhs(1));
            if (tailList) tailList->prepend(rhs(0));
            return tailList ? tailList : new ListNode();
        }

        // ============================================================
        // 26: FuncFParamTail → , funcFParam FuncFParamTail
        // ============================================================
        case 26: {
            auto* tailList = dynamic_cast<ListNode*>(rhs(2));
            if (tailList) tailList->prepend(rhs(1));
            return tailList ? tailList : new ListNode();
        }

        // ============================================================
        // 27: FuncFParamTail → ε
        // ============================================================
        case 27:
            return new ListNode();

        // ============================================================
        // 28: funcFParam → type IDN
        // ============================================================
        case 28: {
            auto* type = dynamic_cast<BTypeNode*>(rhs(0));
            auto* ident = dynamic_cast<IdentNode*>(rhs(1));
            return new FuncFParamNode(type, ident ? ident->name : "");
        }

        // ============================================================
        // 29: block → { BlockItems }
        // ============================================================
        case 29: {
            auto* listNode = dynamic_cast<ListNode*>(rhs(1));
            auto* block = new BlockNode();
            if (listNode) block->items = listNode->items;
            return block;
        }

        // ============================================================
        // 30: BlockItems → blockItem BlockItems
        // ============================================================
        case 30: {
            auto* tailList = dynamic_cast<ListNode*>(rhs(1));
            if (tailList) tailList->prepend(rhs(0));
            return tailList ? tailList : new ListNode();
        }

        // ============================================================
        // 31: BlockItems → ε
        // ============================================================
        case 31:
            return new ListNode();

        // ============================================================
        // 32: blockItem → decl
        // 33: blockItem → stmt
        // ============================================================
        case 32:
        case 33:
            return rhs(0);

        // ============================================================
        // 34: stmt → lVal = exp ;
        // ============================================================
        case 34: {
            return new AssignStmtNode(rhs(0), rhs(2));
        }

        // ============================================================
        // 35: stmt → ExpOpt ;
        // ============================================================
        case 35:
            return new ExpStmtNode(rhs(0));

        // ============================================================
        // 36: stmt → block
        // ============================================================
        case 36:
            return rhs(0);

        // ============================================================
        // 37: stmt → if ( cond ) stmt ElseOpt
        // ============================================================
        case 37: {
            return new IfStmtNode(rhs(2), rhs(4), rhs(5));
        }

        // ============================================================
        // 38: stmt → return ExpOpt ;
        // ============================================================
        case 38:
            return new ReturnStmtNode(rhs(1));

        // ============================================================
        // 39: ExpOpt → exp
        // 40: ExpOpt → ε
        // ============================================================
        case 39:
            return rhs(0);
        case 40:
            return nullptr;

        // ============================================================
        // 41: ElseOpt → else stmt
        // 42: ElseOpt → ε
        // ============================================================
        case 41:
            return rhs(0);
        case 42:
            return nullptr;

        // ============================================================
        // 43: exp → addExp
        // ============================================================
        case 43:
            return rhs(0);

        // ============================================================
        // 44: cond → lOrExp
        // ============================================================
        case 44:
            return rhs(0);

        // ============================================================
        // 45: lVal → IDN
        // ============================================================
        case 45:
            return rhs(0);

        // ============================================================
        // 46: primaryExp → ( exp )
        // ============================================================
        case 46:
            return rhs(1);

        // ============================================================
        // 47: primaryExp → lVal
        // 48: primaryExp → number
        // ============================================================
        case 47:
        case 48:
            return rhs(0);

        // ============================================================
        // 49: number → INT
        // 50: number → FLOAT
        // ============================================================
        case 49:
        case 50:
            return rhs(0);

        // ============================================================
        // 51: unaryExp → primaryExp
        // ============================================================
        case 51:
            return rhs(0);

        // ============================================================
        // 52: unaryExp → IDN ( FuncRParamsOpt )
        // ============================================================
        case 52: {
            auto* ident = dynamic_cast<IdentNode*>(rhs(0));
            auto* argsList = dynamic_cast<ListNode*>(rhs(2));
            auto* call = new FuncCallNode(ident ? ident->name : "");
            if (argsList) call->args = argsList->items;
            return call;
        }

        // ============================================================
        // 53: unaryExp → unaryOp unaryExp
        // ============================================================
        case 53: {
            auto* opNode = dynamic_cast<OpNode*>(rhs(0));
            return new UnaryExpNode(opNode ? opNode->op : "", rhs(1));
        }

        // ============================================================
        // 54: FuncRParamsOpt → funcRParams
        // 55: FuncRParamsOpt → ε
        // ============================================================
        case 54:
            return rhs(0);
        case 55:
            return new ListNode();

        // ============================================================
        // 56: unaryOp → +
        // 57: unaryOp → -
        // 58: unaryOp → !
        // ============================================================
        case 56: return new OpNode("+");
        case 57: return new OpNode("-");
        case 58: return new OpNode("!");

        // ============================================================
        // 59: funcRParams → funcRParam FuncRParamTail
        // ============================================================
        case 59: {
            auto* tailList = dynamic_cast<ListNode*>(rhs(1));
            if (tailList) tailList->prepend(rhs(0));
            return tailList ? tailList : new ListNode();
        }

        // ============================================================
        // 60: FuncRParamTail → , funcRParam FuncRParamTail
        // ============================================================
        case 60: {
            auto* tailList = dynamic_cast<ListNode*>(rhs(2));
            if (tailList) tailList->prepend(rhs(1));
            return tailList ? tailList : new ListNode();
        }

        // ============================================================
        // 61: FuncRParamTail → ε
        // ============================================================
        case 61:
            return new ListNode();

        // ============================================================
        // 62: funcRParam → exp
        // ============================================================
        case 62:
            return rhs(0);

        // ============================================================
        // 63: mulExp → unaryExp
        // ============================================================
        case 63:
            return rhs(0);

        // ============================================================
        // 64: mulExp → mulExp * unaryExp
        // 65: mulExp → mulExp / unaryExp
        // 66: mulExp → mulExp % unaryExp
        // ============================================================
        case 64: return new BinaryExpNode("*", rhs(0), rhs(2));
        case 65: return new BinaryExpNode("/", rhs(0), rhs(2));
        case 66: return new BinaryExpNode("%", rhs(0), rhs(2));

        // ============================================================
        // 67: addExp → mulExp
        // ============================================================
        case 67:
            return rhs(0);

        // ============================================================
        // 68: addExp → addExp + mulExp
        // 69: addExp → addExp - mulExp
        // ============================================================
        case 68: return new BinaryExpNode("+", rhs(0), rhs(2));
        case 69: return new BinaryExpNode("-", rhs(0), rhs(2));

        // ============================================================
        // 70: relExp → addExp
        // ============================================================
        case 70:
            return rhs(0);

        // ============================================================
        // 71: relExp → relExp < addExp
        // 72: relExp → relExp > addExp
        // 73: relExp → relExp <= addExp
        // 74: relExp → relExp >= addExp
        // ============================================================
        case 71: return new BinaryExpNode("<",  rhs(0), rhs(2));
        case 72: return new BinaryExpNode(">",  rhs(0), rhs(2));
        case 73: return new BinaryExpNode("<=", rhs(0), rhs(2));
        case 74: return new BinaryExpNode(">=", rhs(0), rhs(2));

        // ============================================================
        // 75: eqExp → relExp
        // ============================================================
        case 75:
            return rhs(0);

        // ============================================================
        // 76: eqExp → eqExp == relExp
        // 77: eqExp → eqExp != relExp
        // ============================================================
        case 76: return new BinaryExpNode("==", rhs(0), rhs(2));
        case 77: return new BinaryExpNode("!=", rhs(0), rhs(2));

        // ============================================================
        // 78: lAndExp → eqExp
        // ============================================================
        case 78:
            return rhs(0);

        // ============================================================
        // 79: lAndExp → lAndExp && eqExp
        // ============================================================
        case 79: return new BinaryExpNode("&&", rhs(0), rhs(2));

        // ============================================================
        // 80: lOrExp → lAndExp
        // ============================================================
        case 80:
            return rhs(0);

        // ============================================================
        // 81: lOrExp → lOrExp || lAndExp
        // ============================================================
        case 81: return new BinaryExpNode("||", rhs(0), rhs(2));

        // ============================================================
        // 82: constExp → addExp
        // ============================================================
        case 82:
            return rhs(0);

        default:
            throw runtime_error("Unknown production id: " + to_string(prodId));
    }
}

ASTNode* Parser::parse() {
    stateStack.push_back(0);
    int stepNum = 0;
    int iter = 0;
    const int MAX_ITER = 50000;

    while (true) {
        if (++iter > MAX_ITER) {
            throw runtime_error("Parser exceeded max iterations - likely infinite loop");
        }
        int state = stateStack.back();
        // Use dummy EOF as lookahead when token stream exhausted
        Token dummyEOF{TokenType::END_OF_FILE, "EOF", -1};
        Token& currentToken = (tokenIdx < tokens.size()) ? tokens[tokenIdx] : dummyEOF;

        Symbol terminal = slrGen.mapTokenToTerminal(
            currentToken.getTypeString(), currentToken.value);

        auto& row = actionTable[state];
        auto it = row.find(terminal);

        if (it == row.end()) {
            stringstream ss;
            ss << "Syntax Error at line " << currentToken.line_num
               << ": unexpected token <" << currentToken.getTypeString()
               << ", " << currentToken.value << "> in state " << state;
            throw runtime_error(ss.str());
        }

        const ActionEntry& action = it->second;

        if (action.type == ActionEntry::SHIFT) {
            if (tokenIdx >= tokens.size()) {
                throw runtime_error(
                    "Unexpected SHIFT past end of token stream in state " + to_string(state));
            }
            stepNum++;
            if (traceOut) (*traceOut) << stepNum << "\t" << state << "#" << terminal.name << "\tmove" << endl;
            stateStack.push_back(action.value);
            semStack.push_back(tokenToSemNode(currentToken));
            tokenIdx++;
        } else if (action.type == ActionEntry::REDUCE) {
            int prodId = action.value;
            const Production& prod = productions[prodId];
            int bodyLen = (int)prod.body.size();

            stepNum++;
            if (traceOut) (*traceOut) << stepNum << "\t" << state << "#" << terminal.name << "\treduction" << endl;

            vector<ASTNode*> popped;
            for (int i = 0; i < bodyLen; i++) {
                stateStack.pop_back();
                popped.push_back(semStack.back());
                semStack.pop_back();
            }

            ASTNode* newNode = doReduce(prodId, popped);

            int topState = stateStack.back();
            Symbol head{SymbolType::NON_TERMINAL, prod.head};
            auto& gotoRow = gotoTable[topState];
            auto gotoIt = gotoRow.find(head);
            if (gotoIt == gotoRow.end()) {
                throw runtime_error(
                    "GOTO table miss: state " + to_string(topState) +
                    " on " + prod.head);
            }
            stateStack.push_back(gotoIt->second);
            semStack.push_back(newNode);
        } else if (action.type == ActionEntry::ACCEPT) {
            stepNum++;
            if (traceOut) (*traceOut) << stepNum << "\t" << state << "#" << terminal.name << "\taccept" << endl;
            return semStack.back();
        }
    }
}

// ==========================================
// AST 打印
// ==========================================

void Parser::printIndent(int depth) {
    for (int i = 0; i < depth; i++) cout << "  ";
}

void Parser::printAST(ASTNode* node, int depth) {
    if (!node) {
        printIndent(depth); cout << "(null)" << endl;
        return;
    }

    if (auto* n = dynamic_cast<ProgramNode*>(node)) {
        printIndent(depth);
        cout << "Program [" << n->compUnits.size() << " units]" << endl;
        for (auto* u : n->compUnits) printAST(u, depth + 1);
    }
    else if (auto* n = dynamic_cast<FuncDefNode*>(node)) {
        printIndent(depth);
        cout << "FuncDef: " << n->funcName
             << " (params: " << n->params.size() << ")" << endl;
        printIndent(depth + 1); cout << "ReturnType: "; printAST(n->funcType, 0);
        for (auto* p : n->params) printAST(p, depth + 1);
        printIndent(depth + 1); cout << "Body:" << endl;
        printAST(n->body, depth + 2);
    }
    else if (auto* n = dynamic_cast<FuncFParamNode*>(node)) {
        printIndent(depth);
        cout << "Param: " << n->paramName << " : "; printAST(n->bType, 0);
    }
    else if (auto* n = dynamic_cast<BTypeNode*>(node)) {
        printIndent(depth);
        cout << "Type<" << n->typeName << ">" << endl;
    }
    else if (auto* n = dynamic_cast<VarDeclNode*>(node)) {
        printIndent(depth); cout << "VarDecl:" << endl;
        printIndent(depth + 1); cout << "Type: "; printAST(n->bType, 0);
        for (auto* vd : n->varDefs) printAST(vd, depth + 1);
    }
    else if (auto* n = dynamic_cast<ConstDeclNode*>(node)) {
        printIndent(depth); cout << "ConstDecl:" << endl;
        printIndent(depth + 1); cout << "Type: "; printAST(n->bType, 0);
        for (auto* cd : n->constDefs) printAST(cd, depth + 1);
    }
    else if (auto* n = dynamic_cast<VarDefNode*>(node)) {
        printIndent(depth);
        cout << "VarDef: " << n->varName;
        if (n->hasInit) { cout << " =" << endl; printAST(n->initVal, depth + 1); }
        else cout << endl;
    }
    else if (auto* n = dynamic_cast<ConstDefNode*>(node)) {
        printIndent(depth);
        cout << "ConstDef: " << n->constName << " =" << endl;
        printAST(n->initVal, depth + 1);
    }
    else if (auto* n = dynamic_cast<BinaryExpNode*>(node)) {
        printIndent(depth); cout << "BinaryExp<" << n->op << ">" << endl;
        printAST(n->lhs, depth + 1);
        printAST(n->rhs, depth + 1);
    }
    else if (auto* n = dynamic_cast<UnaryExpNode*>(node)) {
        printIndent(depth); cout << "UnaryExp<" << n->op << ">" << endl;
        printAST(n->operand, depth + 1);
    }
    else if (auto* n = dynamic_cast<FuncCallNode*>(node)) {
        printIndent(depth);
        cout << "FuncCall: " << n->funcName << " (" << n->args.size() << " args)" << endl;
        for (auto* a : n->args) printAST(a, depth + 1);
    }
    else if (auto* n = dynamic_cast<NumberNode*>(node)) {
        printIndent(depth);
        if (n->is_float) cout << "Number<float>: " << n->float_val << endl;
        else cout << "Number<int>: " << n->int_val << endl;
    }
    else if (auto* n = dynamic_cast<IdentNode*>(node)) {
        printIndent(depth); cout << "Ident: " << n->name << endl;
    }
    else if (auto* n = dynamic_cast<IfStmtNode*>(node)) {
        printIndent(depth); cout << "IfStmt:" << endl;
        printIndent(depth + 1); cout << "Condition:" << endl;
        printAST(n->condition, depth + 2);
        printIndent(depth + 1); cout << "Then:" << endl;
        printAST(n->thenStmt, depth + 2);
        if (n->elseStmt) {
            printIndent(depth + 1); cout << "Else:" << endl;
            printAST(n->elseStmt, depth + 2);
        }
    }
    else if (auto* n = dynamic_cast<ReturnStmtNode*>(node)) {
        printIndent(depth); cout << "ReturnStmt:" << endl;
        if (n->retValue) printAST(n->retValue, depth + 1);
        else { printIndent(depth + 1); cout << "(void)" << endl; }
    }
    else if (auto* n = dynamic_cast<AssignStmtNode*>(node)) {
        printIndent(depth); cout << "AssignStmt:" << endl;
        printIndent(depth + 1); cout << "LVal:" << endl;
        printAST(n->lVal, depth + 2);
        printIndent(depth + 1); cout << "RVal:" << endl;
        printAST(n->rVal, depth + 2);
    }
    else if (auto* n = dynamic_cast<ExpStmtNode*>(node)) {
        printIndent(depth); cout << "ExpStmt:" << endl;
        if (n->expr) printAST(n->expr, depth + 1);
        else { printIndent(depth + 1); cout << "(empty)" << endl; }
    }
    else if (auto* n = dynamic_cast<BlockNode*>(node)) {
        printIndent(depth);
        cout << "Block [" << n->items.size() << " items]" << endl;
        for (auto* item : n->items) printAST(item, depth + 1);
    }
    else if (auto* n = dynamic_cast<OpNode*>(node)) {
        printIndent(depth); cout << "Op: " << n->op << endl;
    }
    else {
        printIndent(depth); cout << "(unknown AST node)" << endl;
    }
}
