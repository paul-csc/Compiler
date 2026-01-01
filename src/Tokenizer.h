#pragma once

#include <cstdint>
#include <string>
#include <variant>
#include <vector>

namespace Glassy {

// clang-format off

enum TokenType { 
    IDENTIFIER,
    KEYWORD,
    SEPARATOR,
    OPERATOR,
    LITERAL
};

using Identifier = std::string;
using Literal = int; // change to double later

enum class Keyword {
    EXIT,
    LET,
    KEYWORD_NB
};

enum class Separator {
    L_PAREN,
    R_PAREN,
    L_BRACKET,
    R_BRACKET,
    L_BRACE,
    R_BRACE,
    SEMI,
    SEPARATOR_NB
};

enum class Operator {
    PLUS,
    MINUS,
    STAR,
    SLASH,
    PERCENT,
    CARET,
    EQUAL,
    OPERATOR_NB
};

// clang-format on

extern const std::string_view SeparatorToStr[];
extern const std::string_view OperatorToStr[];

struct SourceLocation {
    uint16_t line = 1;
    uint16_t column = 1;
};

struct Token {
    Token(std::string_view id, SourceLocation l)
        : type(IDENTIFIER), location(l), lexeme(id), value(std::string(id)) {}

    Token(Keyword kw, SourceLocation l, std::string_view lex)
        : type(KEYWORD), location(l), lexeme(lex), value(kw) {}

    Token(Literal v, SourceLocation l, std::string_view lex)
        : type(LITERAL), location(l), lexeme(lex), value(v) {}

    Token(Operator op, SourceLocation l)
        : type(OPERATOR), location(l), lexeme(OperatorToStr[size_t(op)]), value(op) {}

    Token(Separator sep, SourceLocation l)
        : type(SEPARATOR), location(l), lexeme(SeparatorToStr[size_t(sep)]), value(sep) {}

    template <typename T>
    const T* GetValue() const {
        return std::get_if<T>(&value);
    }

    TokenType type;
    SourceLocation location;

    std::string_view lexeme;

    std::variant<std::monostate, Identifier, Literal, Keyword, Operator, Separator> value;
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