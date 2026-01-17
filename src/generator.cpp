#include "generator.h"
#include "symbol_table.h"
#include "utils.h"
#include <format>

namespace Compiler {

Generator::Generator(Program* prog, ScopeStack& scopes) : m_Program(prog), m_Scopes(scopes) {}

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

std::string Generator::CreateLabel() {
    return "label" + std::to_string(m_LabelCount++);
}

void Generator::DebugPrint(const std::string& reg) {
    m_Output += "mov rdi, " + reg + "\n";
    m_Output += "call print\n";
}

void Generator::GeneratePrimary(const Primary* primary) {
    std::visit(overloaded{ [&](int64_t i) {
                              m_Output += "mov rax, " + std::to_string(i) + "\n";
                              Push("rax");
                          },
                   [&](const std::string& s) {
                       auto& v = m_Scopes.Lookup(s);
                       Push("QWORD [rsp + " + std::to_string((m_StackSize - v.StackOffset - 1) * 8) + "]");
                   },
                   [&](const Expression* expr) { GenerateExpression(expr); } },
        primary->Value);
}

void Generator::GeneratePostfixExpression(const PostfixExpression* expr) {
    GeneratePrimary(expr->Prim);
}

void Generator::GenerateMultiplicativeExpression(const MultiplicativeExpression* expr) {
    GeneratePostfixExpression(expr->Left);
    for (const auto& [op, right] : expr->Right) {
        GeneratePostfixExpression(right);
        Pop("rcx");
        Pop("rax");

        if (op == BinaryOp::Mul) {
            m_Output += "imul rax, rcx\n";
            Push("rax");
        } else if (op == BinaryOp::Div || op == BinaryOp::Mod) {
            m_Output += "cqo\n";
            m_Output += "idiv rcx\n";
            if (op == BinaryOp::Div) {
                Push("rax");
            } else {
                Push("rdx");
            }
        } else {
            Error("Unknown operator");
        }
    }
}

void Generator::GenerateAdditiveExpression(const AdditiveExpression* expr) {
    GenerateMultiplicativeExpression(expr->Left);
    for (const auto& [op, right] : expr->Right) {
        GenerateMultiplicativeExpression(right);
        Pop("rax");
        Pop("rbx");

        if (op == BinaryOp::Add) {
            m_Output += "add rbx, rax\n";
            Push("rbx");
        } else if (op == BinaryOp::Sub) {
            m_Output += "sub rbx, rax\n";
            Push("rbx");
        } else {
            Error("Unknown operator");
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
        if (op == BinaryOp::Gt) {
            m_Output += "setg al\n";
        } else if (op == BinaryOp::Ge) {
            m_Output += "setge al\n";
        } else if (op == BinaryOp::Lt) {
            m_Output += "setl al\n";
        } else if (op == BinaryOp::Le) {
            m_Output += "setle al\n";
        } else {
            Error("Unknown operator");
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
        if (op == BinaryOp::Eq) {
            m_Output += "sete al\n";
        } else if (op == BinaryOp::Ne) {
            m_Output += "setne al\n";
        } else {
            Error("Unknown operator");
        }
        m_Output += "movzx rax, al\n";
        Push("rax");
    }
}

void Generator::GenerateExpression(const Expression* expr) {
    if (expr->Expr->Ident) { // assignment
        auto& v = m_Scopes.Lookup(*expr->Expr->Ident);

        GenerateEqualityExpression(expr->Expr->Expr);
        Pop("rax");

        m_Output += "mov [rsp + " + std::to_string((m_StackSize - v.StackOffset - 1) * 8) + "], rax\n";
        DebugPrint("rax");
    } else {
        GenerateEqualityExpression(expr->Expr->Expr);
        Pop("rax");
    }
}

void Generator::GenerateBlock(const Block* scope) {
    m_Scopes.EnterScope();

    for (const auto& item : scope->Items) {
        std::visit(overloaded{ [&](const Statement* stmt) { GenerateStatement(stmt); },
                       [&](const Declaration* decl) {
                           m_Scopes.Insert(decl->Ident, { VARIABLE, m_StackSize - 1 });

                           m_Output += "sub rsp, 8\n";
                           m_StackSize++;
                       } },
            item->Item);
    }

    size_t popCount = m_Scopes.ExitScope();
    if (popCount != 0) {
        m_Output += "add rsp, " + std::to_string(popCount * 8) + "\n";
    }
    m_StackSize -= popCount;
}

void Generator::GenerateStatement(const Statement* stmt) {
    std::visit(overloaded{ [&](const ExpressionStatement* exprStmt) { GenerateExpression(exprStmt->Expr); },
                   [&](const ReturnStatement* retStmt) {
                       if (retStmt->Expr) {
                           GenerateExpression(retStmt->Expr);
                           Pop("rdi");
                       } else {
                           m_Output += "xor rdi, rdi\n";
                       }
                       m_Output += "mov rax, 60\n";
                       m_Output += "syscall\n";
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