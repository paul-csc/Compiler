#pragma once
#include "pch.h"

namespace Compiler {

struct Expression;
struct Statement;
struct Block;

struct Factor {
    explicit Factor(Expression* e) : Value(e) {}
    explicit Factor(const std::string& s) : Value(s) {}
    explicit Factor(int i) : Value(i) {}
    std::variant<Expression*, std::string, int> Value;
};

struct Term {
    explicit Term(Factor* t) : Left(t) {}
    Factor* Left;
    std::vector<std::pair<std::string, Factor*>> Right;
};

struct Expression {
    explicit Expression(Term* t) : Left(t) {}
    Term* Left;
    std::vector<std::pair<std::string, Term*>> Right;
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
    IfStatement(Expression* cond, Statement* s) : Cond(cond), Then(s) {}
    Expression* Cond;
    Statement* Then;
};

struct Statement {
    explicit Statement(AssignmentStatement* a) : Stmt(a) {}
    explicit Statement(IfStatement* i) : Stmt(i) {}
    explicit Statement(Block* b) : Stmt(b) {}
    std::variant<AssignmentStatement*, IfStatement*, Block*> Stmt;
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
