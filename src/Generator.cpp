#include "Generator.h"
#include <iostream>

namespace Glassy {

Generator::Generator(std::unique_ptr<Program> prog) : m_Program(std::move(prog)) {}

std::string Generator::GenerateAsm() {
    m_StackSize = 0;
    m_Program->accept(*this);
    return m_Output;
}

void Generator::visit(const LiteralExpr& node) {
    m_Output += "mov rax, " + std::to_string(node.value) + "\n";
}

void Generator::visit(const IdentifierExpr& node) {
    if (!m_Variables.contains(node.name)) {
        Error("Undeclared variable '" + node.name + "'");
    }
    const auto& var = m_Variables[node.name];
    m_Output += "mov rax, QWORD [rsp + " + std::to_string((m_StackSize - var.stackLocation - 1) * 8) + "]\n";
}

void Generator::visit(const BinaryExpr& node) {
    node.left->accept(*this);
    push("rax");
    node.right->accept(*this);
    pop("rbx");

    switch (node.op) {
        case Operator::PLUS: m_Output += "add rax, rbx\n"; break;
        case Operator::MINUS: m_Output += "sub rbx, rax\nmov rax, rbx\n"; break;
        default: break;
    }
}

void Generator::visit(const AssignStmt& stmt) {
    stmt.expr->accept(*this);
    m_Output += "; assign " + stmt.identifier + "\n";
}

void Generator::visit(const DeclarStmt& stmt) {
    if (m_Variables.contains(stmt.identifier)) {
        Error("Identifier already used: " + stmt.identifier);
    }

    stmt.expr->accept(*this);
    push("rax");

    m_Variables.insert({ stmt.identifier, { m_StackSize - 1 } });
}

void Generator::visit(const ExitStmt& stmt) {
    stmt.expr->accept(*this);

    m_Output += "mov rax, 60\n";
    pop("rdi");
    m_Output += "syscall\n";
}

void Generator::visit(const Program& prog) {
    m_Output += "global _start\nsection .text\n_start:\n";

    for (const auto& stmt : prog.statements) {
        stmt->accept(*this);
    }
    
    m_Output += "mov rax, 60\nmov rdi, 0\nsyscall\n";
}

} // namespace Glassy
