#pragma once
#include "lexer.h"
#include <memory>
#include <vector>
#include <string>

// ============================================================
// AST NODE TYPES — Abstract Syntax Tree ke building blocks
// ============================================================

struct Expr;
struct Stmt;
using ExprPtr = std::shared_ptr<Expr>;
using StmtPtr = std::shared_ptr<Stmt>;

// ---------- Expressions ----------

struct NumberExpr  { double value; };
struct StringExpr  { std::string value; };
struct BoolExpr    { bool value; };
struct VarExpr     { std::string name; };

struct BinaryExpr  {
    std::string op;
    ExprPtr left, right;
};

struct UnaryExpr   {
    std::string op;
    ExprPtr operand;
};

struct AssignExpr  {
    std::string name;
    ExprPtr value;
};

struct CallExpr    {
    std::string callee;
    std::vector<ExprPtr> args;
};

// ---------- Expr variant ----------
struct Expr {
    enum class Kind { Number, String, Bool, Var, Binary, Unary, Assign, Call } kind;
    // Only one of these is active at a time (poor man's variant for clarity)
    NumberExpr  numExpr;
    StringExpr  strExpr;
    BoolExpr    boolExpr;
    VarExpr     varExpr;
    BinaryExpr  binExpr;
    UnaryExpr   unaryExpr;
    AssignExpr  assignExpr;
    CallExpr    callExpr;
};

// Helper makers
inline ExprPtr makeNum(double v)
    { auto e = std::make_shared<Expr>(); e->kind = Expr::Kind::Number; e->numExpr = {v}; return e; }
inline ExprPtr makeStr(std::string v)
    { auto e = std::make_shared<Expr>(); e->kind = Expr::Kind::String; e->strExpr = {v}; return e; }
inline ExprPtr makeBool(bool v)
    { auto e = std::make_shared<Expr>(); e->kind = Expr::Kind::Bool; e->boolExpr = {v}; return e; }
inline ExprPtr makeVar(std::string n)
    { auto e = std::make_shared<Expr>(); e->kind = Expr::Kind::Var; e->varExpr = {n}; return e; }
inline ExprPtr makeBin(std::string op, ExprPtr l, ExprPtr r)
    { auto e = std::make_shared<Expr>(); e->kind = Expr::Kind::Binary; e->binExpr = {op, l, r}; return e; }
inline ExprPtr makeUnary(std::string op, ExprPtr operand)
    { auto e = std::make_shared<Expr>(); e->kind = Expr::Kind::Unary; e->unaryExpr = {op, operand}; return e; }
inline ExprPtr makeAssign(std::string n, ExprPtr v)
    { auto e = std::make_shared<Expr>(); e->kind = Expr::Kind::Assign; e->assignExpr = {n, v}; return e; }
inline ExprPtr makeCall(std::string callee, std::vector<ExprPtr> args)
    { auto e = std::make_shared<Expr>(); e->kind = Expr::Kind::Call; e->callExpr = {callee, args}; return e; }

// ---------- Statements ----------

struct ExprStmt   { ExprPtr expr; };
struct VarDecl    { std::string name; ExprPtr init; };
struct PrintStmt  { ExprPtr value; };
struct BlockStmt  { std::vector<StmtPtr> stmts; };
struct IfStmt     { ExprPtr cond; StmtPtr thenBranch; StmtPtr elseBranch; };
struct WhileStmt  { ExprPtr cond; StmtPtr body; };
struct FunDecl    { std::string name; std::vector<std::string> params; StmtPtr body; };
struct ReturnStmt { ExprPtr value; };

struct Stmt {
    enum class Kind { Expr, VarDecl, Print, Block, If, While, FunDecl, Return } kind;
    ExprStmt   exprStmt;
    VarDecl    varDecl;
    PrintStmt  printStmt;
    BlockStmt  blockStmt;
    IfStmt     ifStmt;
    WhileStmt  whileStmt;
    FunDecl    funDecl;
    ReturnStmt returnStmt;
};

// Helper makers for statements
inline StmtPtr makeExprStmt(ExprPtr e)
    { auto s = std::make_shared<Stmt>(); s->kind = Stmt::Kind::Expr; s->exprStmt = {e}; return s; }
inline StmtPtr makeVarDecl(std::string n, ExprPtr init)
    { auto s = std::make_shared<Stmt>(); s->kind = Stmt::Kind::VarDecl; s->varDecl = {n, init}; return s; }
inline StmtPtr makePrint(ExprPtr v)
    { auto s = std::make_shared<Stmt>(); s->kind = Stmt::Kind::Print; s->printStmt = {v}; return s; }
inline StmtPtr makeBlock(std::vector<StmtPtr> stmts)
    { auto s = std::make_shared<Stmt>(); s->kind = Stmt::Kind::Block; s->blockStmt = {stmts}; return s; }
