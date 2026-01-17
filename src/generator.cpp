#include "generator.h"
#include "utils.h"
#include <format>

namespace Compiler {

Generator::Generator(Program* prog) : m_Program(prog) {}

std::string Generator::GenerateAsm() {
    m_StackSize = 0;

    m_Output += "global _start\nsection .text\nextern print\n_start:\n";
    GenerateBlock(m_Program->GlobalBlock);
    m_Output += "mov rax, 60\nxor rdi, rdi\nsyscall\n";

    return m_Output;
}

void Generator::Push(const std::string& reg) {
    m_Output += "push " + reg + "\n";
    m_StackSize++;
}

void Generator::Pop(const std::string& reg) {
    if (m_StackSize <= 0) {
        Error("Stack underflow");
    }
    m_Output += "pop " + reg + "\n";
    m_StackSize--;
}

void Generator::BeginScope() {
    m_Scopes.emplace_back();
}

void Generator::EndScope() {
    size_t popCount = m_Scopes.back().size();
    if (popCount != 0) {
        m_Output += "add rsp, " + std::to_string(popCount * 8) + "\n";
    }
    m_StackSize -= popCount;
    m_Scopes.pop_back();
}

Generator::Variable* Generator::LookupVar(const std::string& name) {
    for (auto it = m_Scopes.rbegin(); it != m_Scopes.rend(); ++it) {
        auto found = it->find(name);
        if (found != it->end()) {
            return &found->second;
        }
    }
    return nullptr;
}

std::string Generator::CreateLabel() {
    return "label" + std::to_string(m_LabelCount++);
}

void Generator::DebugPrint(const std::string& reg) {
    m_Output += "mov rdi, " + reg + "\n";
    m_Output += "call print\n";
}

void Generator::GeneratePrimary(const Primary* primary) {
    std::visit(overloaded{ [&](int i) {
                              m_Output += "mov rax, " + std::to_string(i) + "\n";
                              Push("rax");
                          },
                   [&](const std::string& s) {
                       const Variable* v = LookupVar(s);

                       if (!v) {
                           Error("Undeclared variable '" + s + "'");
                       }
                       Push("QWORD [rsp + " + std::to_string((m_StackSize - v->StackLocation - 1) * 8) + "]");
                   },
                   [&](const Expression* expr) { GenerateExpression(expr); } },
        primary->Value);
}

void Generator::GenerateMultiplicativeExpression(const MultiplicativeExpression* expr) {
    GeneratePrimary(expr->Left);
    for (const auto& [op, right] : expr->Right) {
        GeneratePrimary(right);
        Pop("rax");
        Pop("rbx");

        if (op == "*") {
            m_Output += "imul rax, rbx\n";
        } else if (op == "/") {
            m_Output += "mov rcx, rax\n";
            m_Output += "mov rax, rbx \n";
            m_Output += "xor rdx, rdx\n";
            m_Output += "div rcx\n";
        } else {
            Error(std::format("Unknow operator '{}'", op));
        }
        Push("rax");
    }
}

void Generator::GenerateAdditiveExpression(const AdditiveExpression* expr) {
    GenerateMultiplicativeExpression(expr->Left);
    for (const auto& [op, right] : expr->Right) {
        GenerateMultiplicativeExpression(right);
        Pop("rax");
        Pop("rbx");

        if (op == "+") {
            m_Output += "add rax, rbx\n";
            Push("rax");
        } else if (op == "-") {
            m_Output += "sub rbx, rax\n";
            Push("rbx");
        } else {
            Error(std::format("Unknow operator '{}'", op));
        }
    }
}

void Generator::GenerateRelationalExpression(const RelationalExpression* expr) {
    GenerateAdditiveExpression(expr->Left);
    for (const auto& [op, right] : expr->Right) {
        GenerateAdditiveExpression(right);
        Pop("rax");
        Pop("rbx");

        m_Output += "cmp rbx, rax\n";
        if (op == ">") {
            m_Output += "setg al\n";
        } else if (op == ">=") {
            m_Output += "setge al\n";
        } else if (op == "<") {
            m_Output += "setl al\n";
        } else if (op == "<=") {
            m_Output += "setle al\n";
        } else {
            Error(std::format("Unknow operator '{}'", op));
        }
        m_Output += "movzx rax, al\n";
        Push("rax");
    }
}

void Generator::GenerateEqualityExpression(const EqualityExpression* expr) {
    GenerateRelationalExpression(expr->Left);
    for (const auto& [op, right] : expr->Right) {
        GenerateRelationalExpression(right);
        Pop("rax");
        Pop("rbx");

        m_Output += "cmp rbx, rax\n";
        if (op == "==") {
            m_Output += "sete al\n";
        } else if (op == "!=") {
            m_Output += "setne al\n";
        } else {
            Error(std::format("Unknow operator '{}'", op));
        }
        m_Output += "movzx rax, al\n";
        Push("rax");
    }
}

void Generator::GenerateExpression(const Expression* expr) {
    GenerateEqualityExpression(expr->Expr);
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

                           m_Output += "sub rsp, 8\n";
                           m_StackSize++;

                           scope.emplace(decl->Ident, Variable{ m_StackSize - 1 });
                       } },
            item->Item);
    }
    EndScope();
}

void Generator::GenerateStatement(const Statement* stmt) {
    std::visit(overloaded{ [&](const AssignmentStatement* assignStmt) {
                              const Variable* v = LookupVar(assignStmt->Ident);

                              if (!v) {
                                  Error("Undeclared identifier: " + assignStmt->Ident);
                              }
                              GenerateExpression(assignStmt->Expr);
                              Pop("rax");
                              m_Output += "mov [rsp + " +
                                  std::to_string((m_StackSize - v->StackLocation - 1) * 8) + "], rax\n";

                              DebugPrint("rax");
                          },
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
                   [&](const WhileStatement* whilStmt) {
                       const std::string startLabel = CreateLabel();
                       const std::string endLabel = CreateLabel();

                       m_Output += startLabel + ":\n";

                       GenerateExpression(whilStmt->Cond);
                       Pop("rax");

                       m_Output += "test rax, rax\n";
                       m_Output += "jz " + endLabel + "\n";

                       const int64_t stackBefore = m_StackSize;

                       GenerateStatement(whilStmt->Loop);

                       m_Output += "jmp " + startLabel + "\n";
                       m_Output += endLabel + ":\n";

                       m_StackSize = stackBefore;
                   },
                   [&](const Block* scope) { GenerateBlock(scope); } },
        stmt->Stmt);
}

} // namespace Compiler
