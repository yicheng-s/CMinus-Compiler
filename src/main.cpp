#include "Lexer.h"
#include "SLRTableGenerator.h"
#include "Parser.h"
#include "IRGenerator.h"
#include "GraphvizVisitor.h"
#include <iostream>
#include <fstream>
#include <sstream>

using namespace std;

int main(int argc, char** argv) {
    if (argc < 2) {
        cerr << "Usage: " << argv[0] << " <source.sy>" << endl;
        return 1;
    }

    ifstream file(argv[1]);
    if (!file.is_open()) { cerr << "Cannot open file!" << endl; return 1; }
    stringstream buffer; buffer << file.rdbuf();

    cout << "[1/4] Lexical Analysis started..." << endl;
    Lexer lexer(buffer.str());
    vector<Token> tokens = lexer.tokenize();

    cout << "[2/4] SLR Table Generation started..." << endl;
    SLRTableGenerator slrGen;
    slrGen.generate();

    cout << "[3/4] SLR Parsing & AST Building started..." << endl;
    try {
        Parser parser(tokens, slrGen);
        ASTNode* astRoot = parser.parse();
        cout << "  -> AST Built Successfully!" << endl;

        GraphvizVisitor dotGen("ast_graph.dot");
        astRoot->accept(&dotGen);
        cout << "  -> AST Graphviz exported to 'ast_graph.dot'" << endl;

        cout << "[4/4] LLVM IR Generation started..." << endl;
        IRGeneratorVisitor irGen;
        astRoot->accept(&irGen);
        
        ofstream irFile("output.ll");
        irFile << irGen.getResult();
        irFile.close();
        cout << "  -> LLVM IR exported to 'output.ll'!" << endl;

        cout << "\n*** ALL PIPELINES COMPLETED SUCCESSFULLY! ***\n" << endl;

    } catch (const exception& e) {
        cerr << "\n[FATAL ERROR]: " << e.what() << endl;
        return 1;
    }

    return 0;
}