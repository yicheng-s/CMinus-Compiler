#include "IRGenerator.h"
#include <iomanip>

using namespace std;

namespace {
string formatFloat(float v) {
    ostringstream ss;
    ss.setf(ios::scientific);
    ss << setprecision(6) << v;
    return ss.str();
}
}

void IRGeneratorVisitor::visit(ProgramNode* node) {
    emit("; ModuleID = 'sysy2022_compiler'");
    string src = sourceFilename.empty() ? "input.sy" : sourceFilename;
    emit("source_filename = \"" + src + "\"");

    emit("declare i32 @getint()");
    emit("declare i32 @getch()");
    emit("declare i32 @getarray(i32*)");
    emit("declare void @putint(i32)");
    emit("declare void @putch(i32)");
    emit("declare void @putarray(i32, i32*)");
    emit("declare void @starttime()");
    emit("declare void @stoptime()");

    // Collect function signatures for call type resolution.
    for (auto* unit : node->compUnits) {
        if (auto* fn = dynamic_cast<FuncDefNode*>(unit)) {
            auto* ft = dynamic_cast<FuncTypeNode*>(fn->funcType);
            string retType = ft ? ft->typeName : "int";
            funcRetTypes[fn->funcName] = retType;
            vector<string> ptypes;
            for (auto* p : fn->params) {
                auto* pt = dynamic_cast<BTypeNode*>(p->bType);
                ptypes.push_back(pt ? pt->typeName : "int");
            }
            funcParamTypes[fn->funcName] = ptypes;
        }
    }

    for (auto* unit : node->compUnits) {
        if (unit) unit->accept(this);
    }
}

void IRGeneratorVisitor::visit(VarDeclNode* node) {
    auto* bt = dynamic_cast<BTypeNode*>(node->bType);
    curDeclType = bt ? bt->typeName : "int";
    for (auto* def : node->varDefs) {
        if (def) def->accept(this);
    }
}

void IRGeneratorVisitor::visit(ConstDeclNode* node) {
    auto* bt = dynamic_cast<BTypeNode*>(node->bType);
    curDeclType = bt ? bt->typeName : "int";
    for (auto* def : node->constDefs) {
        if (def) def->accept(this);
    }
}

static bool evalConstExpr(ASTNode* node, double& outVal, bool& outIsFloat) {
    if (auto* num = dynamic_cast<NumberNode*>(node)) {
        if (num->is_float) {
            outIsFloat = true;
            outVal = num->float_val;
        } else {
            outIsFloat = false;
            outVal = num->int_val;
        }
        return true;
    }
    if (auto* un = dynamic_cast<UnaryExpNode*>(node)) {
        double v = 0.0;
        bool isF = false;
        if (!evalConstExpr(un->operand, v, isF)) return false;
        if (un->op == "+") {
            outVal = v;
            outIsFloat = isF;
            return true;
        }
        if (un->op == "-") {
            outVal = -v;
            outIsFloat = isF;
            return true;
        }
        if (un->op == "!") {
            outVal = (v == 0.0) ? 1.0 : 0.0;
            outIsFloat = false;
            return true;
        }
        return false;
    }
    if (auto* bin = dynamic_cast<BinaryExpNode*>(node)) {
        double l = 0.0, r = 0.0;
        bool lf = false, rf = false;
        if (!evalConstExpr(bin->lhs, l, lf)) return false;
        if (!evalConstExpr(bin->rhs, r, rf)) return false;
        bool useFloat = lf || rf;
        if (bin->op == "+") { outVal = l + r; outIsFloat = useFloat; return true; }
        if (bin->op == "-") { outVal = l - r; outIsFloat = useFloat; return true; }
        if (bin->op == "*") { outVal = l * r; outIsFloat = useFloat; return true; }
        if (bin->op == "/") { outVal = r == 0.0 ? 0.0 : (l / r); outIsFloat = useFloat; return true; }
        if (bin->op == "%") { outVal = (int)l % (int)r; outIsFloat = false; return true; }
        if (bin->op == "<")  { outVal = (l <  r) ? 1.0 : 0.0; outIsFloat = false; return true; }
        if (bin->op == ">")  { outVal = (l >  r) ? 1.0 : 0.0; outIsFloat = false; return true; }
        if (bin->op == "<=") { outVal = (l <= r) ? 1.0 : 0.0; outIsFloat = false; return true; }
        if (bin->op == ">=") { outVal = (l >= r) ? 1.0 : 0.0; outIsFloat = false; return true; }
        if (bin->op == "==") { outVal = (l == r) ? 1.0 : 0.0; outIsFloat = false; return true; }
        if (bin->op == "!=") { outVal = (l != r) ? 1.0 : 0.0; outIsFloat = false; return true; }
        if (bin->op == "&&") { outVal = (l != 0.0 && r != 0.0) ? 1.0 : 0.0; outIsFloat = false; return true; }
        if (bin->op == "||") { outVal = (l != 0.0 || r != 0.0) ? 1.0 : 0.0; outIsFloat = false; return true; }
    }
    return false;
}

