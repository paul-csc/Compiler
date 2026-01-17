#pragma once

#include "ast.h"
#include "utils.h"
#include <unordered_map>

namespace Compiler {

struct Generator {
  public:
    explicit Generator(Program* prog);
    std::string GenerateAsm();

  private:
    struct Variable {
        int64_t StackLocation;
    };

    template <typename... Ts>
    struct overloaded : Ts... {
        using Ts::operator()...;
    };

    template <typename... Ts>
    overloaded(Ts...) -> overloaded<Ts...>;

    void Push(const std::string& reg);
    void Pop(const std::string& reg);

    void BeginScope();
    void EndScope();

    Variable* LookupVar(const std::string& name);
    std::string CreateLabel();

    void DebugPrint(const std::string& reg);

    void GeneratePrimary(const Primary* primary);
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

    // std::unordered_map<std::string, Variable> m_Variables;

    std::vector<std::unordered_map<std::string, Variable>> m_Scopes;
};

} // namespace Compiler
