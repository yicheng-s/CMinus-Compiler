#ifndef LEXER_H
#define LEXER_H

#include <string>
#include <vector>

using namespace std;

// 定义 Token 种别
enum class TokenType {
    KW,         // 关键字
    OP,         // 运算符
    SE,         // 界符
    IDN,        // 标识符
    INT,        // 整数
    FLOAT,      // 浮点数
    END_OF_FILE,// EOF
    ERR         // 错误
};

// Token 结构体
struct Token {
    TokenType type;
    string value;
    int line_num; // 记录行号，对后续报错极重要

    string getTypeString() const;
    void print() const;
};

class Lexer {
private:
    string source_code;
    size_t cursor;
    int current_line;

    char peek(int offset = 0);
    char advance();
    bool isAtEnd();
    void skipWhitespaceAndComments();

public:
    Lexer(const string& source);
    vector<Token> tokenize();
};

#endif // LEXER_H