static string constValueToIR(const string& typeName, double val, bool isFloat) {
    if (typeName == "float") {
        return formatFloat(static_cast<float>(val));
    }
    if (isFloat) {
        return to_string(static_cast<int>(val));
    }
    return to_string(static_cast<int>(val));
}

void IRGeneratorVisitor::visit(VarDefNode* node) {
    bool isGlobal = (curFuncName.empty());
    string llvmTy = llvmType(curDeclType);

    if (isGlobal) {
        string initVal = defaultValue(curDeclType);
        if (node->hasInit && node->initVal) {
            double v = 0.0;
            bool isF = false;
            if (evalConstExpr(node->initVal, v, isF)) {
                initVal = constValueToIR(curDeclType, v, isF);
            }
        }
        emit("@" + node->varName + " = global " + llvmTy + " " + initVal);
        addSym(node->varName, "@" + node->varName, curDeclType, false, true);
        return;
    }

    string addr = newReg();
    emit("  " + addr + " = alloca " + llvmTy);
    addSym(node->varName, addr, curDeclType, false, false);

    if (node->hasInit && node->initVal) {
        node->initVal->accept(this);
        string val = lastResultVal;
        string vty = lastResultType;
        if (vty != curDeclType) {
            if (curDeclType == "float" && vty == "int") {
                string castReg = newReg();
                emit("  " + castReg + " = sitofp i32 " + val + " to float");
                val = castReg;
            } else if (curDeclType == "int" && vty == "float") {
                string castReg = newReg();
                emit("  " + castReg + " = fptosi float " + val + " to i32");
                val = castReg;
            }
        }
        emit("  store " + llvmTy + " " + val + ", " + llvmTy + "* " + addr);
    } else {
        emit("  store " + llvmTy + " " + defaultValue(curDeclType) + ", " + llvmTy + "* " + addr);
    }
}

void IRGeneratorVisitor::visit(ConstDefNode* node) {
    bool isGlobal = (curFuncName.empty());
    string llvmTy = llvmType(curDeclType);

    if (isGlobal) {
        string initVal = defaultValue(curDeclType);
        double v = 0.0;
        bool isF = false;
        if (node->initVal && evalConstExpr(node->initVal, v, isF)) {
            initVal = constValueToIR(curDeclType, v, isF);
        }
        emit("@" + node->constName + " = constant " + llvmTy + " " + initVal);
        addSym(node->constName, "@" + node->constName, curDeclType, true, true);
        return;
    }

    string addr = newReg();
    emit("  " + addr + " = alloca " + llvmTy);
    addSym(node->constName, addr, curDeclType, true, false);

    node->initVal->accept(this);
    string val = lastResultVal;
    string vty = lastResultType;
    if (vty != curDeclType) {
        if (curDeclType == "float" && vty == "int") {
            string castReg = newReg();
            emit("  " + castReg + " = sitofp i32 " + val + " to float");
            val = castReg;
        } else if (curDeclType == "int" && vty == "float") {
            string castReg = newReg();
            emit("  " + castReg + " = fptosi float " + val + " to i32");
            val = castReg;
        }
    }
    emit("  store " + llvmTy + " " + val + ", " + llvmTy + "* " + addr);
}

