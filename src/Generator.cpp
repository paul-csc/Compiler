#include "pch.h"
#include "Generator.h"

namespace Glassy {

Generator::Generator(Program* prog) : m_Program(prog) {}

std::string Generator::GenerateAsm() {
    m_StackSize = 0;
    m_HasExit = false;

    beginScope(); // global scope

    m_Output += "global _start\nsection .text\n_start:\n";

    for (const auto& stmt : m_Program->statements) {
        generateStatement(stmt);
    if (!m_HasExit) {
        endScope();
        m_Output += "mov rax, 60\nxor rdi, rdi\nsyscall\n";
    }

    return m_Output;
}

// clang-format off
void Generator::generateTerm(const Term* term) {
    std::visit(overloaded{
        [&](const TermIdentifier* termIdent) {
            const Variable* v = lookup(termIdent->identifier);
            
            if (!v) {
                Error("Undeclared variable '" + termIdent->identifier + "'");
            }
            push("QWORD [rsp + " + std::to_string((m_StackSize - v->stackLocation - 1) * 8) + "]");
        },
        [&](const TermLiteral* termLiteral) {
            m_Output += "mov rax, " + termLiteral->literal + "\n";
            push("rax");
        },
        [&](const TermParen* termParen) {
            generateExpression(termParen->expr);
        }
    }, term->term);
}

void Generator::generateExpression(const Expression* expr) {
    std::visit(overloaded{
        [&](const Term* term) {
            generateTerm(term);
        },
        [&](const ExprBinary* exprBinary) {
            generateExpression(exprBinary->right);
            generateExpression(exprBinary->left);
            pop("rax");
            pop("rbx");

            switch (exprBinary->op) {
                case '+': m_Output += "add rax, rbx\n"; break;
                case '-': m_Output += "sub rax, rbx\n"; break;
                case '*': m_Output += "mul rbx\n"; break;
                case '/': m_Output += "div rbx\n"; break;
                default: Error(std::format("Unknow operator '{}'", exprBinary->op)); break;
            }

            push("rax");
        }
    }, expr->expr);
}

void Generator::generateStatement(const Statement* stmt) {
    std::visit(overloaded{
        [&](const StmtExit* stmtExit) {
            generateExpression(stmtExit->expr);
            endScope();
            m_Output += "mov rax, 60\n";
            pop("rdi");
            m_Output += "syscall\n";

            m_HasExit = true;
        },
        [&](const StmtDeclar* stmtDeclar) {
            auto& scope = m_Scopes.back();

            if (scope.contains(stmtDeclar->identifier)) {
                Error("Redefinition of identifier: " + stmtDeclar->identifier);
            }
            generateExpression(stmtDeclar->expr);
            scope.emplace(stmtDeclar->identifier, Variable{ m_StackSize - 1  });
        },
        [&](const StmtAssign* stmtAssign) {
            const Variable* v = lookup(stmtAssign->identifier);

            if (!v) {
                Error("Undeclared identifier: " + stmtAssign->identifier);
            }
            generateExpression(stmtAssign->expr);
            pop("rax");
            m_Output +=
                "mov [rsp + " + std::to_string((m_StackSize - v->stackLocation - 1) * 8) + "], rax\n";
        },
        [&](const Scope* scope) {
            beginScope();
            for (const auto& stmt : scope->statements) {
                generateStatement(stmt);
            }
            endScope();
        }
    }, stmt->stmt);
}
// clang-format on

} // namespace Glassy
