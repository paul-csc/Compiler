#pragma once

#include "pch.h"
#include "ast.h"
#include "lexer.h"
#include "utils.h"

namespace Compiler {

class Parser {
  public:
    explicit Parser(const std::vector<Token>& tokens);
    Program* ParseProgram();

  private:
    Factor* ParseFactor();
    MultiplicativeExpression* ParseMultiplicativeExpression();
    AdditiveExpression* ParseExpression();
    Statement* ParseStatement();
    Block* ParseBlock();

    const Token& Peek(int offset = 0) const { return m_Tokens[m_Index + offset]; }

    const Token& Consume() { return m_Tokens[m_Index++]; }

    template <typename... Args>
    bool Match(TokenType first, Args... rest) {
        const Token& tok = Peek();
        if (((tok.Type == first) || ... || (tok.Type == rest))) {
            return true;
        }
        return false;
    }

    Token Expect(TokenType type) {
        const Token& tok = Peek();
        if (tok.Type != type) {
            Error(tok.Location, std::format("Expected '{}'", TokenToStr(type)));
        }
        return Consume();
    }

    ArenaAllocator m_Allocator;

    const std::vector<Token> m_Tokens;
    size_t m_Index = 0;
};

} // namespace Compiler
