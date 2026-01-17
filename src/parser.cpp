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
        return m_Allocator.alloc<Primary>(std::stoll(*Consume().Value));
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

PostfixExpression* Parser::ParsePostfixExpression() {
    PostfixExpression* expr = m_Allocator.alloc<PostfixExpression>(ParsePrimary());

    while (Match(LPAREN)) { // function call
        Consume();
        std::vector<AssignmentExpression*> argList;

        if (!Match(RPAREN)) {
            argList.emplace_back(ParseAssignmentExpression());
            while (Match(COMMA)) {
                Consume();
                argList.emplace_back(ParseAssignmentExpression());
            }
        }

        Expect(RPAREN);
        expr->CallList.emplace_back(argList);
    }

    return expr;
}

MultiplicativeExpression* Parser::ParseMultiplicativeExpression() {
    MultiplicativeExpression* expr = m_Allocator.alloc<MultiplicativeExpression>(ParsePostfixExpression());

    while (Match(STAR, FSLASH, PERCENT)) {
        auto t = Consume();
        BinaryOp op = static_cast<BinaryOp>(t.Type);
        PostfixExpression* right = ParsePostfixExpression();
        expr->Right.emplace_back(op, right);
    }

    return expr;
}

AdditiveExpression* Parser::ParseAdditiveExpression() {
    MultiplicativeExpression* left = ParseMultiplicativeExpression();
    AdditiveExpression* expr = m_Allocator.alloc<AdditiveExpression>(left);

    while (Match(PLUS, MINUS)) {
        auto t = Consume();
        BinaryOp op = static_cast<BinaryOp>(t.Type);
        MultiplicativeExpression* right = ParseMultiplicativeExpression();
        expr->Right.emplace_back(op, right);
    }

    return expr;
}

RelationalExpression* Parser::ParseRelationalExpression() {
    AdditiveExpression* left = ParseAdditiveExpression();
    RelationalExpression* expr = m_Allocator.alloc<RelationalExpression>(left);

    while (Match(GT, GE, LT, LE)) {
        auto t = Consume();
        BinaryOp op = static_cast<BinaryOp>(t.Type);
        AdditiveExpression* right = ParseAdditiveExpression();
        expr->Right.emplace_back(op, right);
    }

    return expr;
}

EqualityExpression* Parser::ParseEqualityExpression() {
    RelationalExpression* left = ParseRelationalExpression();
    EqualityExpression* expr = m_Allocator.alloc<EqualityExpression>(left);

    while (Match(IS_EQUAL, NOT_EQUAL)) {
        auto t = Consume();
        BinaryOp op = static_cast<BinaryOp>(t.Type);
        RelationalExpression* right = ParseRelationalExpression();
        expr->Right.emplace_back(op, right);
    }

    return expr;
}

AssignmentExpression* Parser::ParseAssignmentExpression() {
    AssignmentExpression* expr;

    if (Match(IDENTIFIER) && m_Tokens[m_Index + 1].Type == EQUAL) {
        std::string_view name = *Consume().Value;
        Consume(); // '='
        expr = m_Allocator.alloc<AssignmentExpression>(name, ParseEqualityExpression());
    } else {
        expr = m_Allocator.alloc<AssignmentExpression>(ParseEqualityExpression());
    }

    return expr;
}

Expression* Parser::ParseExpression() {
    return m_Allocator.alloc<Expression>(ParseAssignmentExpression());
}

Statement* Parser::ParseStatement() {
    if (Match(RETURN)) {
        Consume();
        Expression* expr = ParseExpression();
        Expect(SEMICOLON);
        ReturnStatement* stmt = m_Allocator.alloc<ReturnStatement>(expr);
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
        return m_Allocator.alloc<Statement>(stmt);
    } else if (Match(LBRACE)) {
        return m_Allocator.alloc<Statement>(ParseBlock());
    }

    Expression* expr = ParseExpression();
    Expect(SEMICOLON);

    ExpressionStatement* stmt = m_Allocator.alloc<ExpressionStatement>(expr);
    return m_Allocator.alloc<Statement>(stmt);
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