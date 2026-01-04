#include "pch.h"
#include "Parser.h"

namespace Glassy {

Parser::Parser(const std::vector<Token>& tokens)
    : m_Tokens(tokens), m_Index(0), m_Allocator(4 * 1024 * 1024) {} // 4 MB

Program* Parser::ParseProgram() {
    m_Index = 0;
    Program* program = m_Allocator.alloc<Program>();

    while (peek().type != END_OF_FILE) {
        program->statements.push_back(parseStatement());
    }

    return program;
}

Term* Parser::parseTerm() {
    const Token& tok = peek();
    if (tok.type == END_OF_FILE) {
        Error(m_Tokens.back().location, "Expected term");
    }

    if (auto t = match(LITERAL)) {
        TermLiteral* term = m_Allocator.alloc<TermLiteral>(*t->value);
        return m_Allocator.alloc<Term>(term);
    } else if (auto t = match(IDENTIFIER)) {
        TermIdentifier* term = m_Allocator.alloc<TermIdentifier>(*t->value);
        return m_Allocator.alloc<Term>(term);
    }

    if (match(L_PAREN)) {
        Expression* expr = parseExpression();
        expect(R_PAREN);
        TermParen* term = m_Allocator.alloc<TermParen>(expr);
        return m_Allocator.alloc<Term>(term);
    }

    Error(tok.location, "Unexpected token in term");
    return nullptr; // never reached
}

Expression* Parser::parseExpression(int minPrecedence) {
    Expression* left = m_Allocator.alloc<Expression>(parseTerm());

    while (true) {
        const Token& opTok = peek();

        auto prec = GetPrecedence(opTok.type);
        if (!prec || *prec < minPrecedence) {
            break;
        }
        consume();

        Expression* right = parseExpression(*prec + 1);
        ExprBinary* bin = m_Allocator.alloc<ExprBinary>(ToStr(opTok.type)[0], left, right);
        left = m_Allocator.alloc<Expression>(bin);
    }

    return left;
}

Statement* Parser::parseStatement() {
    if (match(EXIT)) {
        Expression* expr = parseExpression();
        expect(SEMI);

        StmtExit* stmt = m_Allocator.alloc<StmtExit>(expr);
        return m_Allocator.alloc<Statement>(stmt);
    } else if (match(VAR)) {
        std::string name = *expect(IDENTIFIER).value;

        expect(EQUAL);
        Expression* expr = parseExpression();
        expect(SEMI);

        StmtDeclar* stmt = m_Allocator.alloc<StmtDeclar>(name, expr);
        return m_Allocator.alloc<Statement>(stmt);
    } else if (match(L_BRACE)) {
        Scope* scope = parseScope();
        return m_Allocator.alloc<Statement>(scope);
    } else if (match(R_BRACE)) {
        return nullptr; // special tag for scope end
    }

    std::string name = *expect(IDENTIFIER).value;

    expect(EQUAL);
    Expression* expr = parseExpression();
    expect(SEMI);

    StmtAssign* stmt = m_Allocator.alloc<StmtAssign>(name, expr);
    return m_Allocator.alloc<Statement>(stmt);
}

Scope* Parser::parseScope() {
    Scope* scope = m_Allocator.alloc<Scope>();
    while (Statement* stmt = parseStatement()) {
        scope->statements.push_back(stmt);
    }
    return scope;
}

} // namespace Glassy
