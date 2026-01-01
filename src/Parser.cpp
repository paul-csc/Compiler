#include "Parser.h"

namespace Glassy {

Parser::Parser(const std::vector<Token>& tokens) : m_Tokens(tokens), m_Index(0) {}

std::unique_ptr<Program> Parser::ParseProgram() {
    m_Index = 0;
    std::unique_ptr<Program> program = std::make_unique<Program>();

    while (peek()) {
        program->statements.push_back(std::move(parseStatement()));
    }

    return program;
}

std::unique_ptr<Expression> Parser::parseFactor() {
    auto tok = peek();
    if (!tok) {
        Error(m_Tokens.back().location, "Expected factor");
    }

    if (tok->type == LITERAL) {
        consume();
        return std::make_unique<LiteralExpr>(*tok->GetValue<Literal>());
    } else if (tok->type == IDENTIFIER) {
        consume();
        return std::make_unique<IdentifierExpr>(*tok->GetValue<Identifier>());
    }

    if (match<Separator>(Separator::L_PAREN)) {
        auto expr = parseExpression();
        expect(SEPARATOR, "Expected ')'");
        return expr;
    }

    Error(tok->location, "Unexpected token in factor");
    return nullptr; // never reached
}

std::unique_ptr<Expression> Parser::parseTerm() {
    auto left = parseFactor();

    while (auto op = match<Operator>(Operator::STAR, Operator::SLASH)) {
        auto right = parseFactor();
        left = std::make_unique<BinaryExpr>(*op, std::move(left), std::move(right));
    }

    return left;
}

std::unique_ptr<Expression> Parser::parseExpression() {
    auto left = parseTerm();

    while (auto op = match<Operator>(Operator::PLUS, Operator::MINUS)) {
        auto right = parseTerm();
        left = std::make_unique<BinaryExpr>(*op, std::move(left), std::move(right));
    }

    return left;
}

std::unique_ptr<Statement> Parser::parseStatement() {
    if (match<Keyword>(Keyword::EXIT)) {
        auto expr = parseExpression();

        expect(SEPARATOR, "Expected ';'");

        return std::make_unique<ExitStmt>(std::move(expr));
    } else if (match<Keyword>(Keyword::LET)) {
        Identifier name = *expect(IDENTIFIER, "Expected identifer").GetValue<Identifier>();

        expect(OPERATOR, "Expected '='");
        auto expr = parseExpression();
        expect(SEPARATOR, "Expected ';'");

        return std::make_unique<DeclarStmt>(name, std::move(expr));
    }

    Identifier name = *expect(IDENTIFIER, "Expected identifier").GetValue<Identifier>();

    expect(OPERATOR, "Expected '='");
    auto expr = parseExpression();
    expect(SEPARATOR, "Expected ';'");

    return std::make_unique<AssignStmt>(name, std::move(expr));
}

std::optional<Token> Parser::peek(const int offset) const {
    if (m_Index + offset >= m_Tokens.size()) {
        return std::nullopt;
    }
    return m_Tokens[m_Index + offset];
}

Token Parser::consume() {
    if (m_Index >= m_Tokens.size()) {
        Error(m_Tokens[m_Index - 1].location, "Unexpected end of file");
    }
    return m_Tokens[m_Index++];
}

Token Parser::expect(TokenType type, const char* msg) {
    auto tok = peek();
    if (!tok) {
        Error(m_Tokens.back().location, msg);
    }
    if (tok->type != type) {
        Error(tok->location, msg);
    }
    return consume();
}

} // namespace Glassy
