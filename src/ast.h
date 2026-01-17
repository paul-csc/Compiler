#pragma once

#include "lexer.h"
#include <variant>

namespace Compiler {

struct Expression;
struct Statement;
struct Block;

struct Primary {
    explicit Primary(Expression* e) : Value(e) {}
    explicit Primary(const std::string& s) : Value(s) {}
    explicit Primary(int i) : Value(i) {}
    std::variant<Expression*, std::string, int> Value;
};

struct MultiplicativeExpression { // '*' | '/'
    explicit MultiplicativeExpression(Primary* t) : Left(t) {}
    Primary* Left;
    std::vector<std::pair<std::string, Primary*>> Right;
};

struct AdditiveExpression { // '+' | '-'
    explicit AdditiveExpression(MultiplicativeExpression* t) : Left(t) {}
    MultiplicativeExpression* Left;
    std::vector<std::pair<std::string, MultiplicativeExpression*>> Right;
};

struct EqualityExpression { // '==' | '!='
    explicit EqualityExpression(AdditiveExpression* t) : Left(t) {}
    AdditiveExpression* Left;
    std::vector<std::pair<std::string, AdditiveExpression*>> Right;
};

struct Expression {
    explicit Expression(EqualityExpression* e) : Expr(e) {}
    EqualityExpression* Expr;
};

struct Declaration {
    explicit Declaration(std::string_view ident) : Ident(ident) {}
    std::string Ident;
};

struct AssignmentStatement {
    AssignmentStatement(std::string_view name, Expression* expr) : Ident(name), Expr(expr) {}
    std::string Ident;
    Expression* Expr;
};

struct IfStatement {
    IfStatement(Expression* cond, Statement* then, Statement* e = nullptr)
        : Cond(cond), Then(then), Else(e) {}
    Expression* Cond;
    Statement* Then;
    Statement* Else = nullptr; // optional
};

struct WhileStatement {
    WhileStatement(Expression* cond, Statement* loop) : Cond(cond), Loop(loop) {}
    Expression* Cond;
    Statement* Loop;
};

struct Statement {
    explicit Statement(AssignmentStatement* a) : Stmt(a) {}
    explicit Statement(IfStatement* i) : Stmt(i) {}
    explicit Statement(WhileStatement* w) : Stmt(w) {}
    explicit Statement(Block* b) : Stmt(b) {}
    std::variant<AssignmentStatement*, IfStatement*, WhileStatement*, Block*> Stmt;
};

struct BlockItem {
    explicit BlockItem(Statement* s) : Item(s) {}
    explicit BlockItem(Declaration* d) : Item(d) {}
    std::variant<Statement*, Declaration*> Item;
};

struct Block {
    std::vector<BlockItem*> Items;
};

struct Program {
    explicit Program(Block* b) : GlobalBlock(b) {}
    Block* GlobalBlock;
};

} // namespace Compiler
