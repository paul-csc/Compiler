#pragma once

#include "lexer.h"
#include <variant>

namespace Compiler {

struct AdditiveExpression;
struct Statement;
struct Block;

struct Factor {
    explicit Factor(AdditiveExpression* e) : Value(e) {}
    explicit Factor(const std::string& s) : Value(s) {}
    explicit Factor(int i) : Value(i) {}
    std::variant<AdditiveExpression*, std::string, int> Value;
};

struct MultiplicativeExpression { // '*' | '/'
    explicit MultiplicativeExpression(Factor* t) : Left(t) {}
    Factor* Left;
    std::vector<std::pair<std::string, Factor*>> Right;
};

struct AdditiveExpression { // '+' | '-'
    explicit AdditiveExpression(MultiplicativeExpression* t) : Left(t) {}
    MultiplicativeExpression* Left;
    std::vector<std::pair<std::string, MultiplicativeExpression*>> Right;
};

struct RelationalExpression { // '<' | '>' | '<=' | '>='
    explicit RelationalExpression(AdditiveExpression* t) : Left(t) {}
    AdditiveExpression* Left;
    std::vector<std::pair<std::string, AdditiveExpression*>> Right;
};

struct Declaration {
    explicit Declaration(std::string_view ident) : Ident(ident) {}
    std::string Ident;
};

struct AssignmentStatement {
    AssignmentStatement(std::string_view name, AdditiveExpression* expr) : Ident(name), Expr(expr) {}
    std::string Ident;
    AdditiveExpression* Expr;
};

struct IfStatement {
    IfStatement(AdditiveExpression* cond, Statement* then, Statement* e = nullptr)
        : Cond(cond), Then(then), Else(e) {}
    AdditiveExpression* Cond;
    Statement* Then;
    Statement* Else = nullptr; // optional
};

struct WhileStatement {
    WhileStatement(AdditiveExpression* cond, Statement* loop) : Cond(cond), Loop(loop) {}
    AdditiveExpression* Cond;
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
