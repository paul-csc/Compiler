#pragma once

#include "pch.h"

namespace Glassy {

enum TokenType {
    IDENTIFIER,
    LITERAL,

    EXIT,
    VAR,

    L_PAREN,
    R_PAREN,
    L_BRACE,
    R_BRACE,
    SEMI,

    PLUS,
    MINUS,
    STAR,
    F_SLASH,
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

constexpr std::string_view TokenToStr[TOKEN_TYPE_NB] = { "identifier", "literal", "exit", "var", "(", ")",
    "{", "}", ";", "+", "-", "*", "/", "%", "^", "=", "eof" };

constexpr std::string_view ToStr(TokenType type) {
    return TokenToStr.at(type);
}

struct Token {
    Token(TokenType type, SourceLocation loc) : type(type), location(loc) {}
    Token(TokenType type, SourceLocation loc, std::string_view v) : type(type), location(loc), value(v) {}

    TokenType type;
    SourceLocation location;
    std::optional<std::string> value = std::nullopt; // literal or identifier
};

namespace Tokenizer {

std::vector<Token> Tokenize(std::string_view src);

} // namespace Tokenizer

void Error(SourceLocation loc, const std::string& msg);
void Error(const std::string& msg);

} // namespace Glassy
