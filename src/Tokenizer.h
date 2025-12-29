#pragma once

#include "pch.h"

namespace Compiler {

    template<typename... Args>
    static void Error(Args&&... args) {
        (std::cerr << ... << std::forward<Args>(args)) << "\n";
        std::cin.get();
        std::exit(1);
    }

    enum class TokenType { IDENTIFIER, KEYWORD, SEPARATOR, OPERATOR, LITERAL };

    enum class Keyword { RETURN, IF, ELSE, WHILE, FOR };  // temp
    enum class Separator { L_PAREN, R_PAREN, L_BRACKET, R_BRACKET, L_BRACE, R_BRACE, SEMI };
    enum class Operator { PLUS, MINUS, STAR, SLASH, PERCENT, CARET, EQUAL };

    using SubType = std::variant<Keyword, Separator, Operator>;

    struct Token {
        explicit Token(TokenType t, const std::string_view& lex) : type(t), lexeme(lex) { }

        explicit Token(double val, const std::string_view& lex)
          : type(TokenType::LITERAL),
            lexeme(lex),
            value(val) { }

        explicit Token(Operator op, const std::string_view& lex)
          : type(TokenType::OPERATOR),
            subType(op),
            lexeme(lex) { }

        explicit Token(Separator sep, const std::string_view& lex)
          : type(TokenType::SEPARATOR),
            subType(sep),
            lexeme(lex) { }

        const TokenType type;
        const std::optional<SubType> subType = std::nullopt;
        const std::string lexeme;
        const double value = 0.0;  // valid if type == LITERAL
        unsigned int line;
    };

    class Tokenizer {
      public:
        explicit Tokenizer(const std::string& src) : m_Src(src) { }

        std::vector<Token> Tokenize() const {
            const size_t size = m_Src.size();

            std::vector<Token> tokens;
            std::string buf;

            for (size_t i = 0; i < size; ++i) {
                const char c = m_Src[i];

                if (std::isspace(static_cast<unsigned char>(c))) {
                    continue;
                } else if (std::isalpha(static_cast<unsigned char>(c))) {
                    while (i < size && std::isalnum(m_Src[i])) {
                        buf.push_back(m_Src[i++]);
                    }
                    --i;

                    tokens.emplace_back(TokenType::IDENTIFIER, buf);
                    buf.clear();
                    continue;
                } else if (c == '.' || std::isdigit(static_cast<unsigned char>(m_Src[i]))) {
                    bool hasDot = false;

                    while (
                      i < size
                      && (m_Src[i] == '.' || std::isdigit(static_cast<unsigned char>(m_Src[i])))) {
                        if (m_Src[i] == '.') {
                            if (hasDot) {
                                Error("More than 1 dot in literal");
                            }
                            hasDot = true;
                        }
                        buf.push_back(m_Src[i++]);
                    }
                    --i;

                    try {
                        tokens.emplace_back(std::stod(buf), buf);
                    } catch (...) {
                        Error("Failed to parse token '", buf, "'");
                    }

                    buf.clear();
                    continue;
                }

                switch (c) {
                case '+': tokens.emplace_back(Operator::PLUS, "+"); break;
                case '-': tokens.emplace_back(Operator::MINUS, "-"); break;
                case '*': tokens.emplace_back(Operator::STAR, "*"); break;
                case '/': tokens.emplace_back(Operator::SLASH, "/"); break;
                case '%': tokens.emplace_back(Operator::PERCENT, "%"); break;
                case '^': tokens.emplace_back(Operator::CARET, "^"); break;
                case '=': tokens.emplace_back(Operator::EQUAL, "="); break;

                case '(': tokens.emplace_back(Separator::L_PAREN, "("); break;
                case ')': tokens.emplace_back(Separator::R_PAREN, ")"); break;
                case ';': tokens.emplace_back(Separator::SEMI, ";"); break;
                default : std::string msg = "unrecognized token '" + std::string(1, c) + "'";
                }
            }

            return tokens;
        }

      private:
        const std::string m_Src;
    };

}  // namespace Compiler