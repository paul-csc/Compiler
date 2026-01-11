#pragma once

#include "pch.h"
#include "Parser.h"

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

    void GenerateFactor(const Factor* factor);
    void GenerateMultiplicativeExpression(const MultiplicativeExpression* expr);
    void GenerateExpression(const AdditiveExpression* expr);
    void GenerateBlock(const Block* expr);
    void GenerateStatement(const Statement* stmt);

    void Push(const std::string& reg) {
        m_Output += "push " + reg + "\n";
        m_StackSize++;
    }

    void Pop(const std::string& reg) {
        if (m_StackSize <= 0) {
            Error("Stack underflow");
        }
        m_Output += "pop " + reg + "\n";
        m_StackSize--;
    }

    void BeginScope() { m_Scopes.emplace_back(); }

    void EndScope() {
        const int64_t popCount = m_Scopes.back().size();
        if (popCount != 0) {
            m_Output += "add rsp, " + std::to_string(popCount * 8) + "\n";
        }
        m_StackSize -= popCount;
        m_Scopes.pop_back();
    }

    Variable* Lookup(const std::string& name) {
        for (auto it = m_Scopes.rbegin(); it != m_Scopes.rend(); ++it) {
            auto found = it->find(name);
            if (found != it->end()) {
                return &found->second;
            }
        }
        return nullptr;
    }

    std::string CreateLabel() { return "label" + std::to_string(m_LabelCount++); }

    const Program* m_Program;
    std::string m_Output;
    int64_t m_StackSize = 0;

    int m_LabelCount = 0;

    // std::unordered_map<std::string, Variable> m_Variables;

    std::vector<std::unordered_map<std::string, Variable>> m_Scopes;
};

} // namespace Compiler
