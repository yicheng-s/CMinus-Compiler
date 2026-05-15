#include "SLRTableGenerator.h"

/*
 * SLRTableGenerator.cpp
 * SLR(1) 分析表生成器 — FIRST/FOLLOW集计算、LR(0)项集构造、SLR分析表生成
 */

SLRTableGenerator::SLRTableGenerator() {
    initGrammar();
}

static bool isTerminalSymbol(const string& s) {
    static const set<string> terminals = {
        "const", "int", "void", "return", "float", "if", "else",
        "!", "+", "-", "*", "/", "%", "=", ">", "<", "==", "<=", ">=", "!=", "&&", "||",
        "(", ")", "{", "}", ";", ",",
        "IDN", "INT", "FLOAT", "EOF_TOKEN"
    };
    return terminals.find(s) != terminals.end();
}

void SLRTableGenerator::addProduction(string head, initializer_list<string> bodySym) {
    Production prod;
    prod.head = head;
    nonTerminals.insert(head);
    for (const auto& s : bodySym) {
        if (isTerminalSymbol(s)) {
            Symbol sym{SymbolType::TERMINAL, s};
            prod.body.push_back(sym);
            terminals.insert(sym);
        } else {
            Symbol sym{SymbolType::NON_TERMINAL, s};
            prod.body.push_back(sym);
            nonTerminals.insert(s);
        }
    }
    productions.push_back(prod);
}

