#include "parser.h"
#include <format>

namespace Compiler {

Parser::Parser(const std::vector<Token>& tokens)
    : m_Tokens(tokens), m_Index(0), m_Allocator(4 * 1024 * 1024) {} // 4 MB

Program* Parser::ParseProgram() {
    m_Index = 0;
    return m_Allocator.alloc<Program>(ParseBlock());
}

Primary* Parser::ParsePrimary() {
    if (Match(END_OF_FILE)) {
        Error(m_Tokens.back().Location, "Expected primary");
    } else if (Match(LITERAL)) {
        return m_Allocator.alloc<Primary>(std::stoi(*Consume().Value));
    } else if (Match(IDENTIFIER)) {
        return m_Allocator.alloc<Primary>(*Consume().Value);
    } else if (Match(LPAREN)) {
        Consume();
        Expression* expr = ParseExpression();
        Expect(RPAREN);
        return m_Allocator.alloc<Primary>(expr);
    }

    Error("Unexpected token in primary");
    return nullptr; // never reached
}

MultiplicativeExpression* Parser::ParseMultiplicativeExpression() {
    MultiplicativeExpression* expr = m_Allocator.alloc<MultiplicativeExpression>(ParsePrimary());

    while (Match(STAR, FSLASH)) {
        auto t = Consume();
        Primary* right = ParsePrimary();
        expr->Right.emplace_back(TokenToStr(t.Type), right);
    }

    return expr;
}

AdditiveExpression* Parser::ParseAdditiveExpression() {
    MultiplicativeExpression* left = ParseMultiplicativeExpression();
    AdditiveExpression* expr = m_Allocator.alloc<AdditiveExpression>(left);

    while (Match(PLUS, MINUS)) {
        auto t = Consume();
        MultiplicativeExpression* right = ParseMultiplicativeExpression();
        expr->Right.emplace_back(TokenToStr(t.Type), right);
    }

    return expr;
}

RelationalExpression* Parser::ParseRelationalExpression() {
    AdditiveExpression* left = ParseAdditiveExpression();
    RelationalExpression* expr = m_Allocator.alloc<RelationalExpression>(left);

    while (Match(GT, GE, LT, LE)) {
        auto t = Consume();
        AdditiveExpression* right = ParseAdditiveExpression();
        expr->Right.emplace_back(TokenToStr(t.Type), right);
    }

    return expr;
}

EqualityExpression* Parser::ParseEqualityExpression() {
    RelationalExpression* left = ParseRelationalExpression();
    EqualityExpression* expr = m_Allocator.alloc<EqualityExpression>(left);

    while (Match(IS_EQUAL, NOT_EQUAL)) {
        auto t = Consume();
        RelationalExpression* right = ParseRelationalExpression();
        expr->Right.emplace_back(TokenToStr(t.Type), right);
    }

    return expr;
}

Expression* Parser::ParseExpression() {
    return m_Allocator.alloc<Expression>(ParseEqualityExpression());
}

Statement* Parser::ParseStatement() {
    if (Match(IDENTIFIER)) {
        std::string_view name = *Consume().Value;
        Expect(EQUAL);
        Expression* expr = ParseExpression();
        Expect(SEMICOLON);

        AssignmentStatement* stmt = m_Allocator.alloc<AssignmentStatement>(name, expr);
        return m_Allocator.alloc<Statement>(stmt);
    } else if (Match(IF)) {
        Consume();
        Expect(LPAREN);
        Expression* expr = ParseExpression();
        Expect(RPAREN);

        IfStatement* stmt = m_Allocator.alloc<IfStatement>(expr, ParseStatement());
        if (Match(ELSE)) {
            Consume();
            stmt->Else = ParseStatement();
        }
        return m_Allocator.alloc<Statement>(stmt);
    } else if (Match(WHILE)) {
        Consume();
        Expect(LPAREN);
        Expression* expr = ParseExpression();
        Expect(RPAREN);

        WhileStatement* stmt = m_Allocator.alloc<WhileStatement>(expr, ParseStatement());
        if (Match(ELSE)) {
            Consume();
            stmt->Loop = ParseStatement();
        }
        return m_Allocator.alloc<Statement>(stmt);
    } else if (Match(LBRACE)) {
        return m_Allocator.alloc<Statement>(ParseBlock());
    }

    Error("Expected statement");
    return nullptr;
}

Block* Parser::ParseBlock() {
    Block* block = m_Allocator.alloc<Block>();
    Expect(LBRACE);
    while (!Match(RBRACE, END_OF_FILE)) {
        BlockItem* item;
        if (Match(INT)) {
            Consume();
            std::string name = *Expect(IDENTIFIER).Value;
            Expect(SEMICOLON);
            Declaration* decl = m_Allocator.alloc<Declaration>(name);

            item = m_Allocator.alloc<BlockItem>(decl);
        } else {
            item = m_Allocator.alloc<BlockItem>(ParseStatement());
        }
        block->Items.emplace_back(item);
    }
    Expect(RBRACE);
    return block;
}

Token Parser::Expect(TokenType type) {
    if (m_Tokens[m_Index].Type != type) {
        Error(m_Tokens[m_Index].Location, std::format("Expected '{}'", TokenToStr(type)));
    }
    return Consume();
}

} // namespace Compiler
