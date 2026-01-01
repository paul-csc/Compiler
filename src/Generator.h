#pragma once

#include "Parser.h"
#include <unordered_map>

namespace Glassy {

struct Generator : public AstVisitor {
  public:
    explicit Generator(std::unique_ptr<Program> prog);
    std::string GenerateAsm();

  private:
    struct Variable {
        size_t stackLocation;
    };

    void visit(const LiteralExpr& node) override;
    void visit(const IdentifierExpr& node) override;
    void visit(const BinaryExpr& node) override;

    void visit(const AssignStmt& stmt) override;
    void visit(const DeclarStmt& stmt) override;
    void visit(const ExitStmt& stmt) override;
    void visit(const Program& prog) override;

    void push(const std::string& reg) {
        m_Output += "push " + reg + "\n";
        m_StackSize++;
    }

    void pop(const std::string& reg) {
        m_Output += "pop " + reg + "\n";
        m_StackSize--;
    }

    const std::unique_ptr<Program> m_Program;
    std::string m_Output;
    size_t m_StackSize = 0;
    std::unordered_map<Identifier, Variable> m_Variables;
};

} // namespace Glassy