void SLRTableGenerator::initGrammar() {
    /*
     * C-- 文法硬编码（附录1的36条规则）
     * 将 EBNF 中的 *, ?, | 等扩展语法转换为标准 BNF：
     *   A -> (B)*     → A -> B A | ε
     *   A -> B?       → A -> B | ε
     *   A -> B | C    → A -> B 和 A -> C 两条产生式
     * 文法符号约定：IDN=标识符, INT=整数, FLOAT=浮点数, EOF_TOKEN=文件结束
     */

    addProduction("Program'", {"Program"});

    addProduction("Program", {"compUnit"});

    addProduction("compUnit", {"CompUnitItems", "EOF_TOKEN"});
    addProduction("CompUnitItems", {"decl", "CompUnitItems"});
    addProduction("CompUnitItems", {"funcDef", "CompUnitItems"});
    addProduction("CompUnitItems", {});

    addProduction("decl", {"constDecl"});
    addProduction("decl", {"varDecl"});

    addProduction("constDecl", {"const", "bType", "constDef", "ConstDefTail", ";"});
    addProduction("ConstDefTail", {",", "constDef", "ConstDefTail"});
    addProduction("ConstDefTail", {});

    addProduction("bType", {"int"});
    addProduction("bType", {"float"});

    addProduction("constDef", {"IDN", "=", "constInitVal"});

    addProduction("constInitVal", {"constExp"});

    addProduction("varDecl", {"bType", "varDef", "VarDefTail", ";"});
    addProduction("VarDefTail", {",", "varDef", "VarDefTail"});
    addProduction("VarDefTail", {});

    addProduction("varDef", {"IDN"});
    addProduction("varDef", {"IDN", "=", "initVal"});

    addProduction("initVal", {"exp"});

    addProduction("funcDef", {"funcType", "IDN", "(", "FuncFParamsOpt", ")", "block"});
    addProduction("FuncFParamsOpt", {"funcFParams"});
    addProduction("FuncFParamsOpt", {});

    addProduction("funcType", {"void"});
    addProduction("funcType", {"int"});

    addProduction("funcFParams", {"funcFParam", "FuncFParamTail"});
    addProduction("FuncFParamTail", {",", "funcFParam", "FuncFParamTail"});
    addProduction("FuncFParamTail", {});

    addProduction("funcFParam", {"bType", "IDN"});

    addProduction("block", {"{", "BlockItems", "}"});
    addProduction("BlockItems", {"blockItem", "BlockItems"});
    addProduction("BlockItems", {});

    addProduction("blockItem", {"decl"});
    addProduction("blockItem", {"stmt"});

    addProduction("stmt", {"lVal", "=", "exp", ";"});
    addProduction("stmt", {"ExpOpt", ";"});
    addProduction("stmt", {"block"});
    addProduction("stmt", {"if", "(", "cond", ")", "stmt", "ElseOpt"});
    addProduction("stmt", {"return", "ExpOpt", ";"});
    addProduction("ExpOpt", {"exp"});
    addProduction("ExpOpt", {});

    addProduction("ElseOpt", {"else", "stmt"});
    addProduction("ElseOpt", {});

    addProduction("exp", {"addExp"});

    addProduction("cond", {"lOrExp"});

    addProduction("lVal", {"IDN"});

    addProduction("primaryExp", {"(", "exp", ")"});
    addProduction("primaryExp", {"lVal"});
    addProduction("primaryExp", {"number"});

    addProduction("number", {"INT"});
    addProduction("number", {"FLOAT"});

    addProduction("unaryExp", {"primaryExp"});
    addProduction("unaryExp", {"IDN", "(", "FuncRParamsOpt", ")"});
    addProduction("unaryExp", {"unaryOp", "unaryExp"});
    addProduction("FuncRParamsOpt", {"funcRParams"});
    addProduction("FuncRParamsOpt", {});

    addProduction("unaryOp", {"+"});
    addProduction("unaryOp", {"-"});
    addProduction("unaryOp", {"!"});

    addProduction("funcRParams", {"funcRParam", "FuncRParamTail"});
    addProduction("FuncRParamTail", {",", "funcRParam", "FuncRParamTail"});
    addProduction("FuncRParamTail", {});

    addProduction("funcRParam", {"exp"});

    addProduction("mulExp", {"unaryExp"});
    addProduction("mulExp", {"mulExp", "*", "unaryExp"});
    addProduction("mulExp", {"mulExp", "/", "unaryExp"});
    addProduction("mulExp", {"mulExp", "%", "unaryExp"});

    addProduction("addExp", {"mulExp"});
    addProduction("addExp", {"addExp", "+", "mulExp"});
    addProduction("addExp", {"addExp", "-", "mulExp"});

    addProduction("relExp", {"addExp"});
    addProduction("relExp", {"relExp", "<", "addExp"});
    addProduction("relExp", {"relExp", ">", "addExp"});
    addProduction("relExp", {"relExp", "<=", "addExp"});
    addProduction("relExp", {"relExp", ">=", "addExp"});

    addProduction("eqExp", {"relExp"});
    addProduction("eqExp", {"eqExp", "==", "relExp"});
    addProduction("eqExp", {"eqExp", "!=", "relExp"});

    addProduction("lAndExp", {"eqExp"});
    addProduction("lAndExp", {"lAndExp", "&&", "eqExp"});

    addProduction("lOrExp", {"lAndExp"});
    addProduction("lOrExp", {"lOrExp", "||", "lAndExp"});

    addProduction("constExp", {"addExp"});

    startSymbol = "Program";

    for (const auto& p : productions) {
        for (const auto& s : p.body) {
            if (s.type == SymbolType::TERMINAL) {
                terminals.insert(s);
            }
        }
    }
}