void IRGeneratorVisitor::visit(FuncDefNode* node) {
    auto* ft = dynamic_cast<FuncTypeNode*>(node->funcType);
    curFuncRetType = ft ? ft->typeName : "int";
    curFuncName = node->funcName;

    string sig = "define " + llvmType(curFuncRetType) + " @" + curFuncName + "(";
    for (size_t i = 0; i < node->params.size(); ++i) {
        auto* pt = dynamic_cast<BTypeNode*>(node->params[i]->bType);
        string ptype = pt ? pt->typeName : "int";
        sig += llvmType(ptype) + " %" + node->params[i]->paramName;
        if (i + 1 < node->params.size()) sig += ", ";
    }
    sig += ") {";
    emit(sig);

    enterScope();
    startBlock(curFuncName + "_ENTRY");

    for (auto* p : node->params) {
        auto* pt = dynamic_cast<BTypeNode*>(p->bType);
        string ptype = pt ? pt->typeName : "int";
        string llvmTy = llvmType(ptype);
        string addr = newReg();
        emit("  " + addr + " = alloca " + llvmTy);
        emit("  store " + llvmTy + " %" + p->paramName + ", " + llvmTy + "* " + addr);
        addSym(p->paramName, addr, ptype, false, false);
    }

    if (node->body) node->body->accept(this);

    if (!needTerminator) {
        if (curFuncRetType == "void") {
            emit("  ret void");
        } else {
            emit("  ret " + llvmType(curFuncRetType) + " " + defaultValue(curFuncRetType));
        }
    }

    emit("}");
    exitScope();
    curFuncName.clear();
    curFuncRetType.clear();
    needTerminator = false;
}

void IRGeneratorVisitor::visit(BlockNode* node) {
    enterScope();
    for (auto* item : node->items) {
        if (item) item->accept(this);
    }
    exitScope();
}

static string emitToBool(IRGeneratorVisitor* v, const string& val, const string& typeName) {
    string reg = v->newReg();
    if (typeName == "float") {
        v->emit("  " + reg + " = fcmp one float " + val + ", 0.000000e+00");
    } else {
        v->emit("  " + reg + " = icmp ne i32 " + val + ", 0");
    }
    return reg;
}

void IRGeneratorVisitor::visit(BinaryExpNode* node) {
    node->lhs->accept(this);
    string lhsVal = lastResultVal;
    string lhsType = lastResultType;

    node->rhs->accept(this);
    string rhsVal = lastResultVal;
    string rhsType = lastResultType;

    string op = node->op;
    bool isFloat = (lhsType == "float" || rhsType == "float");

    if (op == "+" || op == "-" || op == "*" || op == "/" || op == "%") {
        if (isFloat) {
            if (lhsType == "int") {
                string castReg = newReg();
                emit("  " + castReg + " = sitofp i32 " + lhsVal + " to float");
                lhsVal = castReg;
            }
            if (rhsType == "int") {
                string castReg = newReg();
                emit("  " + castReg + " = sitofp i32 " + rhsVal + " to float");
                rhsVal = castReg;
            }
        }

        string res = newReg();
        if (!isFloat) {
            string irOp = (op == "+") ? "add" : (op == "-") ? "sub" : (op == "*") ? "mul" : (op == "/") ? "sdiv" : "srem";
            emit("  " + res + " = " + irOp + " i32 " + lhsVal + ", " + rhsVal);
            lastResultVal = res;
            lastResultType = "int";
        } else {
            string irOp = (op == "+") ? "fadd" : (op == "-") ? "fsub" : (op == "*") ? "fmul" : (op == "/") ? "fdiv" : "frem";
            emit("  " + res + " = " + irOp + " float " + lhsVal + ", " + rhsVal);
            lastResultVal = res;
            lastResultType = "float";
        }
        return;
    }

    if (op == "<" || op == ">" || op == "<=" || op == ">=" || op == "==" || op == "!=") {
        if (isFloat) {
            if (lhsType == "int") {
                string castReg = newReg();
                emit("  " + castReg + " = sitofp i32 " + lhsVal + " to float");
                lhsVal = castReg;
            }
            if (rhsType == "int") {
                string castReg = newReg();
                emit("  " + castReg + " = sitofp i32 " + rhsVal + " to float");
                rhsVal = castReg;
            }
        }

        string cmpReg = newReg();
        if (isFloat) {
            string irOp = (op == "<") ? "olt" : (op == ">") ? "ogt" : (op == "<=") ? "ole" : (op == ">=") ? "oge" : (op == "==") ? "oeq" : "one";
            emit("  " + cmpReg + " = fcmp " + irOp + " float " + lhsVal + ", " + rhsVal);
        } else {
            string irOp = (op == "<") ? "slt" : (op == ">") ? "sgt" : (op == "<=") ? "sle" : (op == ">=") ? "sge" : (op == "==") ? "eq" : "ne";
            emit("  " + cmpReg + " = icmp " + irOp + " i32 " + lhsVal + ", " + rhsVal);
        }

        string zextReg = newReg();
        emit("  " + zextReg + " = zext i1 " + cmpReg + " to i32");
        lastResultVal = zextReg;
        lastResultType = "int";
        return;
    }

    if (op == "&&" || op == "||") {
        string lhsBool = emitToBool(this, lhsVal, lhsType);
        string rhsBool = emitToBool(this, rhsVal, rhsType);
        string logicReg = newReg();
        string irOp = (op == "&&") ? "and" : "or";
        emit("  " + logicReg + " = " + irOp + " i1 " + lhsBool + ", " + rhsBool);
        string zextReg = newReg();
        emit("  " + zextReg + " = zext i1 " + logicReg + " to i32");
        lastResultVal = zextReg;
        lastResultType = "int";
        return;
    }
}

