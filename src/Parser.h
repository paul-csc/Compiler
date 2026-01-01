#pragma once

#include "Tokenizer.h"
#include <memory>
#include <optional>

namespace Glassy {

/*
    TODO:
    - unary operators
    - variable expressions
*/

struct LiteralExpr;
struct IdentifierExpr;
struct BinaryExpr;
struct AssignStmt;
struct DeclarStmt;
struct ExitStmt;
struct Program;

struct AstVisitor {
    virtual void visit(const LiteralExpr& node) = 0;
    virtual void visit(const IdentifierExpr& node) = 0;
    virtual void visit(const BinaryExpr& node) = 0;
    virtual void visit(const AssignStmt& node) = 0;
    virtual void visit(const DeclarStmt& node) = 0;
    virtual void visit(const ExitStmt& node) = 0;
    virtual void visit(const Program& node) = 0;

    virtual ~AstVisitor() = default;
};

struct ASTNode {
    ASTNode() = default;
    virtual ~ASTNode() = default;
    ASTNode(const ASTNode&) = delete;
    ASTNode& operator=(const ASTNode&) = delete;

    virtual void print(std::ostream& out) const = 0;
    virtual void accept(AstVisitor& visitor) const = 0;

    friend std::ostream& operator<<(std::ostream& os, const ASTNode& node) {
        node.print(os);
        return os;
    }
};

struct Expression : ASTNode {
    virtual ~Expression() = default;
};

struct LiteralExpr : Expression { // 3
    explicit LiteralExpr(Literal val) : value(val) {}

    void print(std::ostream& out) const override { out << value; }
    void accept(AstVisitor& visitor) const override { visitor.visit(*this); }

    Literal value;
};

struct IdentifierExpr : Expression { // a
    explicit IdentifierExpr(std::string_view id) : name(id) {}

    void print(std::ostream& out) const override { out << name; }
    void accept(AstVisitor& visitor) const override { visitor.visit(*this); }

    Identifier name;
};

struct BinaryExpr : Expression { // 3 + b
    BinaryExpr(Operator op, std::unique_ptr<Expression> lhs, std::unique_ptr<Expression> rhs)
        : op(op), left(std::move(lhs)), right(std::move(rhs)) {}

    void print(std::ostream& out) const override {
        out << *left << ' ' << OperatorToStr[size_t(op)][0] << ' ' << *right;
    }
    void accept(AstVisitor& visitor) const override { visitor.visit(*this); }

    Operator op;
    std::unique_ptr<Expression> left;
    std::unique_ptr<Expression> right;
};

struct Statement : ASTNode {
    virtual ~Statement() = default;
};

struct AssignStmt : Statement { // a = 3 + b
    AssignStmt(std::string_view name, std::unique_ptr<Expression> expr)
        : identifier(name), expr(std::move(expr)) {}

    void print(std::ostream& out) const override { out << identifier << " = " << *expr << ";"; }
    void accept(AstVisitor& visitor) const override { visitor.visit(*this); }

    Identifier identifier;
    std::unique_ptr<Expression> expr;
};

struct DeclarStmt : Statement { // let a = 3 + b
    DeclarStmt(std::string_view name, std::unique_ptr<Expression> expr)
        : identifier(name), expr(std::move(expr)) {}

    void print(std::ostream& out) const override { out << "let " << identifier << " = " << *expr << ";"; }
    void accept(AstVisitor& visitor) const override { visitor.visit(*this); }

    Identifier identifier;
    std::unique_ptr<Expression> expr;
};

struct ExitStmt : Statement { // exit 3;
    ExitStmt(std::unique_ptr<Expression> e) : expr(std::move(e)) {}

    void print(std::ostream& out) const override { out << "exit " << *expr << ";"; }
    void accept(AstVisitor& visitor) const override { visitor.visit(*this); }

    std::unique_ptr<Expression> expr;
};

struct Program : ASTNode {
    void print(std::ostream& out) const override {
        out << "Program:\n";
        for (const auto& stmt : statements) {
            out << *stmt << '\n';
        }
    }
    void accept(AstVisitor& visitor) const override { visitor.visit(*this); }

    std::vector<std::unique_ptr<Statement>> statements;
};

class Parser {
  public:
    explicit Parser(const std::vector<Token>& tokens);

    std::unique_ptr<Program> ParseProgram();

  private:
    std::unique_ptr<Expression> parseFactor();
    std::unique_ptr<Expression> parseTerm();
    std::unique_ptr<Expression> parseExpression();
    std::unique_ptr<Statement> parseStatement();

    [[nodiscard]] std::optional<Token> peek(const int offset = 0) const;
    Token consume();

    template <typename T, typename... Ts>
    std::optional<T> match(T type, Ts... rest);

    Token expect(TokenType type, const char* msg);

    const std::vector<Token> m_Tokens;
    size_t m_Index = 0;
};

template <typename T, typename... Ts>
inline std::optional<T> Parser::match(T first, Ts... rest) {
    auto tok = peek();
    if (!tok) {
        return std::nullopt;
    }

    if (auto val = tok->GetValue<T>()) {
        if (((*val == first) || ... || (*val == rest))) {
            consume();
            return *val;
        }
    }
    return std::nullopt;
}

} // namespace Glassy
