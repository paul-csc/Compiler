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
    auto tok = peek();
    if (tok.type == END_OF_FILE) {
        Error(m_Tokens.back().location, "Expected term");
    }

    if (auto t = match(LITERAL)) {
        TermLiteral* termLiteral = m_Allocator.alloc<TermLiteral>(*t->value);
        return m_Allocator.alloc<Term>(termLiteral);
    } else if (auto t = match(IDENTIFIER)) {
        TermIdentifier* termIdentifier = m_Allocator.alloc<TermIdentifier>(*t->value);
        return m_Allocator.alloc<Term>(termIdentifier);
    }

    if (match(L_PAREN)) {
        Expression* expr = parseExpression();
        expect(R_PAREN, "Expected ')'");
        TermParen* termParen = m_Allocator.alloc<TermParen>(expr);
        return m_Allocator.alloc<Term>(termParen);
    }

    Error(tok.location, "Unexpected token in term");
    return nullptr; // never reached
}

Expression* Parser::parseExpression() {
    Expression* left = m_Allocator.alloc<Expression>(parseTerm());

    while (auto tok = match(PLUS, MINUS, STAR, SLASH)) {
        Expression* right = m_Allocator.alloc<Expression>(parseTerm());
        ExprBinary* bin = m_Allocator.alloc<ExprBinary>(tok->ToStr()[0], left, right);
        left = m_Allocator.alloc<Expression>(bin);
    }

    return left;
}

Statement* Parser::parseStatement() {
    if (match(EXIT)) {
        Expression* expr = parseExpression();
        expect(SEMI, "Expected ';'");

        StmtExit* stmtExit = m_Allocator.alloc<StmtExit>(expr);
        return m_Allocator.alloc<Statement>(stmtExit);
    } else if (match(LET)) {
        std::string name = *expect(IDENTIFIER, "Expected identifer").value;

        expect(EQUAL, "Expected '='");
        Expression* expr = parseExpression();
        expect(SEMI, "Expected ';'");

        StmtLet* stmtLet = m_Allocator.alloc<StmtLet>(name, expr);
        return m_Allocator.alloc<Statement>(stmtLet);
    }

    std::string name = *expect(IDENTIFIER, "Expected identifier").value;

    expect(EQUAL, "Expected '='");
    Expression* expr = parseExpression();
    expect(SEMI, "Expected ';");

    StmtAssign* stmtAssign = m_Allocator.alloc<StmtAssign>(name, expr);
    return m_Allocator.alloc<Statement>(stmtAssign);
}

} // namespace Glassy