void IRGeneratorVisitor::visit(NumberNode* node) {
    if (node->is_float) {
        lastResultVal = formatFloat(node->float_val);
        lastResultType = "float";
    } else {
        lastResultVal = to_string(node->int_val);
        lastResultType = "int";
    }
}

void IRGeneratorVisitor::visit(IdentNode* node) {
    SymEntry* sym = lookupSym(node->name);
    if (!sym) {
        lastResultVal = "0";
        lastResultType = "int";
        return;
    }
    string llvmTy = llvmType(sym->typeName);
    string reg = newReg();
    emit("  " + reg + " = load " + llvmTy + ", " + llvmTy + "* " + sym->llvmName);
    lastResultVal = reg;
    lastResultType = sym->typeName;
}

void IRGeneratorVisitor::visit(IfStmtNode* node) {
    node->condition->accept(this);
    string condVal = emitToBool(this, lastResultVal, lastResultType);

    string thenLabel = newLabel();
    string elseLabel = node->elseStmt ? newLabel() : "";
    string endLabel = newLabel();

    if (node->elseStmt) {
        emit("  br i1 " + condVal + ", label %" + thenLabel + ", label %" + elseLabel);
    } else {
        emit("  br i1 " + condVal + ", label %" + thenLabel + ", label %" + endLabel);
    }
    needTerminator = false;

    startBlock(thenLabel);
    if (node->thenStmt) node->thenStmt->accept(this);
    if (!needTerminator) {
        emit("  br label %" + endLabel);
    }
    needTerminator = false;

    if (node->elseStmt) {
        startBlock(elseLabel);
        node->elseStmt->accept(this);
        if (!needTerminator) {
            emit("  br label %" + endLabel);
        }
        needTerminator = false;
    }

    startBlock(endLabel);
}

void IRGeneratorVisitor::visit(ReturnStmtNode* node) {
    if (!node->retValue || curFuncRetType == "void") {
        emit("  ret void");
        needTerminator = true;
        return;
    }

    node->retValue->accept(this);
    string val = lastResultVal;
    string vty = lastResultType;
    if (vty != curFuncRetType) {
        if (curFuncRetType == "float" && vty == "int") {
            string castReg = newReg();
            emit("  " + castReg + " = sitofp i32 " + val + " to float");
            val = castReg;
        } else if (curFuncRetType == "int" && vty == "float") {
            string castReg = newReg();
            emit("  " + castReg + " = fptosi float " + val + " to i32");
            val = castReg;
        }
    }

    emit("  ret " + llvmType(curFuncRetType) + " " + val);
    needTerminator = true;
}

