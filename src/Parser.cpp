#include "pch.h"
#include "Parser.h"

namespace Compiler {

Parser::Parser(const std::vector<Token>& tokens)
    : m_Tokens(tokens), m_Index(0), m_Allocator(4 * 1024 * 1024) {} // 4 MB

Program* Parser::ParseProgram() {
    m_Index = 0;
    return m_Allocator.alloc<Program>(ParseBlock());
}

Factor* Parser::ParseFactor() {
    const Token& tok = Peek();

    if (tok.Type == END_OF_FILE) {
        Error(m_Tokens.back().Location, "Expected term");
    } else if (auto t = Match(LITERAL, IDENTIFIER)) {
        return m_Allocator.alloc<Factor>(std::stoi(*t->Value));
    } else if (Match(IDENTIFIER)) {
        return m_Allocator.alloc<Factor>(*t->Value);
    } else if (Match(LPAREN)) {
        Expression* expr = ParseExpression();
        Expect(RPAREN);
        return m_Allocator.alloc<Factor>(expr);
    }

    Error(tok.Location, "Unexpected token in term");
    return nullptr; // never reached
}

Term* Parser::ParseTerm() {
    Term* term = m_Allocator.alloc<Term>(ParseFactor());

    while (auto op = Match(STAR, FSLASH)) {
        Factor* right = ParseFactor();
        term->Right.emplace_back(TokenToStr(op->Type), right);
    }

    return term;
}

Expression* Parser::ParseExpression() {
    Term* left = ParseTerm();
    Expression* term = m_Allocator.alloc<Expression>(left);

    while (auto op = Match(PLUS, MINUS)) {
        Term* right = ParseTerm();
        term->Right.emplace_back(TokenToStr(op->Type), right);
    }

    return term;
}

Declaration* Parser::ParseDeclaration() {
    Expect(INT);
    std::string_view name = *Expect(IDENTIFIER).Value;
    Expect(SEMICOLON);
    return m_Allocator.alloc<Declaration>(name);
}

Statement* Parser::ParseStatement() {
    if (auto id = Match(IDENTIFIER)) {
        std::string_view name = *id->Value;
        Expect(EQUAL);
        Expression* expr = ParseExpression();
        Expect(SEMICOLON);

        AssignmentStatement* stmt = m_Allocator.alloc<AssignmentStatement>(name, expr);
        return m_Allocator.alloc<Statement>(stmt);
    } else if (Match(IF)) {
        Expect(LPAREN);
        Expression* expr = ParseExpression();
        Expect(RPAREN);
        Expect(LBRACE);

        IfStatement* stmt = m_Allocator.alloc<IfStatement>(expr, ParseStatement());
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
    while (Peek().Type != RBRACE && Peek().Type != END_OF_FILE) {
        BlockItem* item;
        if (Peek().Type == INT) {
            item = m_Allocator.alloc<BlockItem>(ParseDeclaration());
        } else {
            item = m_Allocator.alloc<BlockItem>(ParseStatement());
        }
        block->Items.emplace_back(item);
    }
    Expect(RBRACE);
    return block;
}

} // namespace Compiler
