#include "pch.h"
#include "Generator.h"

namespace Compiler {

Generator::Generator(Program* prog) : m_Program(prog) {}

std::string Generator::GenerateAsm() {
    m_StackSize = 0;
    m_HasExit = false;

    BeginScope(); // global scope

    m_Output += "global _start\nsection .text\n_start:\n";

    for (const auto& stmt : m_Program->Statements) {
        GenerateStatement(stmt);
    }

    if (!m_HasExit) {
        EndScope();
        m_Output += "mov rax, 60\nxor rdi, rdi\nsyscall\n";
    }

    return m_Output;
}

// clang-format off
void Generator::GenerateTerm(const Term* term) {
    std::visit(overloaded{
        [&](const TermIdentifier* termIdent) {
            const Variable* v = Lookup(termIdent->Identifier);
            
            if (!v) {
                Error("Undeclared variable '" + termIdent->Identifier + "'");
            }
            Push("QWORD [rsp + " + std::to_string((m_StackSize - v->StackLocation - 1) * 8) + "]");
        },
        [&](const TermLiteral* termLiteral) {
            m_Output += "mov rax, " + termLiteral->Literal + "\n";
            Push("rax");
        },
        [&](const TermParen* termParen) {
            GenerateExpression(termParen->Expr);
        }
    }, term->Term);
}

void Generator::GenerateExpression(const Expression* expr) {
    std::visit(overloaded{
        [&](const Term* term) {
            GenerateTerm(term);
        },
        [&](const ExprBinary* exprBinary) {
            GenerateExpression(exprBinary->Right);
            GenerateExpression(exprBinary->Left);
            Pop("rax");
            Pop("rbx");

            switch (exprBinary->Op) {
                case '+': m_Output += "add rax, rbx\n"; break;
                case '-': m_Output += "sub rax, rbx\n"; break;
                case '*': m_Output += "mul rbx\n"; break;
                case '/': m_Output += "div rbx\n"; break;
                default: Error(std::format("Unknow operator '{}'", exprBinary->Op)); break;
            }

            Push("rax");
        }
    }, expr->Expr);
}

void Generator::GenerateScope(const Scope* scope) {   
    BeginScope();
    for (const auto& stmt : scope->Statements) {
        GenerateStatement(stmt);
    }
    EndScope();
}

void Generator::GenerateStatement(const Statement* stmt) {
    std::visit(overloaded{
        [&](const StmtExit* stmtExit) {
            GenerateExpression(stmtExit->Expr);
            EndScope();
            m_Output += "mov rax, 60\n";
            Pop("rdi");
            m_Output += "syscall\n";

            m_HasExit = true;
        },
        [&](const StmtDeclar* stmtDeclar) {
            auto& scope = m_Scopes.back();

            if (scope.contains(stmtDeclar->Identifier)) {
                Error("Redefinition of identifier: " + stmtDeclar->Identifier);
            }
            GenerateExpression(stmtDeclar->Expr);
            scope.emplace(stmtDeclar->Identifier, Variable{ m_StackSize - 1  });
        },
        [&](const StmtAssign* stmtAssign) {
            const Variable* v = Lookup(stmtAssign->identifier);

            if (!v) {
                Error("Undeclared identifier: " + stmtAssign->identifier);
            }
            GenerateExpression(stmtAssign->expr);
            Pop("rax");
            m_Output +=
                "mov [rsp + " + std::to_string((m_StackSize - v->StackLocation - 1) * 8) + "], rax\n";
        },
        [&](const StmtIf* stmtIf) {
            GenerateExpression(stmtIf->Cond);
            Pop("rax");
            const std::string label = CreateLabel();
            m_Output += "test rax, rax\n";
            m_Output += "jz " + label + "\n";
            GenerateScope(stmtIf->Scope);
            m_Output += label + ":\n";
        },
        [&](const Scope* scope) {
            GenerateScope(scope);
        }
    }, stmt->Stmt);
}
// clang-format on

} // namespace Compiler
