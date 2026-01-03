#include "pch.h"
#include "Generator.h"

namespace Glassy {

Generator::Generator(Program* prog) : m_Program(prog) {}

std::string Generator::GenerateAsm() {
    m_StackSize = 0;

    m_Output += "global _start\nsection .text\n_start:\n";

    for (const auto& stmt : m_Program->statements) {
        generateStatement(stmt);
    }

    m_Output += "mov rax, 60\nmov rdi, 0\nsyscall\n";
    return m_Output;
}

// clang-format off
void Generator::generateTerm(const Term* term) {
    std::visit(overloaded{
        [&](const TermIdentifier* termIdent) {
            if (!m_Variables.contains(termIdent->identifier)) {
                Error("Undeclared variable '" + termIdent->identifier + "'");
            }
            const auto& var = m_Variables[termIdent->identifier];
            push("QWORD [rsp + " + std::to_string((m_StackSize - var.stackLocation - 1) * 8) + "]");
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
            m_Output += "mov rax, 60\n";
            pop("rdi");
            m_Output += "syscall\n";
        },
        [&](const StmtLet* stmtLet) {
            if (m_Variables.contains(stmtLet->identifier)) {
                Error("Identifier already used: " + stmtLet->identifier);
            }
            generateExpression(stmtLet->expr);
            m_Variables.insert({ stmtLet->identifier, { m_StackSize - 1 } });
        },
        [&](const StmtAssign* stmtAssign) {
            if (!m_Variables.contains(stmtAssign->identifier)) {
                Error("Undeclared identifier: " + stmtAssign->identifier);
            }
            generateExpression(stmtAssign->expr);
            pop("rax");
            const auto& var = m_Variables[stmtAssign->identifier];
            m_Output +=
                "mov [rsp + " + std::to_string((m_StackSize - var.stackLocation - 1) * 8) + "], rax\n";
        }
    }, stmt->stmt);
    // clang-format on
}

} // namespace Glassy
