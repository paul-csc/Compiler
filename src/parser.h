#pragma once

#include "ast.h"
#include "utils.h"

namespace Compiler {

class Parser {
  public:
    explicit Parser(const std::vector<Token>& tokens);
    Program* ParseProgram();

  private:
    Primary* ParsePrimary();
    PostfixExpression* ParsePostfixExpression();
    MultiplicativeExpression* ParseMultiplicativeExpression();
    AdditiveExpression* ParseAdditiveExpression();
    RelationalExpression* ParseRelationalExpression();
    EqualityExpression* ParseEqualityExpression();
    AssignmentExpression* ParseAssignmentExpression();
    Expression* ParseExpression();
    Statement* ParseStatement();
    Block* ParseBlock();

    const Token& Consume() { return m_Tokens[m_Index++]; }

    template <typename... Args>
    bool Match(TokenType first, Args... rest) {
        if (((m_Tokens[m_Index].Type == first) || ... || (m_Tokens[m_Index].Type == rest))) {
            return true;
        }
        return false;
    }

    Token Expect(TokenType type);

    const std::vector<Token> m_Tokens;
    size_t m_Index = 0;
    ArenaAllocator m_Allocator;
};

} // namespace Compiler
