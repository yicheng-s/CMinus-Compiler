#include "Lexer.h"
#include <iostream>
#include <cctype>
#include <unordered_set>

using namespace std;

const unordered_set<string> KEYWORDS = {
    "int", "void", "return", "const", "main", "float", "if", "else"
};

string Token::getTypeString() const {
    switch(type) {
        case TokenType::KW: return "KW";
        case TokenType::OP: return "OP";
        case TokenType::SE: return "SE";
        case TokenType::IDN: return "IDN";
        case TokenType::INT: return "INT";
        case TokenType::FLOAT: return "FLOAT";
        case TokenType::END_OF_FILE: return "EOF";
        case TokenType::ERR: return "ERROR";
        default: return "UNKNOWN";
    }
}

void Token::print() const {
    if (type == TokenType::END_OF_FILE) return;
    cout << "[Line " << line_num << "]\t<" << getTypeString() << ", " << value << ">" << endl;
}

Lexer::Lexer(const string& source) : source_code(source), cursor(0), current_line(1) {}

char Lexer::peek(int offset) {
    if (cursor + offset >= source_code.length()) return '\0';
    return source_code[cursor + offset];
}

char Lexer::advance() {
    return source_code[cursor++];
}

bool Lexer::isAtEnd() {
    return cursor >= source_code.length();
}

void Lexer::skipWhitespaceAndComments() {
    while (!isAtEnd()) {
        char c = peek();
        if (c == ' ' || c == '\r' || c == '\t') {
            advance();
        } else if (c == '\n') {
            current_line++;
            advance();
        } else if (c == '/' && peek(1) == '/') { // 单行注释
            while (!isAtEnd() && peek() != '\n') advance();
        } else if (c == '/' && peek(1) == '*') { // 多行注释
            advance(); advance(); // 跳过 /*
            while (!isAtEnd()) {
                if (peek() == '\n') current_line++;
                if (peek() == '*' && peek(1) == '/') {
                    advance(); advance(); // 跳过 */
                    break;
                }
                advance();
            }
        } else {
            break;
        }
    }
}

vector<Token> Lexer::tokenize() {
    vector<Token> tokens;

    while (!isAtEnd()) {
        skipWhitespaceAndComments();
        if (isAtEnd()) break;

        char c = peek();
        int start_line = current_line;

        // 1. 标识符与关键字
        if (isalpha(c) || c == '_') {
            string text = "";
            while (!isAtEnd() && (isalnum(peek()) || peek() == '_')) {
                text += advance();
            }
            if (KEYWORDS.find(text) != KEYWORDS.end()) {
                tokens.push_back({TokenType::KW, text, start_line});
            } else {
                tokens.push_back({TokenType::IDN, text, start_line});
            }
        }
        // 2. 数字 (整数与浮点数)
        else if (isdigit(c)) {
            string num = "";
            bool is_float = false;
            while (!isAtEnd() && (isdigit(peek()) || peek() == '.')) {
                if (peek() == '.') is_float = true;
                num += advance();
            }
            tokens.push_back({is_float ? TokenType::FLOAT : TokenType::INT, num, start_line});
        }
        // 3. 运算符 (处理双字符)
        else if (string("=!<>&|+-*/%").find(c) != string::npos) {
            string op(1, advance());
            char next = peek();
            if ((op == "=" && next == '=') || (op == "!" && next == '=') ||
                (op == "<" && next == '=') || (op == ">" && next == '=') ||
                (op == "&" && next == '&') || (op == "|" && next == '|')) {
                op += advance();
            }
            tokens.push_back({TokenType::OP, op, start_line});
        }
        // 4. 界符
        else if (string("(){};,").find(c) != string::npos) {
            tokens.push_back({TokenType::SE, string(1, advance()), start_line});
        }
        // 5. 无法识别的非法字符
        else {
            tokens.push_back({TokenType::ERR, string(1, advance()), start_line});
        }
    }
    tokens.push_back({TokenType::END_OF_FILE, "EOF", current_line});
    return tokens;
}