#include "pch.h"
#include "generator.h"

namespace Compiler {

Generator::Generator(Program* prog) : m_Program(prog) {}

std::string Generator::GenerateAsm() {
    m_StackSize = 0;

    m_Output += "global _start\nsection .text\nextern print\n_start:\n";
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
                   [&](const AdditiveExpression* expr) { GenerateExpression(expr); } },
        factor->Value);
}

void Generator::GenerateMultiplicativeExpression(const MultiplicativeExpression* expr) {
    GenerateFactor(expr->Left);
    for (const auto& [op, right] : expr->Right) {
        GenerateFactor(right);
        Pop("rax");
        Pop("rbx");

        if (op == "*") {
            m_Output += "mul rbx\n";
        } else if (op == "/") {
            m_Output += "xor rdx, rdx\n";
            m_Output += "div rbx\n";
        } else {
            Error(std::format("Unknow operator '{}'", op));
        }
        Push("rax");
    }
}

void Generator::GenerateExpression(const AdditiveExpression* expr) {
    GenerateMultiplicativeExpression(expr->Left);
    for (const auto& [op, right] : expr->Right) {
        GenerateMultiplicativeExpression(right);
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

void Generator::GenerateBlock(const Block* scope) {
    BeginScope();
    for (const auto& item : scope->Items) {
        std::visit(overloaded{ [&](const Statement* stmt) { GenerateStatement(stmt); },
                       [&](const Declaration* decl) {
                           auto& scope = m_Scopes.back();

                           if (scope.contains(decl->Ident)) {
                               Error("Redefinition of identifier: " + decl->Ident);
                           }
                           scope.emplace(decl->Ident, Variable{ m_StackSize - 1 });
                       } },
            item->Item);
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
                   //    [&](const IfStatement* ifStmt) {
                   //        GenerateExpression(ifStmt->Cond);
                   //        Pop("rax");
                   //        const std::string label = CreateLabel();
                   //        m_Output += "test rax, rax\n";
                   //        m_Output += "jz " + label + "\n";
                   //        GenerateStatement(ifStmt->Then);
                   //        m_Output += label + ":\n";
                   //        if (ifStmt->Else) {
                   //        }
                   //    },
                   [&](const IfStatement* ifStmt) {
                       GenerateExpression(ifStmt->Cond);
                       Pop("rax");

                       const std::string elseLabel = CreateLabel();
                       const std::string endLabel = CreateLabel();

                       m_Output += "test rax, rax\n";
                       m_Output += "jz " + elseLabel + "\n";

                       const int64_t stackBefore = m_StackSize;

                       // then-branch
                       GenerateStatement(ifStmt->Then);
                       const int64_t thenStack = m_StackSize;

                       m_Output += "jmp " + endLabel + "\n";

                       // else-branch
                       m_Output += elseLabel + ":\n";
                       m_StackSize = stackBefore;
                       if (ifStmt->Else) {
                           GenerateStatement(ifStmt->Else);
                       }

                       const int64_t elseStack = m_StackSize;

                       // enforce stack agreement
                       if (thenStack != elseStack) {
                           Error("Stack height mismatch between if branches");
                       }

                       // merged stack height
                       m_StackSize = thenStack;

                       // end
                       m_Output += endLabel + ":\n";
                   },
                   [&](const Block* scope) { GenerateBlock(scope); } },
        stmt->Stmt);
}

} // namespace Compiler