void SLRTableGenerator::computeFirst() {
    /*
     * 不动点迭代计算所有非终结符的 FIRST 集
     * FIRST(X) = { 从 X 出发能推导出的第一个终结符 | 若能推出 ε 则包含 $ }
     * 规则：
     *   1. X 是终结符: FIRST(X) = { X }
     *   2. X → ε:       FIRST(X) ∪= { $ }
     *   3. X → Y1Y2...Yk: 依次加入 FIRST(Yi) 的非 ε 符号，
     *      若所有 Yi 都可推导出 ε，则加入 $
     */
    bool changed = true;
    while (changed) {
        changed = false;
        for (const auto& prod : productions) {
            const string& head = prod.head;

            if (prod.body.empty()) {
                Symbol eps{SymbolType::EPSILON, "$"};
                firstSets[head].insert(eps);
            } else {
                bool allEpsilon = true;
                for (const auto& sym : prod.body) {
                    if (sym.type == SymbolType::TERMINAL) {
                        auto& fset = firstSets[head];
                        if (fset.find(sym) == fset.end()) {
                            fset.insert(sym);
                            changed = true;
                        }
                        allEpsilon = false;
                        break;
                    } else {
                        bool hasEpsilon = false;
                        for (const auto& f : firstSets[sym.name]) {
                            if (f.type == SymbolType::EPSILON) {
                                hasEpsilon = true;
                            } else {
                                if (firstSets[head].find(f) == firstSets[head].end()) {
                                    firstSets[head].insert(f);
                                    changed = true;
                                }
                            }
                        }
                        if (!hasEpsilon) {
                            allEpsilon = false;
                            break;
                        }
                    }
                }
                if (allEpsilon) {
                    Symbol eps{SymbolType::EPSILON, "$"};
                    if (firstSets[head].find(eps) == firstSets[head].end()) {
                        firstSets[head].insert(eps);
                        changed = true;
                    }
                }
            }
        }
    }
}

set<Symbol> SLRTableGenerator::computeFirstOfSeq(const vector<Symbol>& seq) {
    set<Symbol> result;
    if (seq.empty()) {
        result.insert(Symbol{SymbolType::EPSILON, "$"});
        return result;
    }
    bool allEpsilon = true;
    for (const auto& sym : seq) {
        if (sym.type == SymbolType::TERMINAL) {
            result.insert(sym);
            allEpsilon = false;
            break;
        } else {
            bool hasEpsilon = false;
            for (const auto& f : firstSets[sym.name]) {
                if (f.type == SymbolType::EPSILON) {
                    hasEpsilon = true;
                } else {
                    result.insert(f);
                }
            }
            if (!hasEpsilon) {
                allEpsilon = false;
                break;
            }
        }
    }
    if (allEpsilon) {
        result.insert(Symbol{SymbolType::EPSILON, "$"});
    }
    return result;
}

void SLRTableGenerator::computeFollow() {
    /*
     * 不动点迭代计算所有非终结符的 FOLLOW 集
     * FOLLOW(B) = { 句型中紧跟在 B 之后的终结符 }
     * 规则：
     *   1. 若 S 是开始符号，则 EOF ∈ FOLLOW(S)
     *   2. 若 A → αBβ，则 FIRST(β) 的非 ε 符号 ∈ FOLLOW(B)
     *   3. 若 A → αB 或 A → αBβ 且 β →* ε，则 FOLLOW(A) ⊆ FOLLOW(B)
     */
    Symbol eof{SymbolType::TERMINAL, "EOF_TOKEN"};
    followSets[startSymbol].insert(eof);

    bool changed = true;
    while (changed) {
        changed = false;
        for (const auto& prod : productions) {
            const string& head = prod.head;
            for (size_t i = 0; i < prod.body.size(); i++) {
                if (prod.body[i].type == SymbolType::NON_TERMINAL || prod.body[i].type == SymbolType::EPSILON) {
                    if (prod.body[i].type == SymbolType::EPSILON) continue;
                    const string& B = prod.body[i].name;
                    size_t oldSize = followSets[B].size();

                    if (i + 1 < prod.body.size()) {
                        vector<Symbol> beta(prod.body.begin() + i + 1, prod.body.end());
                        auto firstBeta = computeFirstOfSeq(beta);

                        bool hasEpsilon = false;
                        for (const auto& f : firstBeta) {
                            if (f.type == SymbolType::EPSILON) {
                                hasEpsilon = true;
                            } else {
                                followSets[B].insert(f);
                            }
                        }
                        if (hasEpsilon) {
                            for (const auto& f : followSets[head]) {
                                followSets[B].insert(f);
                            }
                        }
                    } else {
                        for (const auto& f : followSets[head]) {
                            followSets[B].insert(f);
                        }
                    }

                    if (followSets[B].size() != oldSize) {
                        changed = true;
                    }
                }
            }
        }
    }
}

