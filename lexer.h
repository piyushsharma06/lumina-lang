#pragma once
#include <string>
#include <vector>
#include <stdexcept>

// ============================================================
// TOKEN TYPES — har keyword/symbol ka ek type hota hai
// ============================================================
enum class TokenType {
    // Literals
    NUMBER, STRING, BOOL_TRUE, BOOL_FALSE,

    // Identifiers & Keywords
    IDENTIFIER,
    VAR, IF, ELSE, WHILE, FUN, RETURN, PRINT,

    // Operators
    PLUS, MINUS, STAR, SLASH, PERCENT,
    EQ, NEQ, LT, GT, LTE, GTE,
    AND, OR, NOT,
    ASSIGN,

    // Delimiters
    LPAREN, RPAREN, LBRACE, RBRACE,
    COMMA, SEMICOLON,

    END_OF_FILE
};

// ============================================================
// TOKEN — ek token = type + value + line number
// ============================================================
struct Token {
    TokenType type;
    std::string value;
    int line;

    Token(TokenType t, std::string v, int l)
        : type(t), value(std::move(v)), line(l) {}
};

// ============================================================
// LEXER — source code ko tokens mein todta hai
// ============================================================
class Lexer {
    std::string src;
    int pos = 0;
    int line = 1;

public:
    explicit Lexer(std::string source) : src(std::move(source)) {}

    std::vector<Token> tokenize() {
        std::vector<Token> tokens;

        while (pos < (int)src.size()) {
            skipWhitespaceAndComments();
            if (pos >= (int)src.size()) break;

            char c = src[pos];

            // Numbers
            if (isdigit(c)) {
                tokens.push_back(readNumber());
            }
            // Strings
            else if (c == '"') {
                tokens.push_back(readString());
            }
            // Identifiers and keywords
            else if (isalpha(c) || c == '_') {
                tokens.push_back(readIdentifier());
            }
            // Two-character operators
            else if (c == '=' && peek() == '=') { pos += 2; tokens.push_back({TokenType::EQ,  "==", line}); }
            else if (c == '!' && peek() == '=') { pos += 2; tokens.push_back({TokenType::NEQ, "!=", line}); }
            else if (c == '<' && peek() == '=') { pos += 2; tokens.push_back({TokenType::LTE, "<=", line}); }
            else if (c == '>' && peek() == '=') { pos += 2; tokens.push_back({TokenType::GTE, ">=", line}); }
            else if (c == '&' && peek() == '&') { pos += 2; tokens.push_back({TokenType::AND, "&&", line}); }
            else if (c == '|' && peek() == '|') { pos += 2; tokens.push_back({TokenType::OR,  "||", line}); }
            // Single-character tokens
            else {
                tokens.push_back(readSingle());
            }
        }

        tokens.push_back({TokenType::END_OF_FILE, "", line});
        return tokens;
    }

private:
    char peek(int offset = 1) {
        int idx = pos + offset;
        return (idx < (int)src.size()) ? src[idx] : '\0';
    }

    void skipWhitespaceAndComments() {
        while (pos < (int)src.size()) {
            char c = src[pos];
            if (c == '\n') { line++; pos++; }
            else if (isspace(c)) { pos++; }
            else if (c == '/' && peek() == '/') {
                // Single-line comment
                while (pos < (int)src.size() && src[pos] != '\n') pos++;
            }
            else break;
        }
    }

    Token readNumber() {
        std::string num;
        while (pos < (int)src.size() && (isdigit(src[pos]) || src[pos] == '.'))
            num += src[pos++];
        return {TokenType::NUMBER, num, line};
    }

    Token readString() {
        pos++; // skip opening "
        std::string str;
        while (pos < (int)src.size() && src[pos] != '"') {
            if (src[pos] == '\\' && pos + 1 < (int)src.size()) {
                pos++;
                switch (src[pos]) {
                    case 'n':  str += '\n'; break;
                    case 't':  str += '\t'; break;
                    case '"':  str += '"';  break;
                    case '\\': str += '\\'; break;
                    default:   str += src[pos];
                }
            } else {
                str += src[pos];
            }
            pos++;
        }
        pos++; // skip closing "
        return {TokenType::STRING, str, line};
    }

    Token readIdentifier() {
        std::string id;
        while (pos < (int)src.size() && (isalnum(src[pos]) || src[pos] == '_'))
            id += src[pos++];

        // Keywords check
        if (id == "var")    return {TokenType::VAR,        id, line};
        if (id == "if")     return {TokenType::IF,         id, line};
        if (id == "else")   return {TokenType::ELSE,       id, line};
        if (id == "while")  return {TokenType::WHILE,      id, line};
        if (id == "fun")    return {TokenType::FUN,        id, line};
        if (id == "return") return {TokenType::RETURN,     id, line};
        if (id == "print")  return {TokenType::PRINT,      id, line};
        if (id == "true")   return {TokenType::BOOL_TRUE,  id, line};
        if (id == "false")  return {TokenType::BOOL_FALSE, id, line};
        if (id == "not")    return {TokenType::NOT,        id, line};

        return {TokenType::IDENTIFIER, id, line};
    }

    Token readSingle() {
        char c = src[pos++];
        switch (c) {
            case '+': return {TokenType::PLUS,      "+", line};
            case '-': return {TokenType::MINUS,     "-", line};
            case '*': return {TokenType::STAR,      "*", line};
            case '/': return {TokenType::SLASH,     "/", line};
            case '%': return {TokenType::PERCENT,   "%", line};
            case '<': return {TokenType::LT,        "<", line};
            case '>': return {TokenType::GT,        ">", line};
            case '=': return {TokenType::ASSIGN,    "=", line};
            case '!': return {TokenType::NOT,       "!", line};
            case '(': return {TokenType::LPAREN,    "(", line};
            case ')': return {TokenType::RPAREN,    ")", line};
            case '{': return {TokenType::LBRACE,    "{", line};
            case '}': return {TokenType::RBRACE,    "}", line};
            case ',': return {TokenType::COMMA,     ",", line};
            case ';': return {TokenType::SEMICOLON, ";", line};
            default:
                throw std::runtime_error(
                    "Line " + std::to_string(line) +
                    ": Unknown character '" + c + "'");
        }
    }
};
