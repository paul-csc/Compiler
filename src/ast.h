#pragma once

#include "lexer.h"
#include <optional>
#include <variant>

namespace Compiler {

enum class BinaryOp : int {
    Add = PLUS,
    Sub = MINUS,
    Mul = STAR,
    Div = FSLASH,
    Mod = PERCENT,
    Gt = GT,
    Ge = GE,
    Lt = LT,
    Le = LE,
    Eq = IS_EQUAL,
    Ne = NOT_EQUAL,
};

struct AssignmentExpression;
struct Expression;
struct Statement;
struct Block;

struct Primary {
    explicit Primary(Expression* e) : Value(e) {}
    explicit Primary(const std::string& s) : Value(s) {}
    explicit Primary(int64_t i) : Value(i) {}
    std::variant<Expression*, std::string, int64_t> Value;
};

struct PostfixExpression {
    explicit PostfixExpression(Primary* p) : Prim(p) {}
    Primary* Prim;
    std::vector<std::vector<AssignmentExpression*>> CallList;
};

struct MultiplicativeExpression { // '*' | '/'
    explicit MultiplicativeExpression(PostfixExpression* p) : Left(p) {}
    PostfixExpression* Left;
    std::vector<std::pair<BinaryOp, PostfixExpression*>> Right;
};

struct AdditiveExpression { // '+' | '-'
    explicit AdditiveExpression(MultiplicativeExpression* e) : Left(e) {}
    MultiplicativeExpression* Left;
    std::vector<std::pair<BinaryOp, MultiplicativeExpression*>> Right;
};

struct RelationalExpression { // '>' | '>=' | '<' | '<='
    explicit RelationalExpression(AdditiveExpression* e) : Left(e) {}
    AdditiveExpression* Left;
    std::vector<std::pair<BinaryOp, AdditiveExpression*>> Right;
};

struct EqualityExpression { // '==' | '!='
    explicit EqualityExpression(RelationalExpression* e) : Left(e) {}
    RelationalExpression* Left;
    std::vector<std::pair<BinaryOp, RelationalExpression*>> Right;
};

struct AssignmentExpression { // '='
    AssignmentExpression(EqualityExpression* e) : Expr(e) {}
    AssignmentExpression(std::string_view name, EqualityExpression* e) : Ident(name), Expr(e) {}
    std::optional<std::string> Ident = std::nullopt;
    EqualityExpression* Expr;
};

struct Expression {
    explicit Expression(AssignmentExpression* e) : Expr(e) {}
    AssignmentExpression* Expr;
};

struct Declaration {
    explicit Declaration(std::string_view ident) : Ident(ident) {}
    std::string Ident;
};

struct ExpressionStatement {
    explicit ExpressionStatement(Expression* e) : Expr(e) {}
    Expression* Expr;
};

struct IfStatement {
    IfStatement(Expression* cond, Statement* then, Statement* e = nullptr)
        : Cond(cond), Then(then), Else(e) {}
    Expression* Cond;
    Statement* Then;
    Statement* Else = nullptr; // optional
};

struct ReturnStatement {
    explicit ReturnStatement(Expression* e = nullptr) : Expr(e) {}
    Expression* Expr;
};

struct WhileStatement {
    WhileStatement(Expression* cond, Statement* loop) : Cond(cond), Loop(loop) {}
    Expression* Cond;
    Statement* Loop;
};

struct Statement {
    explicit Statement(ExpressionStatement* e) : Stmt(e) {}
    explicit Statement(IfStatement* i) : Stmt(i) {}
    explicit Statement(ReturnStatement* r) : Stmt(r) {}
    explicit Statement(WhileStatement* w) : Stmt(w) {}
    explicit Statement(Block* b) : Stmt(b) {}
    std::variant<ExpressionStatement*, IfStatement*, ReturnStatement*, WhileStatement*, Block*> Stmt;
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
