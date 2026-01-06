#include "pch.h"
#include "Lexer.h"

namespace Compiler {

std::vector<Token> Lexer::Lex(std::string_view src) {
    const size_t size = src.size();

    std::vector<Token> tokens;
    SourceLocation loc;

    for (size_t i = 0; i < size;) {
        const char c = src[i];

        if (std::isspace(static_cast<unsigned char>(c))) {
            if (c == '\n') {
                ++loc.Line;
                loc.Column = 0;
            }

            ++i;
            ++loc.Column;
            continue;
        }

        SourceLocation startLoc = loc;

        if (std::isalpha(static_cast<unsigned char>(c)) || c == '_') {
            const size_t start = i;

            while (i < size && (std::isalnum(static_cast<unsigned char>(src[i])) || src[i] == '_')) {
                ++i;
                ++loc.Column;
            }

            std::string_view lexeme(src.data() + start, i - start);

            if (lexeme == ToStr(RETURN)) {
                tokens.emplace_back(RETURN, startLoc);
            } else if (lexeme == ToStr(INT)) {
                tokens.emplace_back(INT, startLoc);
            } else if (lexeme == ToStr(IF)) {
                tokens.emplace_back(IF, startLoc);
            } else {
                tokens.emplace_back(IDENTIFIER, startLoc, lexeme);
            }
            continue;
        } else if (std::isdigit(static_cast<unsigned char>(c))) {
            const size_t start = i;

            while (i < size && (std::isdigit(static_cast<unsigned char>(src[i])))) {
                ++i;
                ++loc.Column;
            }

            std::string_view lexeme(src.data() + start, i - start);
            tokens.emplace_back(LITERAL, startLoc, lexeme);
            continue;
        }

        switch (c) {
            // operators
            case '+': tokens.emplace_back(PLUS, startLoc); break;
            case '-': tokens.emplace_back(MINUS, startLoc); break;
            case '*': tokens.emplace_back(STAR, startLoc); break;
            case '/': tokens.emplace_back(F_SLASH, startLoc); break;
            case '%': tokens.emplace_back(PERCENT, startLoc); break;
            case '^': tokens.emplace_back(CARET, startLoc); break;
            case '=': tokens.emplace_back(EQUAL, startLoc); break;

            // separators
            case '(': tokens.emplace_back(L_PAREN, startLoc); break;
            case ')': tokens.emplace_back(R_PAREN, startLoc); break;
            case '{': tokens.emplace_back(L_BRACE, startLoc); break;
            case '}': tokens.emplace_back(R_BRACE, startLoc); break;
            case ';': tokens.emplace_back(SEMICOLON, startLoc); break;
            case ',': tokens.emplace_back(COMMA, startLoc); break;

            // comments
            case '#':
                while (src[++i] != '\n')
                    ;
                ++loc.Line;
                break;

            default: Error(startLoc, std::format("Unknow token '{}'", c));
        }

        ++i;
        ++loc.Column;
    }

    tokens.emplace_back(END_OF_FILE, loc);

    return tokens;
}

void Error(SourceLocation loc, const std::string& msg) {
    Error(std::format("{} [Ln {}, Col {}]", msg, loc.Line, loc.Column));
}

void Error(const std::string& msg) {
    std::cerr << msg << "\n";
    std::cin.get();
    std::exit(1);
}

} // namespace Compiler