set<Item> SLRTableGenerator::closure(set<Item> items) {
    /*
     * LR(0) 闭包运算
     * 若 A → α·Bβ 在项集中，则对所有 B → γ，将 B → ·γ 加入项集
     */
    bool changed = true;
    while (changed) {
        changed = false;
        set<Item> newItems = items;
        for (const auto& item : items) {
            const Production& prod = productions[item.prodId];
            if (item.dotPos < (int)prod.body.size()) {
                Symbol nextSym = prod.body[item.dotPos];
                if (nextSym.type == SymbolType::NON_TERMINAL) {
                    for (size_t i = 0; i < productions.size(); i++) {
                        if (productions[i].head == nextSym.name) {
                            Item newItem{(int)i, 0};
                            if (newItems.find(newItem) == newItems.end()) {
                                newItems.insert(newItem);
                                changed = true;
                            }
                        }
                    }
                }
            }
        }
        items = newItems;
    }
    return items;
}

set<Item> SLRTableGenerator::go(const set<Item>& items, Symbol X) {
    /*
     * GOTO 函数：从当前项集出发，读取符号 X 后进入的新项集的闭包
     * 将 I 中所有 A → α·Xβ 的项变为 A → αX·β，再求闭包
     */
    set<Item> result;
    for (const auto& item : items) {
        const Production& prod = productions[item.prodId];
        if (item.dotPos < (int)prod.body.size() && prod.body[item.dotPos] == X) {
            Item newItem{item.prodId, item.dotPos + 1};
            result.insert(newItem);
        }
    }
    return closure(result);
}

int SLRTableGenerator::findOrAddState(const set<Item>& items) {
    for (size_t i = 0; i < states.size(); i++) {
        if (states[i] == items) return (int)i;
    }
    states.push_back(items);
    return (int)states.size() - 1;
}

void SLRTableGenerator::buildLR0Items() {
    /*
     * 构造 LR(0) 项目集规范族
     * 从 Program' → ·Program 出发，反复应用 closure 和 go 生成所有状态
     */
    Item startItem{0, 0};
    set<Item> startSet = closure({startItem});
    findOrAddState(startSet);

    for (size_t i = 0; i < states.size(); i++) {
        set<string> allSymbols;
        for (const auto& item : states[i]) {
            const Production& prod = productions[item.prodId];
            if (item.dotPos < (int)prod.body.size()) {
                const Symbol& sym = prod.body[item.dotPos];
                allSymbols.insert(sym.name);
            }
        }
        for (const auto& symName : allSymbols) {
            Symbol sym;
            bool found = false;
            for (const auto& item : states[i]) {
                const Production& prod = productions[item.prodId];
                if (item.dotPos < (int)prod.body.size() && prod.body[item.dotPos].name == symName) {
                    sym = prod.body[item.dotPos];
                    found = true;
                    break;
                }
            }
            if (!found) continue;

            set<Item> goSet = go(states[i], sym);
            if (!goSet.empty()) {
                findOrAddState(goSet);
            }
        }
    }
}

