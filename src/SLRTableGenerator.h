#ifndef SLR_TABLE_GENERATOR_H
#define SLR_TABLE_GENERATOR_H

#include <string>
#include <vector>
#include <map>
#include <set>
#include <iostream>
#include <fstream>
#include <algorithm>

using namespace std;

enum class SymbolType { TERMINAL, NON_TERMINAL, EPSILON };

struct Symbol {
    SymbolType type;
    string name;
    bool operator==(const Symbol& other) const {
        return type == other.type && name == other.name;
    }
    bool operator<(const Symbol& other) const {
        if (type != other.type) return type < other.type;
        return name < other.name;
    }
};

struct Production {
    string head;
    vector<Symbol> body;
};

struct Item {
    int prodId;
    int dotPos;

    bool operator==(const Item& other) const {
        return prodId == other.prodId && dotPos == other.dotPos;
    }
    bool operator<(const Item& other) const {
        if (prodId != other.prodId) return prodId < other.prodId;
        return dotPos < other.dotPos;
    }
};

struct ActionEntry {
    enum ActionType { SHIFT, REDUCE, ACCEPT, ERROR } type;
    int value;
};

class SLRTableGenerator {
private:
    vector<Production> productions;
    map<string, set<Symbol>> firstSets;
    map<string, set<Symbol>> followSets;
    vector<map<Symbol, int>> gotoTable;
    vector<map<Symbol, ActionEntry>> actionTable;
    vector<set<Item>> states;
    set<string> nonTerminals;
    set<Symbol> terminals;
    string startSymbol;

    void initGrammar();
    void addProduction(string head, initializer_list<string> bodySym);

    void computeFirst();
    void computeFollow();
    set<Symbol> computeFirstOfSeq(const vector<Symbol>& seq);

    void buildLR0Items();
    set<Item> closure(set<Item> items);
    set<Item> go(const set<Item>& items, Symbol X);
    int findOrAddState(const set<Item>& items);

    void buildSLRTable();

    string symbolToString(Symbol s) const;

public:
    SLRTableGenerator();
    void generate();
    void printFirstFollow() const;
    void printTable() const;
    void exportTableCSV(const string& filename) const;

    const vector<map<Symbol, ActionEntry>>& getActionTable() const { return actionTable; }
    const vector<map<Symbol, int>>& getGotoTable() const { return gotoTable; }
    const vector<Production>& getProductions() const { return productions; }
    const map<string, set<Symbol>>& getFirstSets() const { return firstSets; }
    const map<string, set<Symbol>>& getFollowSets() const { return followSets; }

    Symbol mapTokenToTerminal(const string& type, const string& value) const;
};

#endif
