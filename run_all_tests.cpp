#include "src/Lexer.h"
#include "src/SLRTableGenerator.h"
#include "src/Parser.h"
#include "src/IRGenerator.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
using namespace std;

int main() {
    vector<string> tests = {
        "tests/final/t01_var_decl.sy",
        "tests/final/t02_const_decl.sy",
        "tests/final/t03_func_def.sy",
        "tests/final/t04_arithmetic.sy",
        "tests/final/t05_relational.sy",
        "tests/final/t06_logical.sy",
        "tests/final/t07_unary.sy",
        "tests/final/t08_if_else.sy",
        "tests/final/t09_block_stmt.sy",
        "tests/final/t10_func_call.sy",
        "tests/final/t11_float_ops.sy",
        "tests/final/t12_complex_branch.sy",
        "tests/final/t13_multi_func.sy",
        "tests/final/t14_comprehensive.sy",
        "tests/final/t15_lexer_edge.sy",
    };

    int total = tests.size();
    int passA = 0, passB = 0, passC = 0, passD = 0;

    cout << "================================================" << endl;
    cout << "  C-- Compiler  --  Group 24  Full-Pipeline Test" << endl;
    cout << "  A(Lexer) + B(SLR Table) + C(Parser) + D(IRGen)" << endl;
    cout << "================================================" << endl;

    for (auto& f : tests) {
        cout << endl << "--- " << f << " ---" << endl;

        ifstream file(f);
        stringstream buf; buf << file.rdbuf();
        string src = buf.str();

        // ===== A: Lexer =====
        Lexer lexer(src);
        vector<Token> tokens;
        try {
            tokens = lexer.tokenize();
            cout << "  [A-Lexer     OK] " << tokens.size() << " tokens" << endl;
            passA++;
        } catch (const exception& e) {
            cout << "  [A-Lexer   FAIL] " << e.what() << endl;
            continue;
        }

        // ===== B: SLR Table =====
        SLRTableGenerator slrGen;
        try {
            slrGen.generate();
            cout << "  [B-SLRTable  OK] " << slrGen.getActionTable().size() << " states" << endl;
            passB++;
        } catch (const exception& e) {
            cout << "  [B-SLRTable FAIL] " << e.what() << endl;
            continue;
        }

        // ===== C: Parser =====
        Parser parser(tokens, slrGen, nullptr);
        ASTNode* root = nullptr;
        try {
            root = parser.parse();
            cout << "  [C-Parser    OK]" << endl;
            passC++;
        } catch (const exception& e) {
            cout << "  [C-Parser   FAIL] " << e.what() << endl;
            continue;
        }

        // ===== D: IRGenerator =====
        try {
            IRGeneratorVisitor irGen;
            root->accept(&irGen);
            string ir = irGen.getResult();
            if (!ir.empty()) {
                cout << "  [D-IRGen     OK] " << ir.size() << " chars" << endl;
                passD++;
            } else {
                cout << "  [D-IRGen   FAIL] empty output" << endl;
            }
        } catch (const exception& e) {
            cout << "  [D-IRGen   FAIL] " << e.what() << endl;
        }
    }

    cout << endl << "==================== RESULT ====================" << endl;
    cout << "  A  Lexer       : " << passA << "/" << total << (passA == total ? "  PASS" : "  FAIL") << endl;
    cout << "  B  SLR Table   : " << passB << "/" << total << (passB == total ? "  PASS" : "  FAIL") << endl;
    cout << "  C  Parser      : " << passC << "/" << total << (passC == total ? "  PASS" : "  FAIL") << endl;
    cout << "  D  IRGenerator : " << passD << "/" << total << (passD == total ? "  PASS" : "  FAIL") << endl;
    cout << "================================================" << endl;

    int allPass = (passA == total && passB == total && passC == total && passD == total);
    cout << (allPass ? "  ALL 4 MODULES PASSED" : "  SOME MODULES FAILED") << endl;
    cout << "================================================" << endl;

    return allPass ? 0 : 1;
}