inline StmtPtr makeIf(ExprPtr cond, StmtPtr th, StmtPtr el)
    { auto s = std::make_shared<Stmt>(); s->kind = Stmt::Kind::If; s->ifStmt = {cond, th, el}; return s; }
inline StmtPtr makeWhile(ExprPtr cond, StmtPtr body)
    { auto s = std::make_shared<Stmt>(); s->kind = Stmt::Kind::While; s->whileStmt = {cond, body}; return s; }
inline StmtPtr makeFunDecl(std::string n, std::vector<std::string> p, StmtPtr b)
    { auto s = std::make_shared<Stmt>(); s->kind = Stmt::Kind::FunDecl; s->funDecl = {n, p, b}; return s; }
inline StmtPtr makeReturn(ExprPtr v)
    { auto s = std::make_shared<Stmt>(); s->kind = Stmt::Kind::Return; s->returnStmt = {v}; return s; }


// ============================================================
// PARSER — Tokens → AST (Recursive Descent)
// ============================================================
class Parser {
    std::vector<Token> tokens;
    int pos = 0;

public:
    explicit Parser(std::vector<Token> toks) : tokens(std::move(toks)) {}

    std::vector<StmtPtr> parse() {
        std::vector<StmtPtr> program;
        while (!check(TokenType::END_OF_FILE))
            program.push_back(parseStmt());
        return program;
    }

private:
    // ---------- Utilities ----------
    Token& current() { return tokens[pos]; }
    Token& peek(int offset = 1) { return tokens[std::min(pos + offset, (int)tokens.size() - 1)]; }

    bool check(TokenType t) { return current().type == t; }

    bool match(TokenType t) {
        if (check(t)) { pos++; return true; }
        return false;
    }

    Token consume(TokenType t, const std::string& msg) {
        if (!check(t))
            throw std::runtime_error("Line " + std::to_string(current().line) + ": " + msg + " (got '" + current().value + "')");
        return tokens[pos++];
    }

    // ---------- Statements ----------

    StmtPtr parseStmt() {
        if (check(TokenType::VAR))    return parseVarDecl();
        if (check(TokenType::IF))     return parseIf();
        if (check(TokenType::WHILE))  return parseWhile();
        if (check(TokenType::FUN))    return parseFunDecl();
        if (check(TokenType::RETURN)) return parseReturn();
        if (check(TokenType::PRINT))  return parsePrint();
        if (check(TokenType::LBRACE)) return parseBlock();
        return parseExprStmt();
    }

    StmtPtr parseVarDecl() {
        consume(TokenType::VAR, "Expected 'var'");
        std::string name = consume(TokenType::IDENTIFIER, "Expected variable name").value;
        ExprPtr init = makeNum(0);
        if (match(TokenType::ASSIGN))
            init = parseExpr();
        consume(TokenType::SEMICOLON, "Expected ';' after var declaration");
        return makeVarDecl(name, init);
    }

    StmtPtr parseIf() {
        consume(TokenType::IF, "Expected 'if'");
        consume(TokenType::LPAREN, "Expected '(' after if");
        ExprPtr cond = parseExpr();
        consume(TokenType::RPAREN, "Expected ')' after condition");
        StmtPtr thenBranch = parseStmt();
        StmtPtr elseBranch = nullptr;
        if (match(TokenType::ELSE))
            elseBranch = parseStmt();
        return makeIf(cond, thenBranch, elseBranch);
    }

    StmtPtr parseWhile() {
        consume(TokenType::WHILE, "Expected 'while'");
        consume(TokenType::LPAREN, "Expected '(' after while");
        ExprPtr cond = parseExpr();
        consume(TokenType::RPAREN, "Expected ')' after condition");
        StmtPtr body = parseStmt();
        return makeWhile(cond, body);
    }

    StmtPtr parseFunDecl() {
        consume(TokenType::FUN, "Expected 'fun'");
        std::string name = consume(TokenType::IDENTIFIER, "Expected function name").value;
        consume(TokenType::LPAREN, "Expected '(' after function name");
        std::vector<std::string> params;
        if (!check(TokenType::RPAREN)) {
            do {
                params.push_back(consume(TokenType::IDENTIFIER, "Expected parameter name").value);
            } while (match(TokenType::COMMA));
        }
        consume(TokenType::RPAREN, "Expected ')' after parameters");
        StmtPtr body = parseBlock();
        return makeFunDecl(name, params, body);
    }

    StmtPtr parseReturn() {
        consume(TokenType::RETURN, "Expected 'return'");
        ExprPtr val = makeNum(0);
        if (!check(TokenType::SEMICOLON))
            val = parseExpr();
        consume(TokenType::SEMICOLON, "Expected ';' after return");
        return makeReturn(val);
    }

