#pragma once

#include <unordered_map>
#include <string_view>

namespace YAOPT {

enum class TokenType {
    INVALID,

    IDENTIFIER,

    LPAREN,
    RPAREN,
    LBRACKET,
    RBRACKET,
    LBRACE,
    RBRACE,

    OP_ASSIGN,
    OP_STAR,
    OP_PERCENT,
    OP_DOT,
    OP_COMMA,
    OP_COLON,

    INTEGER,
    FLOATING_POINT
};

const std::unordered_map<std::string_view, TokenType> PUNCTUATIONS {
    {"=", TokenType::OP_ASSIGN},
    {"*", TokenType::OP_STAR},
    {"%", TokenType::OP_PERCENT},
    {".", TokenType::OP_DOT},
    {",", TokenType::OP_COMMA},
    {":", TokenType::OP_COLON},
    {"(", TokenType::LPAREN},
    {")", TokenType::RPAREN},
    {"[", TokenType::LBRACKET},
    {"]", TokenType::RBRACKET},
    {"{", TokenType::LBRACE},
    {"}", TokenType::RBRACE},
};

struct Segment {
    size_t line1, line2, column1, column2;
};

struct Token {
    size_t line, column, width;
    TokenType type;

    operator Segment() const noexcept {
        return {.line1 = line, .line2 = line, .column1 = column, .column2 = column + width};
    }
};

inline Segment range(Token from, Token to) noexcept {
    return {.line1 = from.line, .line2 = to.line, .column1 = from.column, .column2 = to.column + to.width};
}

inline Segment range(Segment from, Segment to) noexcept {
    return {.line1 = from.line1, .line2 = to.line2, .column1 = from.column1, .column2 = to.column2};
}

}