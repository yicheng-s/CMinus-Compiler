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
        cerr << "Usage: " << argv[0] << " <source.sy> [output_prefix]" << endl;
        cerr << "  source.sy       input source file" << endl;
        cerr << "  output_prefix   (optional) if given, produces three .tsv files:" << endl;
        cerr << "                    <prefix> lex.tsv, <prefix> gra.tsv, <prefix> ir.tsv" << endl;
        return 1;
    }

    // ── 读取源文件 ──
    ifstream file(argv[1]);
    if (!file.is_open()) { cerr << "Cannot open file: " << argv[1] << endl; return 1; }
    stringstream buffer; buffer << file.rdbuf();
    string source = buffer.str();

    // ── 确定输出目标 ──
    bool evalMode = (argc >= 3);
    string prefix = evalMode ? argv[2] : "";

    ofstream lexFile, graFile, irFile;
    ostream* graOut = &cout;

    if (evalMode) {
        lexFile.open(prefix + " lex.tsv");
        graFile.open(prefix + " gra.tsv");
        irFile.open(prefix + " ir.tsv");
        if (!lexFile || !graFile || !irFile) {
            cerr << "Cannot open output .tsv files with prefix: " << prefix << endl;
            return 1;
        }
        graOut = &graFile;
    }

    // ── [1/4] 词法分析 ──
    cerr << "[1/4] Lexical Analysis started..." << endl;
    Lexer lexer(source);
    vector<Token> tokens = lexer.tokenize();

    // 输出 lex.tsv
    if (evalMode) {
        for (auto& t : tokens) {
            lexFile << t.getTypeString() << "\t" << t.value << "\t" << t.line_num << "\n";
        }
        lexFile.close();
        cerr << "  -> " << prefix << " lex.tsv exported (" << tokens.size() << " tokens)" << endl;
    }

    // ── [2/4] SLR 分析表生成 ──
    cerr << "[2/4] SLR Table Generation started..." << endl;
    SLRTableGenerator slrGen;
    slrGen.generate();

    // ── [3/4] SLR 语法分析 & AST 构建 ──
    cerr << "[3/4] SLR Parsing & AST Building started..." << endl;
    try {
        Parser parser(tokens, slrGen, graOut);
        ASTNode* astRoot = parser.parse();

        if (evalMode) {
            graFile.close();
            cerr << "  -> " << prefix << " gra.tsv exported" << endl;
        }
        cerr << "  -> AST Built Successfully!" << endl;

        // AST 可视化 (开发用)
        GraphvizVisitor dotGen("ast_graph.dot");
        astRoot->accept(&dotGen);
        cerr << "  -> AST Graphviz exported to 'ast_graph.dot'" << endl;

        // ── [4/4] LLVM IR 生成 ──
        cerr << "[4/4] LLVM IR Generation started..." << endl;
        IRGeneratorVisitor irGen;
        irGen.setSourceFilename(argv[1]);
        astRoot->accept(&irGen);

        if (evalMode) {
            irFile << irGen.getResult();
            irFile.close();
            cerr << "  -> " << prefix << " ir.tsv exported" << endl;
        } else {
            ofstream irOut("output.ll");
            irOut << irGen.getResult();
            irOut.close();
            cerr << "  -> LLVM IR exported to 'output.ll'" << endl;
        }

        cerr << "\n*** ALL PIPELINES COMPLETED SUCCESSFULLY! ***\n" << endl;

    } catch (const exception& e) {
        cerr << "\n[FATAL ERROR]: " << e.what() << endl;
        return 1;
    }

    return 0;
}
