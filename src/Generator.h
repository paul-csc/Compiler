#pragma once

#include "pch.h"
#include "Parser.h"

namespace Glassy {

struct Generator {
  public:
    explicit Generator(Program* prog);
    std::string GenerateAsm();

  private:
    struct Variable {
        size_t stackLocation;
    };

    template <typename... Ts>
    struct overloaded : Ts... {
        using Ts::operator()...;
    };

    template <typename... Ts>
    overloaded(Ts...) -> overloaded<Ts...>;

    void generateTerm(const Term* term);
    void generateExpression(const Expression* expr);
    void generateStatement(const Statement* stmt);

    void push(const std::string& reg) {
        m_Output += "push " + reg + "\n";
        m_StackSize++;
    }

    void pop(const std::string& reg) {
        m_Output += "pop " + reg + "\n";
        m_StackSize--;
    }

    const Program* m_Program;
    std::string m_Output;
    size_t m_StackSize = 0;
    std::unordered_map<std::string, Variable> m_Variables;
};

} // namespace Glassy
