#pragma once

#include "pch.h"

namespace Glassy {

enum TokenType {
    IDENTIFIER,
    LITERAL,

    EXIT,
    LET,

    L_PAREN,
    R_PAREN,
    L_BRACKET,
    R_BRACKET,
    L_BRACE,
    R_BRACE,
    SEMI,

    PLUS,
    MINUS,
    STAR,
    SLASH,
    PERCENT,
    CARET,
    EQUAL,

    END_OF_FILE,

    TOKEN_TYPE_NB
};

struct SourceLocation {
    uint16_t line = 1;
    uint16_t column = 1;
};

constexpr const char* TokenToStr[TOKEN_TYPE_NB] = { "identifier", "literal", "exit", "let", "(", ")", "[",
    "]", "{", "}", ";", "+", "-", "*", "/", "%", "^", "=", "eof" };

constexpr const char* ToStr(TokenType type) {
    return TokenToStr[type];
}

struct Token {
    Token(TokenType type, SourceLocation loc) : type(type), location(loc) {}
    Token(TokenType type, SourceLocation loc, std::string_view v) : type(type), location(loc), value(v) {}

    constexpr const char* ToStr() const { return TokenToStr[type]; }

    TokenType type;
    SourceLocation location;
    std::optional<std::string> value = std::nullopt; // literal or identifier
};

class Tokenizer {
  public:
    explicit Tokenizer(std::string_view src);
    std::vector<Token> Tokenize() const;

  private:
    std::string_view m_Src;
};

void Error(SourceLocation loc, const std::string& msg);
void Error(const std::string& msg);

} // namespace Glassy
