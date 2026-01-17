#pragma once

#include <array>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace Compiler {

enum TokenType {
    IDENTIFIER,
    LITERAL,

    RETURN,
    INT,
    IF,
    ELSE,
    WHILE,

    LPAREN,
    RPAREN,
    LBRACE,
    RBRACE,
    SEMICOLON,
    COMMA,

    PLUS,
    MINUS,
    STAR,
    FSLASH,
    PERCENT,
    GT,
    GE,
    LT,
    LE,
    IS_EQUAL,
    NOT_EQUAL,
    EQUAL,

    END_OF_FILE,

    TOKEN_TYPE_NB
};

struct SourceLocation {
    uint16_t Line = 1;
    uint16_t Column = 1;
};

constexpr std::array<std::string_view, TOKEN_TYPE_NB> TokenNames = { "identifier", "literal", "return", "int",
    "if", "else", "while", "(", ")", "{", "}", ";", ",", "+", "-", "*", "/", "%", ">", ">=", "<",
    "<=", "==", "!=", "=", "eof" };

constexpr std::string_view TokenToStr(TokenType type) {
    return TokenNames.at(type);
}

struct Token {
    Token(TokenType type, SourceLocation loc) : Type(type), Location(loc) {}
    Token(TokenType type, SourceLocation loc, std::string_view v) : Type(type), Location(loc), Value(v) {}

    TokenType Type;
    SourceLocation Location;
    std::optional<std::string> Value = std::nullopt; // literal or identifier
};

class Lexer {
  public:
    Lexer(std::string_view src);
    std::vector<Token> Lex();

  private:
    static bool IsAlpha(char c);
    static bool IsAlnum(char c);
    static bool IsDigit(char c);
    static bool IsSpace(char c);

    void Advance();
    void Newline();

    template <typename Predicate>
    std::string_view LexWhile(Predicate predicate) {
        const size_t start = m_Index;
        while (m_Index < m_Size && predicate(m_Src[m_Index])) {
            Advance();
        }
        return std::string_view(m_Src.data() + start, m_Index - start);
    }

    bool Match(char expected);

    const std::string_view m_Src;
    const size_t m_Size;
    size_t m_Index;
    SourceLocation m_Loc;
};

} // namespace Compiler
