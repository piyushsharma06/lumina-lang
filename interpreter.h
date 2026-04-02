#pragma once
#include "parser.h"
#include <unordered_map>
#include <functional>
#include <iostream>
#include <sstream>
#include <cmath>

// ============================================================
// VALUE — interpreter ke andar har cheez ek Value hai
// ============================================================
struct Value {
    enum class Type { Number, String, Bool, Nil } type = Type::Nil;
    double      numVal  = 0;
    std::string strVal;
    bool        boolVal = false;

    static Value fromNum(double v)        { Value r; r.type = Type::Number; r.numVal  = v; return r; }
    static Value fromStr(std::string v)   { Value r; r.type = Type::String; r.strVal  = std::move(v); return r; }
    static Value fromBool(bool v)         { Value r; r.type = Type::Bool;   r.boolVal = v; return r; }
    static Value nil()                    { return {}; }

    bool isTruthy() const {
        if (type == Type::Bool)   return boolVal;
        if (type == Type::Number) return numVal != 0;
        if (type == Type::String) return !strVal.empty();
        return false;
    }

    std::string toString() const {
        if (type == Type::Number) {
            // Agar integer hai toh .0 mat dikhao
            if (numVal == (long long)numVal)
                return std::to_string((long long)numVal);
            std::ostringstream oss;
            oss << numVal;
            return oss.str();
        }
        if (type == Type::String) return strVal;
        if (type == Type::Bool)   return boolVal ? "true" : "false";
        return "nil";
    }
};

// ============================================================
// FUNCTION — user-defined function ka representation
// ============================================================
struct Function {
    std::string              name;
    std::vector<std::string> params;
    StmtPtr                  body;
};

// ============================================================
// ENVIRONMENT — variable scope (linked chain of scopes)
// ============================================================
class Environment {
    std::unordered_map<std::string, Value>    vars;
    std::unordered_map<std::string, Function> funs;
    Environment* parent = nullptr;

public:
    explicit Environment(Environment* p = nullptr) : parent(p) {}

    void setVar(const std::string& name, Value val) {
        // Agar parent scope mein hai toh wahan update karo
        if (vars.count(name)) { vars[name] = val; return; }
        if (parent && parent->hasVar(name)) { parent->setVar(name, val); return; }
        vars[name] = val; // naya variable
    }

    void declareVar(const std::string& name, Value val) {
        vars[name] = val; // current scope mein declare
    }

    bool hasVar(const std::string& name) const {
        if (vars.count(name)) return true;
        return parent && parent->hasVar(name);
    }

    Value getVar(const std::string& name) const {
        auto it = vars.find(name);
        if (it != vars.end()) return it->second;
        if (parent) return parent->getVar(name);
        throw std::runtime_error("Undefined variable: '" + name + "'");
    }

    void setFun(const std::string& name, Function f) {
        funs[name] = std::move(f);
        if (parent) parent->setFun(name, funs[name]);
    }

    bool hasFun(const std::string& name) const {
        if (funs.count(name)) return true;
        return parent && parent->hasFun(name);
    }

    Function getFun(const std::string& name) const {
        auto it = funs.find(name);
        if (it != funs.end()) return it->second;
        if (parent) return parent->getFun(name);
        throw std::runtime_error("Undefined function: '" + name + "'");
    }
};

// ============================================================
// RETURN EXCEPTION — return statement implement karne ke liye
// ============================================================
struct ReturnException {
    Value value;
};

// ============================================================
// INTERPRETER — AST tree walk karke execute karta hai
// ============================================================
class Interpreter {
    Environment globalEnv;

public:
    void run(const std::vector<StmtPtr>& program) {
        for (auto& stmt : program)
            execStmt(stmt, globalEnv);
    }

private:
    // ---------- Statement executor ----------

    void execStmt(const StmtPtr& stmt, Environment& env) {
        switch (stmt->kind) {

        case Stmt::Kind::Expr:
            evalExpr(stmt->exprStmt.expr, env);
            break;

        case Stmt::Kind::VarDecl:
            env.declareVar(stmt->varDecl.name,
                           evalExpr(stmt->varDecl.init, env));
            break;

        case Stmt::Kind::Print:
            std::cout << evalExpr(stmt->printStmt.value, env).toString() << "\n";
            break;

        case Stmt::Kind::Block: {
            Environment blockEnv(&env);
            for (auto& s : stmt->blockStmt.stmts)
                execStmt(s, blockEnv);
            break;
        }

        case Stmt::Kind::If: {
            Value cond = evalExpr(stmt->ifStmt.cond, env);
            if (cond.isTruthy())
                execStmt(stmt->ifStmt.thenBranch, env);
            else if (stmt->ifStmt.elseBranch)
                execStmt(stmt->ifStmt.elseBranch, env);
            break;
        }

        case Stmt::Kind::While: {
            while (evalExpr(stmt->whileStmt.cond, env).isTruthy())
                execStmt(stmt->whileStmt.body, env);
            break;
        }

        case Stmt::Kind::FunDecl:
            env.setFun(stmt->funDecl.name, {
                stmt->funDecl.name,
                stmt->funDecl.params,
                stmt->funDecl.body
            });
            break;

        case Stmt::Kind::Return:
            throw ReturnException{ evalExpr(stmt->returnStmt.value, env) };
        }
    }

