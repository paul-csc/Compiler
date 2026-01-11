#pragma once

#include "pch.h"

namespace Compiler {

enum TokenType {
    IDENTIFIER,
    LITERAL,

    RETURN,
    INT,
    IF,
    ELSE,

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
    CARET,
    EQUAL,

    END_OF_FILE,

    TOKEN_TYPE_NB
};

struct SourceLocation {
    uint16_t Line = 1;
    uint16_t Column = 1;
};

constexpr std::array<std::string_view, TOKEN_TYPE_NB> TokenNames = { "identifier", "literal", "return", "int",
    "if", "else", "(", ")", "{", "}", ";", ",", "+", "-", "*", "/", "%", "^", "=", "eof" };

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

std::vector<Token> Lex(std::string_view src);

} // namespace Compiler
