#include "lexer.h"
#include "utils.h"
#include <format>
#include <unordered_map>

namespace Compiler {

static const std::unordered_map<std::string_view, TokenType> keywords{ { "return", RETURN }, { "int", INT },
    { "if", IF }, { "else", ELSE }, { "while", WHILE } };

Lexer::Lexer(std::string_view src) : m_Src(src), m_Size(src.size()) {}

std::vector<Token> Lexer::Lex() {
    m_Index = 0;

    std::vector<Token> tokens;

    while (m_Index < m_Size) {
        const char c = m_Src[m_Index];

        if (IsSpace(c)) {
            if (c == '\n') {
                ++m_Index;
                Newline();
            } else {
                Advance();
            }
            continue;
        }

        SourceLocation startLoc = m_Loc;

        if (IsAlpha(c) || c == '_') {
            auto lexeme = LexWhile([&](char ch) { return IsAlnum(ch) || ch == '_'; });

            auto it = keywords.find(lexeme);
            if (it != keywords.end()) {
                tokens.emplace_back(it->second, startLoc);
            } else {
                tokens.emplace_back(IDENTIFIER, startLoc, lexeme);
            }
            continue;
        } else if (IsDigit(c)) {
            auto lexeme = LexWhile([&](char ch) { return IsDigit(ch); });
            tokens.emplace_back(LITERAL, startLoc, lexeme);
            continue;
        }

        switch (c) {
            // operators
            case '+': tokens.emplace_back(PLUS, startLoc); break;
            case '-': tokens.emplace_back(MINUS, startLoc); break;
            case '*': tokens.emplace_back(STAR, startLoc); break;
            case '/':
                if (Match('/')) {
                    while (m_Index < m_Size && m_Src[m_Index] != '\n') {
                        Advance();
                    }
                    Newline();
                } else {
                    tokens.emplace_back(FSLASH, startLoc);
                }
                break;
            case '%': tokens.emplace_back(PERCENT, startLoc); break;
            case '>': tokens.emplace_back(Match('=') ? GE : GT, startLoc); break;
            case '<': tokens.emplace_back(Match('=') ? LE : LT, startLoc); break;
            case '=': tokens.emplace_back(Match('=') ? IS_EQUAL : EQUAL, startLoc); break;
            case '!':
                if (Match('=')) {
                    tokens.emplace_back(NOT_EQUAL, startLoc);
                } else {
                    Error(startLoc, "Unknown token '!'");
                }
                break;

            // separators
            case '(': tokens.emplace_back(LPAREN, startLoc); break;
            case ')': tokens.emplace_back(RPAREN, startLoc); break;
            case '{': tokens.emplace_back(LBRACE, startLoc); break;
            case '}': tokens.emplace_back(RBRACE, startLoc); break;
            case ';': tokens.emplace_back(SEMICOLON, startLoc); break;
            case ',': tokens.emplace_back(COMMA, startLoc); break;

            default: Error(startLoc, std::format("Unknow token '{}'", c));
        }

        Advance();
    }

    tokens.emplace_back(END_OF_FILE, m_Loc);

    return tokens;
}

bool Lexer::IsAlpha(char c) {
    return std::isalpha(static_cast<unsigned char>(c));
}

bool Lexer::IsAlnum(char c) {
    return std::isalnum(static_cast<unsigned char>(c));
}

bool Lexer::IsDigit(char c) {
    return std::isdigit(static_cast<unsigned char>(c));
}

bool Lexer::IsSpace(char c) {
    return std::isspace(static_cast<unsigned char>(c));
}

void Lexer::Advance() {
    ++m_Index;
    ++m_Loc.Column;
}

void Lexer::Newline() {
    ++m_Loc.Line;
    m_Loc.Column = 0;
}

bool Lexer::Match(char expected) {
    if (m_Index + 1 < m_Size && m_Src[m_Index + 1] == expected) {
        Advance();
        return true;
    }
    return false;
}

} // namespace Compiler