    // ---------- Expression evaluator ----------

    Value evalExpr(const ExprPtr& expr, Environment& env) {
        switch (expr->kind) {

        case Expr::Kind::Number:
            return Value::fromNum(expr->numExpr.value);

        case Expr::Kind::String:
            return Value::fromStr(expr->strExpr.value);

        case Expr::Kind::Bool:
            return Value::fromBool(expr->boolExpr.value);

        case Expr::Kind::Var:
            return env.getVar(expr->varExpr.name);

        case Expr::Kind::Assign: {
            Value val = evalExpr(expr->assignExpr.value, env);
            env.setVar(expr->assignExpr.name, val);
            return val;
        }

        case Expr::Kind::Unary: {
            Value operand = evalExpr(expr->unaryExpr.operand, env);
            const std::string& op = expr->unaryExpr.op;
            if (op == "-")   return Value::fromNum(-operand.numVal);
            if (op == "not" || op == "!")
                             return Value::fromBool(!operand.isTruthy());
            throw std::runtime_error("Unknown unary op: " + op);
        }

        case Expr::Kind::Binary:
            return evalBinary(expr->binExpr, env);

        case Expr::Kind::Call:
            return evalCall(expr->callExpr, env);
        }
        return Value::nil();
    }

    Value evalBinary(const BinaryExpr& bin, Environment& env) {
        Value left  = evalExpr(bin.left,  env);
        Value right = evalExpr(bin.right, env);
        const std::string& op = bin.op;

        // Arithmetic
        if (op == "+") {
            // String concatenation support
            if (left.type == Value::Type::String || right.type == Value::Type::String)
                return Value::fromStr(left.toString() + right.toString());
            return Value::fromNum(left.numVal + right.numVal);
        }
        if (op == "-") return Value::fromNum(left.numVal - right.numVal);
        if (op == "*") return Value::fromNum(left.numVal * right.numVal);
        if (op == "/") {
            if (right.numVal == 0) throw std::runtime_error("Division by zero!");
            return Value::fromNum(left.numVal / right.numVal);
        }
        if (op == "%") return Value::fromNum(std::fmod(left.numVal, right.numVal));

        // Comparison
        if (op == "<")  return Value::fromBool(left.numVal <  right.numVal);
        if (op == ">")  return Value::fromBool(left.numVal >  right.numVal);
        if (op == "<=") return Value::fromBool(left.numVal <= right.numVal);
        if (op == ">=") return Value::fromBool(left.numVal >= right.numVal);
        if (op == "==") return Value::fromBool(left.toString() == right.toString());
        if (op == "!=") return Value::fromBool(left.toString() != right.toString());

        // Logical
        if (op == "&&") return Value::fromBool(left.isTruthy() && right.isTruthy());
        if (op == "||") return Value::fromBool(left.isTruthy() || right.isTruthy());

        throw std::runtime_error("Unknown binary operator: " + op);
    }

    Value evalCall(const CallExpr& call, Environment& env) {
        // ---------- Built-in functions ----------
        if (call.callee == "sqrt") {
            Value arg = evalExpr(call.args[0], env);
            return Value::fromNum(std::sqrt(arg.numVal));
        }
        if (call.callee == "abs") {
            Value arg = evalExpr(call.args[0], env);
            return Value::fromNum(std::abs(arg.numVal));
        }
        if (call.callee == "pow") {
            Value base = evalExpr(call.args[0], env);
            Value exp  = evalExpr(call.args[1], env);
            return Value::fromNum(std::pow(base.numVal, exp.numVal));
        }
        if (call.callee == "str") {
            Value arg = evalExpr(call.args[0], env);
            return Value::fromStr(arg.toString());
        }
        if (call.callee == "num") {
            Value arg = evalExpr(call.args[0], env);
            return Value::fromNum(std::stod(arg.strVal));
        }

        // ---------- User-defined functions ----------
        Function fun = env.getFun(call.callee);

        if (call.args.size() != fun.params.size())
            throw std::runtime_error(
                "Function '" + call.callee + "' expects " +
                std::to_string(fun.params.size()) + " args, got " +
                std::to_string(call.args.size()));

        // Naya scope banao function ke liye
        Environment funEnv(&globalEnv);
        for (int i = 0; i < (int)fun.params.size(); i++)
            funEnv.declareVar(fun.params[i], evalExpr(call.args[i], env));

        // Execute and catch return
        try {
            execStmt(fun.body, funEnv);
        } catch (ReturnException& ret) {
            return ret.value;
        }
        return Value::nil();
    }
};