    StmtPtr parsePrint() {
        consume(TokenType::PRINT, "Expected 'print'");
        consume(TokenType::LPAREN, "Expected '(' after print");
        ExprPtr val = parseExpr();
        consume(TokenType::RPAREN, "Expected ')' after print argument");
        consume(TokenType::SEMICOLON, "Expected ';' after print");
        return makePrint(val);
    }

    StmtPtr parseBlock() {
        consume(TokenType::LBRACE, "Expected '{'");
        std::vector<StmtPtr> stmts;
        while (!check(TokenType::RBRACE) && !check(TokenType::END_OF_FILE))
            stmts.push_back(parseStmt());
        consume(TokenType::RBRACE, "Expected '}'");
        return makeBlock(stmts);
    }

    StmtPtr parseExprStmt() {
        ExprPtr e = parseExpr();
        consume(TokenType::SEMICOLON, "Expected ';' after expression");
        return makeExprStmt(e);
    }

    // ---------- Expressions (precedence climbing) ----------

    ExprPtr parseExpr()       { return parseAssign(); }

    ExprPtr parseAssign() {
        // Check if this is an assignment: IDENTIFIER '=' expr
        if (check(TokenType::IDENTIFIER) && peek().type == TokenType::ASSIGN) {
            std::string name = tokens[pos++].value;
            pos++; // consume '='
            ExprPtr val = parseExpr();
            return makeAssign(name, val);
        }
        return parseOr();
    }

    ExprPtr parseOr() {
        ExprPtr left = parseAnd();
        while (check(TokenType::OR)) {
            std::string op = tokens[pos++].value;
            left = makeBin(op, left, parseAnd());
        }
        return left;
    }

    ExprPtr parseAnd() {
        ExprPtr left = parseEquality();
        while (check(TokenType::AND)) {
            std::string op = tokens[pos++].value;
            left = makeBin(op, left, parseEquality());
        }
        return left;
    }

    ExprPtr parseEquality() {
        ExprPtr left = parseComparison();
        while (check(TokenType::EQ) || check(TokenType::NEQ)) {
            std::string op = tokens[pos++].value;
            left = makeBin(op, left, parseComparison());
        }
        return left;
    }

    ExprPtr parseComparison() {
        ExprPtr left = parseTerm();
        while (check(TokenType::LT)  || check(TokenType::GT) ||
               check(TokenType::LTE) || check(TokenType::GTE)) {
            std::string op = tokens[pos++].value;
            left = makeBin(op, left, parseTerm());
        }
        return left;
    }

    ExprPtr parseTerm() {
        ExprPtr left = parseFactor();
        while (check(TokenType::PLUS) || check(TokenType::MINUS)) {
            std::string op = tokens[pos++].value;
            left = makeBin(op, left, parseFactor());
        }
        return left;
    }

    ExprPtr parseFactor() {
        ExprPtr left = parseUnary();
        while (check(TokenType::STAR) || check(TokenType::SLASH) || check(TokenType::PERCENT)) {
            std::string op = tokens[pos++].value;
            left = makeBin(op, left, parseUnary());
        }
        return left;
    }

    ExprPtr parseUnary() {
        if (check(TokenType::MINUS) || check(TokenType::NOT)) {
            std::string op = tokens[pos++].value;
            return makeUnary(op, parseUnary());
        }
        return parseCall();
    }

    ExprPtr parseCall() {
        // Check: IDENTIFIER '(' → function call
        if (check(TokenType::IDENTIFIER) && peek().type == TokenType::LPAREN) {
            std::string callee = tokens[pos++].value;
            pos++; // consume '('
            std::vector<ExprPtr> args;
            if (!check(TokenType::RPAREN)) {
                do { args.push_back(parseExpr()); } while (match(TokenType::COMMA));
            }
            consume(TokenType::RPAREN, "Expected ')' after arguments");
            return makeCall(callee, args);
        }
        return parsePrimary();
    }

    ExprPtr parsePrimary() {
        if (check(TokenType::NUMBER))
            return makeNum(std::stod(tokens[pos++].value));
        if (check(TokenType::STRING))
            return makeStr(tokens[pos++].value);
        if (check(TokenType::BOOL_TRUE))  { pos++; return makeBool(true); }
        if (check(TokenType::BOOL_FALSE)) { pos++; return makeBool(false); }
        if (check(TokenType::IDENTIFIER))
            return makeVar(tokens[pos++].value);
        if (match(TokenType::LPAREN)) {
            ExprPtr e = parseExpr();
            consume(TokenType::RPAREN, "Expected ')' after expression");
            return e;
        }
        throw std::runtime_error(
            "Line " + std::to_string(current().line) +
            ": Unexpected token '" + current().value + "'");
    }
};