void SLRTableGenerator::buildSLRTable() {
    /*
     * SLR(1) 分析表构造
     * 第一趟：遍历所有状态，若项 A → α·Xβ 且 X 是终结符 → Shift
     *         若 X 是非终结符 → GOTO
     * 第二趟：遍历所有状态，若项 A → α· 则对 FOLLOW(A) 中每个终结符 → Reduce
     * 两趟处理保证 Shift 动作先写入表格，当 Shift/Reduce 冲突时 Shift 优先
     * （解决了 dangling-else 和算符优先级带来的冲突）
     */
    size_t nStates = states.size();
    gotoTable.resize(nStates);
    actionTable.resize(nStates);

    for (size_t i = 0; i < nStates; i++) {
        for (const auto& item : states[i]) {
            const Production& prod = productions[item.prodId];

            if (item.dotPos < (int)prod.body.size()) {
                Symbol nextSym = prod.body[item.dotPos];
                if (nextSym.type == SymbolType::TERMINAL) {
                    set<Item> goSet = go(states[i], nextSym);
                    if (!goSet.empty()) {
                        int nextState = findOrAddState(goSet);
                        actionTable[i][nextSym] = {ActionEntry::SHIFT, nextState};
                    }
                } else {
                    set<Item> goSet = go(states[i], nextSym);
                    if (!goSet.empty()) {
                        int nextState = findOrAddState(goSet);
                        gotoTable[i][nextSym] = nextState;
                    }
                }
            }
        }
    }

    for (size_t i = 0; i < nStates; i++) {
        for (const auto& item : states[i]) {
            const Production& prod = productions[item.prodId];

            if (item.dotPos == (int)prod.body.size()) {
                if (prod.head == "Program'") {
                    Symbol eof{SymbolType::TERMINAL, "EOF_TOKEN"};
                    actionTable[i][eof] = {ActionEntry::ACCEPT, 0};
                } else {
                    for (const auto& followSym : followSets[prod.head]) {
                        if (actionTable[i].find(followSym) != actionTable[i].end()) {
                            continue;
                        }
                        actionTable[i][followSym] = {ActionEntry::REDUCE, item.prodId};
                    }
                }
            }
        }
    }
}

void SLRTableGenerator::generate() {
    // 生成SLR(1)分析表
    computeFirst();
    computeFollow();
    buildLR0Items();
    buildSLRTable();
}

string SLRTableGenerator::symbolToString(Symbol s) const {
    if (s.type == SymbolType::EPSILON) return "$";
    string n = s.name;
    if (n == "IDN") return "IDN";
    if (n == "INT") return "INT";
    if (n == "FLOAT") return "FLOAT";
    if (n == "EOF_TOKEN") return "EOF";
    return n;
}

void SLRTableGenerator::printFirstFollow() const {
    cout << "\n========== FIRST 集 ==========" << endl;
    for (const auto& nt : nonTerminals) {
        if (nt == "Program'") continue;
        cout << "FIRST(" << nt << ") = { ";
        bool first = true;
        for (const auto& sym : firstSets.at(nt)) {
            if (!first) cout << ", ";
            first = false;
            if (sym.type == SymbolType::EPSILON) cout << "$";
            else cout << symbolToString(sym);
        }
        cout << " }" << endl;
    }

    cout << "\n========== FOLLOW 集 ==========" << endl;
    for (const auto& nt : nonTerminals) {
        if (nt == "Program'") continue;
        cout << "FOLLOW(" << nt << ") = { ";
        bool first = true;
        auto it = followSets.find(nt);
        if (it != followSets.end()) {
            for (const auto& sym : it->second) {
                if (!first) cout << ", ";
                first = false;
                cout << symbolToString(sym);
            }
        }
        cout << " }" << endl;
    }
}

