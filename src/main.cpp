#include "Lexer.h"
#include "SLRTableGenerator.h"
#include "Parser.h"
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
    if (!file.is_open()) {
        cerr << "Cannot open file: " << argv[1] << endl;
        return 1;
    }
    stringstream buffer;
    buffer << file.rdbuf();
    string source_code = buffer.str();

    // [Day 1] Lexer
    Lexer lexer(source_code);
    vector<Token> tokens = lexer.tokenize();

    // [Day 2] SLR Table Generation
    SLRTableGenerator slrGen;
    slrGen.generate();

    // [Day 3] SLR Parsing + AST Construction
    try {
        Parser parser(tokens, slrGen);
        ASTNode* astRoot = parser.parse();

        cout << "Parse succeeded! AST:" << endl;
        cout << "================================================" << endl;
        Parser::printAST(astRoot);
        cout << "================================================" << endl;
    } catch (const exception& e) {
        cerr << "Parse failed: " << e.what() << endl;
        return 1;
    }

    return 0;
}
