#pragma once

#include "ast.h"
#include "utils.h"
#include <unordered_map>

namespace Compiler {

class ScopeStack;

class Generator {
  public:
    explicit Generator(Program* prog, ScopeStack& scopes);
    std::string GenerateAsm();

  private:

    void Push(const std::string& reg);
    void Pop(const std::string& reg);

    std::string CreateLabel();

    void DebugPrint(const std::string& reg);

    void GeneratePrimary(const Primary* primary);
    void GeneratePostfixExpression(const PostfixExpression* expr);
    void GenerateMultiplicativeExpression(const MultiplicativeExpression* expr);
    void GenerateAdditiveExpression(const AdditiveExpression* expr);
    void GenerateRelationalExpression(const RelationalExpression* expr);
    void GenerateEqualityExpression(const EqualityExpression* expr);
    void GenerateExpression(const Expression* expr);
    void GenerateBlock(const Block* expr);
    void GenerateStatement(const Statement* stmt);

    const Program* m_Program;
    std::string m_Output;
    int64_t m_StackSize = 0;

    int m_LabelCount = 0;

    ScopeStack& m_Scopes;
};

} // namespace Compiler
