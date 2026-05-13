#include "Lexer.h"
#include <iostream>
#include <fstream>
#include <sstream>

using namespace std;

int main(int argc, char** argv) {
    if (argc < 2) {
        cerr << "用法: " << argv[0] << " <源文件.sy>" << endl;
        return 1;
    }

    // 读取文件内容
    ifstream file(argv[1]);
    if (!file.is_open()) {
        cerr << "无法打开文件: " << argv[1] << endl;
        return 1;
    }
    stringstream buffer;
    buffer << file.rdbuf();
    string source_code = buffer.str();

    // 组长 A 负责的词法分析器运行验证
    cout << "========== [Day 1] Lexer 词法分析测试 ==========" << endl;
    Lexer lexer(source_code);
    vector<Token> tokens = lexer.tokenize();

    // 打印 Token 序列流，严格满足大作业输出要求
    for (const auto& token : tokens) {
        token.print();
    }
    cout << "================================================" << endl;
    cout << "共解析得到 " << tokens.size() - 1 << " 个有效 Token (已滤除注释与空白)。" << endl;
    cout << "已为 B/C 同学准备好输入流！" << endl;

    return 0;
}