void IRGeneratorVisitor::visit(AssignStmtNode* node) {
    auto* ident = dynamic_cast<IdentNode*>(node->lVal);
    if (!ident) return;
    SymEntry* sym = lookupSym(ident->name);
    if (!sym || sym->isConst) return;

    node->rVal->accept(this);
    string val = lastResultVal;
    string vty = lastResultType;
    if (vty != sym->typeName) {
        if (sym->typeName == "float" && vty == "int") {
            string castReg = newReg();
            emit("  " + castReg + " = sitofp i32 " + val + " to float");
            val = castReg;
        } else if (sym->typeName == "int" && vty == "float") {
            string castReg = newReg();
            emit("  " + castReg + " = fptosi float " + val + " to i32");
            val = castReg;
        }
    }

    string llvmTy = llvmType(sym->typeName);
    emit("  store " + llvmTy + " " + val + ", " + llvmTy + "* " + sym->llvmName);
}

void IRGeneratorVisitor::visit(UnaryExpNode* node) {
    node->operand->accept(this);
    string val = lastResultVal;
    string vty = lastResultType;

    if (node->op == "+") {
        lastResultVal = val;
        lastResultType = vty;
        return;
    }

    if (node->op == "-") {
        string res = newReg();
        if (vty == "float") {
            emit("  " + res + " = fsub float 0.000000e+00, " + val);
            lastResultVal = res;
            lastResultType = "float";
        } else {
            emit("  " + res + " = sub i32 0, " + val);
            lastResultVal = res;
            lastResultType = "int";
        }
        return;
    }

    if (node->op == "!") {
        string boolReg = emitToBool(this, val, vty);
        string notReg = newReg();
        emit("  " + notReg + " = xor i1 " + boolReg + ", true");
        string zextReg = newReg();
        emit("  " + zextReg + " = zext i1 " + notReg + " to i32");
        lastResultVal = zextReg;
        lastResultType = "int";
        return;
    }
}

void IRGeneratorVisitor::visit(FuncCallNode* node) {
    vector<string> argVals;
    vector<string> argTypes;
    vector<string> paramTypes;
    auto it = funcParamTypes.find(node->funcName);
    if (it != funcParamTypes.end()) paramTypes = it->second;

    for (auto* arg : node->args) {
        arg->accept(this);
        string val = lastResultVal;
        string vty = lastResultType;
        if (argVals.size() < paramTypes.size()) {
            string pty = paramTypes[argVals.size()];
            if (vty != pty) {
                if (pty == "float" && vty == "int") {
                    string castReg = newReg();
                    emit("  " + castReg + " = sitofp i32 " + val + " to float");
                    val = castReg;
                    vty = "float";
                } else if (pty == "int" && vty == "float") {
                    string castReg = newReg();
                    emit("  " + castReg + " = fptosi float " + val + " to i32");
                    val = castReg;
                    vty = "int";
                }
            }
        }
        argVals.push_back(val);
        argTypes.push_back(vty);
    }

    string retTy = "int";
    auto rit = funcRetTypes.find(node->funcName);
    if (rit != funcRetTypes.end()) retTy = rit->second;

    string callPrefix = "  ";
    if (retTy != "void") {
        string callRes = newReg();
        callPrefix += callRes + " = ";
        lastResultVal = callRes;
        lastResultType = retTy;
    } else {
        lastResultVal.clear();
        lastResultType = "void";
    }

    string sig = callPrefix + "call " + llvmType(retTy) + " @" + node->funcName + "(";
    for (size_t i = 0; i < argVals.size(); ++i) {
        sig += llvmType(argTypes[i]) + " " + argVals[i];
        if (i + 1 < argVals.size()) sig += ", ";
    }
    sig += ")";
    emit(sig);
}

void IRGeneratorVisitor::visit(FuncFParamNode* node) {
    (void)node;
}

void IRGeneratorVisitor::visit(ExpStmtNode* node) {
    if (node->expr) node->expr->accept(this);
}

void IRGeneratorVisitor::visit(BTypeNode* node) {
    (void)node;
}

void IRGeneratorVisitor::visit(FuncTypeNode* node) {
    (void)node;
}

void IRGeneratorVisitor::visit(ListNode* node) {
    for (auto* item : node->items) {
        if (item) item->accept(this);
    }
}

void IRGeneratorVisitor::visit(EmptyNode* node) {
    (void)node;
}

void IRGeneratorVisitor::visit(OpNode* node) {
    (void)node;
}
