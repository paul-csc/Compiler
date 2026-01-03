#pragma once

#include "pch.h"
#include "Arena.h"
#include "Tokenizer.h"

namespace Glassy {

struct Expression;

struct ASTNode {
    ASTNode() = default;
    virtual ~ASTNode() = default;
    ASTNode(const ASTNode&) = delete;
    ASTNode& operator=(const ASTNode&) = delete;
};

struct TermLiteral : ASTNode {
    explicit TermLiteral(std::string_view l) : literal(l) {}

    std::string literal;
};
struct TermIdentifier : ASTNode {
    explicit TermIdentifier(std::string_view ident) : identifier(ident) {}

    std::string identifier;
};
struct TermParen : ASTNode {
    explicit TermParen(Expression* expr) : expr(expr) {}
    Expression* expr;
};
struct Term : ASTNode {
    explicit Term(std::variant<TermLiteral*, TermIdentifier*, TermParen*> term) : term(term) {}

    std::variant<TermLiteral*, TermIdentifier*, TermParen*> term;
};

struct ExprBinary : ASTNode {
    ExprBinary(char ch, Expression* lhs, Expression* rhs) : op(ch), left(lhs), right(rhs) {}

    char op;
    Expression* left;
    Expression* right;
};
struct Expression : ASTNode {
    explicit Expression(std::variant<ExprBinary*, Term*> expr) : expr(expr) {}

    std::variant<ExprBinary*, Term*> expr;
};

struct StmtExit : ASTNode {
    StmtExit(Expression* e) : expr(e) {}

    Expression* expr;
};
struct StmtAssign : ASTNode {
    StmtAssign(std::string_view name, Expression* expr) : identifier(name), expr(expr) {}

    std::string identifier;
    Expression* expr;
};
struct StmtLet : ASTNode {
    StmtLet(std::string_view name, Expression* expr) : identifier(name), expr(expr) {}

    std::string identifier;
    Expression* expr;
};
struct Statement : ASTNode {
    explicit Statement(std::variant<StmtExit*, StmtAssign*, StmtLet*> stmt) : stmt(stmt) {}
    std::variant<StmtExit*, StmtAssign*, StmtLet*> stmt;
};

struct Program : ASTNode {
    std::vector<Statement*> statements;
};

class Parser {
  public:
    explicit Parser(const std::vector<Token>& tokens);
    Program* ParseProgram();

  private:
    Term* parseTerm();
    Expression* parseExpression();
    Statement* parseStatement();

    const Token& peek(int offset = 0) const {
        return m_Tokens[m_Index + offset]; // always safe because EOF exists
    }

    const Token& consume() { return m_Tokens[m_Index++]; }

    template <typename... Args>
    const Token* match(TokenType first, Args... rest) {
        const Token& tok = peek();
        if (((tok.type == first) || ... || (tok.type == rest))) {
            return &consume();
        }
        return nullptr;
    }

    Token expect(TokenType type, const char* msg) {
        const Token& tok = peek();
        if (tok.type != type) {
            Error(tok.location, msg);
        }
        return consume();
    }

    ArenaAllocator m_Allocator;

    const std::vector<Token> m_Tokens;
    size_t m_Index = 0;
};

} // namespace Glassy
