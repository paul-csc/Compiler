#pragma once

#include "pch.h"
#include "Arena.h"
#include "Tokenizer.h"

namespace Compiler {

struct Expression;
struct Scope;

struct ASTNode {
    ASTNode() = default;
    virtual ~ASTNode() = default;
    ASTNode(const ASTNode&) = delete;
    ASTNode& operator=(const ASTNode&) = delete;
};

struct TermLiteral : ASTNode {
    explicit TermLiteral(std::string_view l) : Literal(l) {}
    std::string Literal;
};
struct TermIdentifier : ASTNode {
    explicit TermIdentifier(std::string_view ident) : Identifier(ident) {}
    std::string Identifier;
};
struct TermParen : ASTNode {
    explicit TermParen(Expression* expr) : Expr(expr) {}
    Expression* Expr;
};
struct Term : ASTNode {
    explicit Term(std::variant<TermLiteral*, TermIdentifier*, TermParen*> term) : Term(term) {}
    std::variant<TermLiteral*, TermIdentifier*, TermParen*> Term;
};

struct ExprBinary : ASTNode {
    ExprBinary(char ch, Expression* lhs, Expression* rhs) : Op(ch), Left(lhs), Right(rhs) {}
    char Op;
    Expression* Left;
    Expression* Right;
};
struct Expression : ASTNode {
    explicit Expression(std::variant<ExprBinary*, Term*> expr) : Expr(expr) {}
    std::variant<ExprBinary*, Term*> Expr;
};

struct StmtExit : ASTNode {
    StmtExit(Expression* e) : Expr(e) {}
    Expression* Expr;
};
struct StmtAssign : ASTNode {
    StmtAssign(std::string_view name, Expression* expr) : identifier(name), expr(expr) {}
    std::string identifier;
    Expression* expr;
};
struct StmtDeclar : ASTNode {
    StmtDeclar(std::string_view name, Expression* expr) : Identifier(name), Expr(expr) {}
    std::string Identifier;
    Expression* Expr;
};
struct StmtIf : ASTNode {
    StmtIf(Expression* cond, Scope* s) : Cond(cond), Scope(s) {}
    Expression* Cond;
    Scope* Scope;
};
struct Statement : ASTNode {
    explicit Statement(std::variant<StmtExit*, StmtAssign*, StmtDeclar*, StmtIf*, Scope*> stmt)
        : Stmt(stmt) {}
    std::variant<StmtExit*, StmtAssign*, StmtDeclar*, StmtIf*, Scope*> Stmt;
};

struct Scope : ASTNode {
    std::vector<Statement*> Statements;
};

struct Program : ASTNode {
    std::vector<Statement*> Statements;
};

class Parser {
  public:
    explicit Parser(const std::vector<Token>& tokens);
    Program* ParseProgram();

  private:
    Term* ParseTerm();
    Expression* ParseExpression(int minPrecedence = 0);
    Statement* ParseStatement();
    Scope* ParseScope();

    const Token& Peek(int offset = 0) const {
        return m_Tokens[m_Index + offset]; // always safe because EOF exists
    }

    const Token& Consume() { return m_Tokens[m_Index++]; }

    template <typename... Args>
    const Token* Match(TokenType first, Args... rest) {
        const Token& tok = Peek();
        if (((tok.Type == first) || ... || (tok.Type == rest))) {
            return &Consume();
        }
        return nullptr;
    }

    Token Expect(TokenType type) {
        const Token& tok = Peek();
        if (tok.Type != type) {
            Error(tok.Location, std::format("Expected '{}'", ToStr(type)));
        }
        return Consume();
    }

    std::optional<int> GetPrecedence(TokenType type) {
        switch (type) {
            case PLUS:
            case MINUS: return 0;

            case STAR:
            case F_SLASH: return 1;

            default: return {};
        }
    }

    ArenaAllocator m_Allocator;

    const std::vector<Token> m_Tokens;
    size_t m_Index = 0;
};

} // namespace Compiler
