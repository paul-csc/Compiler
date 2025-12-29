#pragma once

#include "pch.h"
#include "Tokenizer.h"

namespace Compiler {

    // program     -> statement*
    // statement   -> identifier "=" expression ";"
    // expression  -> term (("+" | "-") term)*
    // term        -> factor (("*" | "/") factor)*
    // factor      -> NUMBER | IDENTIFIER | "(" expression ")"
    // ex:  "var = (3 + 8.5) * .2;"

    struct ASTNode {
        virtual ~ASTNode() = default;
        virtual void print(std::ostream& out, int indent = 0) const = 0;
    };

    struct Expression : public ASTNode { };

    struct NumberExpr : public Expression {
        explicit NumberExpr(double val) : value(val) { }

        double value;
    };

    struct BinaryExpr : public Expression {
        BinaryExpr(char op, std::unique_ptr<Expression> left, std::unique_ptr<Expression> right)
          : op(op),
            left(std::move(left)),
            right(std::move(right)) { }

        char op;
        std::unique_ptr<Expression> left;
        std::unique_ptr<Expression> right;
    };

    struct Statement : public ASTNode { };

    struct AssignStmt : public Statement {
        AssignStmt(const std::string& name, std::unique_ptr<Expression> value)
          : name(name),
            value(std::move(value)) { }

        void print(std::ostream& out, int indent = 0) const override {
            out << std::string(indent, ' ') << "AssignStmt: " << name << "\n";
        }

        std::string name;
        std::unique_ptr<Expression> value;
    };

    struct Program : public ASTNode {
        void print(std::ostream& out, int indent = 0) const override {
            out << "Program:\n";
            for (const auto& stmt : statements) {
                stmt->print(out, indent + 2);
            }
        }

        std::vector<std::unique_ptr<Statement>> statements;
    };

    class Parser {
      public:
        explicit Parser(const std::vector<Token>& tokens) : m_Tokens(tokens) { }

        std::unique_ptr<Expression> ParseExpression() { return nullptr; }

        std::unique_ptr<Statement> ParseStatement() { return nullptr; }

        std::unique_ptr<Program> ParseProgram() {
            std::unique_ptr<Program> program = std::make_unique<Program>();

            while (peek().has_value()) {
                Token name = expect(TokenType::IDENTIFIER, "Expected identifier");
                expect(TokenType::OPERATOR, "Expected '='");
                expect(TokenType::SEPARATOR, "Expected ';'");

                program->statements.push_back(std::make_unique<AssignStmt>(name.lexeme, nullptr));
            }

            return program;
        }

      private:
        [[nodiscard]] std::optional<Token> peek(const int offset = 0) const {
            if (m_Index + offset >= m_Tokens.size()) {
                return std::nullopt;
            }
            return m_Tokens[m_Index + offset];
        }

        Token consume() {
            if (m_Index >= m_Tokens.size()) {
                Error("Unexpected end of input");
            }
            return m_Tokens[m_Index++];
        }

        bool match(TokenType type) {
            auto tok = peek();
            if (!tok || tok->type != type)
                return false;

            consume();
            return true;
        }

        Token expect(TokenType type, const char* msg) {
            auto tok = peek();
            if (!tok || tok->type != type) {
                Error(msg);
            }
            return consume();
        }

        const std::vector<Token> m_Tokens;
        size_t m_Index = 0;
    };

}  // namespace Compiler