void SLRTableGenerator::printTable() const {
    cout << "\n========== SLR 分析表 ==========" << endl;

    set<Symbol> allTerminals;
    for (const auto& stateEntry : actionTable) {
        for (const auto& entry : stateEntry) {
            allTerminals.insert(entry.first);
        }
    }
    vector<string> sortedNt;
    for (const auto& nt : nonTerminals) {
        if (nt != "Program'") sortedNt.push_back(nt);
    }
    sort(sortedNt.begin(), sortedNt.end());

    cout << "状态\t";
    for (const auto& t : allTerminals) {
        cout << symbolToString(t) << "\t";
    }
    for (const auto& nt : sortedNt) {
        cout << "| " << nt << "\t";
    }
    cout << endl;

    for (size_t i = 0; i < states.size(); i++) {
        cout << i << "\t";
        for (const auto& t : allTerminals) {
            auto it = actionTable[i].find(t);
            if (it != actionTable[i].end()) {
                if (it->second.type == ActionEntry::SHIFT) {
                    cout << "s" << it->second.value;
                } else if (it->second.type == ActionEntry::REDUCE) {
                    cout << "r" << it->second.value;
                } else if (it->second.type == ActionEntry::ACCEPT) {
                    cout << "acc";
                }
            }
            cout << "\t";
        }
        for (const auto& nt : sortedNt) {
            Symbol ns{SymbolType::NON_TERMINAL, nt};
            auto it = gotoTable[i].find(ns);
            if (it != gotoTable[i].end()) {
                cout << "| " << it->second;
            }
            cout << "\t";
        }
        cout << endl;
    }

    cout << "\n产生式列表:" << endl;
    for (size_t i = 0; i < productions.size(); i++) {
        cout << "[" << i << "] " << productions[i].head << " -> ";
        if (productions[i].body.empty()) {
            cout << "$";
        } else {
            for (const auto& s : productions[i].body) {
                cout << symbolToString(s) << " ";
            }
        }
        cout << endl;
    }
}

void SLRTableGenerator::exportTableCSV(const string& filename) const {
    ofstream f(filename);
    if (!f.is_open()) return;

    set<Symbol> allTerminals;
    for (const auto& stateEntry : actionTable) {
        for (const auto& entry : stateEntry) {
            allTerminals.insert(entry.first);
        }
    }
    vector<string> sortedNt;
    for (const auto& nt : nonTerminals) {
        if (nt != "Program'") sortedNt.push_back(nt);
    }
    sort(sortedNt.begin(), sortedNt.end());

    f << "State";
    for (const auto& t : allTerminals) {
        f << "\t" << symbolToString(t);
    }
    for (const auto& nt : sortedNt) {
        f << "\t" << nt;
    }
    f << endl;

    for (size_t i = 0; i < states.size(); i++) {
        f << i;
        for (const auto& t : allTerminals) {
            f << "\t";
            auto it = actionTable[i].find(t);
            if (it != actionTable[i].end()) {
                if (it->second.type == ActionEntry::SHIFT) {
                    f << "s" << it->second.value;
                } else if (it->second.type == ActionEntry::REDUCE) {
                    f << "r" << it->second.value;
                } else if (it->second.type == ActionEntry::ACCEPT) {
                    f << "acc";
                }
            }
        }
        for (const auto& nt : sortedNt) {
            f << "\t";
            Symbol ns{SymbolType::NON_TERMINAL, nt};
            auto it = gotoTable[i].find(ns);
            if (it != gotoTable[i].end()) {
                f << it->second;
            }
        }
        f << endl;
    }
    f.close();
    cout << "SLR 分析表已导出到: " << filename << endl;
}

Symbol SLRTableGenerator::mapTokenToTerminal(const string& type, const string& value) const {
    /*
     * 将词法分析器的 Token 映射到 SLR 分析表中的终结符
     * 特殊处理：KW "main" 映射为 IDN，因为文法中函数名位置使用 Ident
     */
    if (type == "KW") {
        if (value == "main") {
            return Symbol{SymbolType::TERMINAL, "IDN"};
        }
        return Symbol{SymbolType::TERMINAL, value};
    }
    if (type == "OP") {
        return Symbol{SymbolType::TERMINAL, value};
    }
    if (type == "SE") {
        return Symbol{SymbolType::TERMINAL, value};
    }
    if (type == "IDN") {
        return Symbol{SymbolType::TERMINAL, "IDN"};
    }
    if (type == "INT") {
        return Symbol{SymbolType::TERMINAL, "INT"};
    }
    if (type == "FLOAT") {
        return Symbol{SymbolType::TERMINAL, "FLOAT"};
    }
    if (type == "EOF") {
        return Symbol{SymbolType::TERMINAL, "EOF_TOKEN"};
    }
    return Symbol{SymbolType::TERMINAL, type};
}
