#include "Tokenizer.h"

#include <cctype>
#include <charconv>
#include <format>
#include <iostream>

namespace Glassy {

const std::string_view SeparatorToStr[] = { "(", ")", "[", "]", "{", "}", ";" };

const std::string_view OperatorToStr[] = { "+", "-", "*", "/", "%", "^", "=" };

Tokenizer::Tokenizer(std::string_view src) : m_Src(src) {}

std::vector<Token> Tokenizer::Tokenize() const {
    const size_t size = m_Src.size();

    std::vector<Token> tokens;
    SourceLocation loc;

    for (size_t i = 0; i < size;) {
        const char c = m_Src[i];

        if (std::isspace(static_cast<unsigned char>(c))) {
            if (c == '\n') {
                ++loc.line;
                loc.column = 1;
            }
            ++i;
            ++loc.column;

            continue;
        } else if (std::isalpha(static_cast<unsigned char>(c))) {
            const size_t start = i;
            while (i < size && std::isalnum(static_cast<unsigned char>(m_Src[i]))) {
                ++i;
                ++loc.column;
            }

            std::string_view lex(m_Src.data() + start, i - start);

            if (lex == "exit") {
                tokens.emplace_back(Keyword::EXIT, loc, lex);
            } else if (lex == "let") {
                tokens.emplace_back(Keyword::LET, loc, lex);
            } else {
                tokens.emplace_back(lex, loc); // identifier
            }

            continue;
        } else if (c == '.' || std::isdigit(static_cast<unsigned char>(c))) {
            const size_t start = i;
            bool hasDot = false;

            while (i < size && (m_Src[i] == '.' || std::isdigit(static_cast<unsigned char>(m_Src[i])))) {
                if (m_Src[i] == '.') {
                    if (hasDot) {
                        Error(loc, "More than 1 dot in literal");
                    }
                    hasDot = true;
                }
                ++i;
                ++loc.column;
            }

            std::string_view lex(m_Src.data() + start, i - start);

            Literal result;
            auto ec = std::from_chars(lex.data(), lex.data() + lex.size(), result).ec;

            if (ec == std::errc()) {
                tokens.emplace_back(result, loc, lex); // literal
            } else if (ec == std::errc::invalid_argument) {
                Error(loc, std::format("'{}' is not a number", std::string(lex)));
            } else if (ec == std::errc::result_out_of_range) {
                Error(loc, std::format("'{}' is out of range", std::string(lex)));
            }

            continue;
        }

        switch (c) {
            // operators
            case '+': tokens.emplace_back(Operator::PLUS, loc); break;
            case '-': tokens.emplace_back(Operator::MINUS, loc); break;
            case '*': tokens.emplace_back(Operator::STAR, loc); break;
            case '/': tokens.emplace_back(Operator::SLASH, loc); break;
            case '%': tokens.emplace_back(Operator::PERCENT, loc); break;
            case '^': tokens.emplace_back(Operator::CARET, loc); break;
            case '=': tokens.emplace_back(Operator::EQUAL, loc); break;

            // separators
            case '(': tokens.emplace_back(Separator::L_PAREN, loc); break;
            case ')': tokens.emplace_back(Separator::R_PAREN, loc); break;
            case ';': tokens.emplace_back(Separator::SEMI, loc); break;

            default: Error(loc, std::format("Unrecognized token '{}'", c));
        }

        ++i;
        ++loc.column;
    }

    return tokens;
}

void Error(SourceLocation loc, const std::string& msg) {
    std::cerr << std::format("{} [Ln {}, Col {}]\n", msg, loc.line, loc.column);
    std::cin.get();
    std::exit(1);
}

} // namespace Glassy
