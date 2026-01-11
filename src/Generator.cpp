#include "pch.h"
#include "Generator.h"

namespace Compiler {

Generator::Generator(Program* prog) : m_Program(prog) {}

std::string Generator::GenerateAsm() {
    m_StackSize = 0;

    m_Output += "global _start\nsection .text\n_start:\n";
    GenerateBlock(m_Program->GlobalBlock);
    m_Output += "mov rax, 60\nxor rdi, rdi\nsyscall\n";

    return m_Output;
}

void Generator::GenerateFactor(const Factor* factor) {
    std::visit(overloaded{ [&](int i) {
                              m_Output += "mov rax, " + std::to_string(i) + "\n";
                              Push("rax");
                          },
                   [&](const std::string& s) {
                       const Variable* v = Lookup(s);

                       if (!v) {
                           Error("Undeclared variable '" + s + "'");
                       }
                       Push("QWORD [rsp + " + std::to_string((m_StackSize - v->StackLocation - 1) * 8) + "]");
                   },
                   [&](const Expression* expr) { GenerateExpression(expr); } },
        factor->Value);
}

void Generator::GenerateTerm(const Term* term) {
    GenerateFactor(term->Left);
    for (const auto& [op, right] : term->Right) {
        GenerateFactor(right);
        Pop("rax");
        Pop("rbx");

        if (op == "*") {
            m_Output += "mul rbx\n";
        } else if (op == "/") {
            m_Output += "div rbx\n";
        } else {
            Error(std::format("Unknow operator '{}'", op));
        }
        Push("rax");
    }
}

void Generator::GenerateExpression(const Expression* expr) {
    GenerateTerm(expr->Left);
    for (const auto& [op, right] : expr->Right) {
        GenerateTerm(right);
        Pop("rax");
        Pop("rbx");
        
        if (op == "+") {
            m_Output += "add rax, rbx\n";
        } else if (op == "-") {
            m_Output += "sub rax, rbx\n";
        } else {
            Error(std::format("Unknow operator '{}'", op));
        }

        Push("rax");
    }
}

void Generator::GenerateBlockItem(const BlockItem* blockItem) {
    std::visit(overloaded{ [&](const Statement* stmt) { GenerateStatement(stmt); },
                   [&](const Declaration* decl) {
                       auto& scope = m_Scopes.back();

                       if (scope.contains(decl->Ident)) {
                           Error("Redefinition of identifier: " + decl->Ident);
                       }
                       scope.emplace(decl->Ident, Variable{ m_StackSize - 1 });
                   } },
        blockItem->Item);
}

void Generator::GenerateBlock(const Block* scope) {
    BeginScope();
    for (const auto& item : scope->Items) {
        GenerateBlockItem(item);
    }
    EndScope();
}

void Generator::GenerateStatement(const Statement* stmt) {
    std::visit(overloaded{ [&](const AssignmentStatement* assignStmt) {
                              const Variable* v = Lookup(assignStmt->Ident);

                              if (!v) {
                                  Error("Undeclared identifier: " + assignStmt->Ident);
                              }
                              GenerateExpression(assignStmt->Expr);
                              Pop("rax");
                              m_Output += "mov [rsp + " +
                                  std::to_string((m_StackSize - v->StackLocation - 1) * 8) + "], rax\n";
                          },
                   [&](const IfStatement* ifStmt) {
                       GenerateExpression(ifStmt->Cond);
                       Pop("rax");
                       const std::string label = CreateLabel();
                       m_Output += "test rax, rax\n";
                       m_Output += "jz " + label + "\n";
                       GenerateStatement(ifStmt->Then);
                       m_Output += label + ":\n";
                   },
                   [&](const Block* scope) { GenerateBlock(scope); } },
        stmt->Stmt);
}

} // namespace Compiler
