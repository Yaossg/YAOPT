#include "lexer.hpp"
#include "diagnostics.hpp"


namespace YAOPT {

[[nodiscard]] constexpr bool isDecimal(char ch) noexcept {
    return ch >= '0' && ch <= '9';
}

[[nodiscard]] constexpr bool isNumberStart(char ch) noexcept {
    return isDecimal(ch);
}

[[nodiscard]] constexpr bool isPunctuation(char ch) {
    return ispunct(ch);
}

[[nodiscard]] bool isIdentifierStart(char ch) {
    return ch == '@' || ch == '%' || ch == '"' || ch == '_' || isalpha(ch);
}

[[nodiscard]] bool isIdentifierPart(char ch) {
    return ch == '"' || ch == '\\' || ch == '_' || isalnum(ch);
}

void LineTokenizer::raise(const char *msg) const {
    YAOPT::raise(msg, make(TokenType::INVALID));
}

void LineTokenizer::tokenize() {
    context.tokens.emplace_back();
    while (remains()) {
        switch (char ch = getc()) {
            case '#':
                q = r;
                break;
            case '\n':
            case '\r':
            case '\t':
            case ' ':
                break;
            case '"':
                addId();
                break;
            default: {
                ungetc(ch);
                if (isIdentifierStart(ch)) {
                    addId();
                } else if (isNumberStart(ch)) {
                    addNumber();
                } else if (isPunctuation(ch)) {
                    addPunct();
                }
            }
        }
        step();
    }
    if (context.tokens.back().empty())
        context.tokens.pop_back();
}


void LineTokenizer::addId() {
    std::string_view remains{p, r};
    if (!isIdentifierStart(remains.front())) {
        raise("unexpected character");
    }
    do q = (remains = remains.substr(1)).data();
    while (!remains.empty() && isIdentifierPart(remains.front()));
    add(TokenType::IDENTIFIER);
}

void LineTokenizer::addPunct() {
    std::string_view remains{p, r};
    auto punct = PUNCTUATIONS.end();
    for (auto it = PUNCTUATIONS.begin(); it != PUNCTUATIONS.end(); ++it) {
        if (remains.starts_with(it->first)
            && (punct == PUNCTUATIONS.end() || it->first.length() > punct->first.length())) {
            punct = it;
        }
    }
    if (punct != PUNCTUATIONS.end()) {
        q += punct->first.length();
        add(punct->second);
    } else {
        raise("invalid punctuation");
    }
}

void LineTokenizer::scanDigits(bool (*pred)(char) noexcept) {
    char ch = getc();
    if (!pred(ch)) raise("invalid number literal");
    do ch = getc();
    while (ch == '_' || pred(ch));
    ungetc(ch);
    if (q[-1] == '_') raise("invalid number literal");
}

void LineTokenizer::addNumber() {
    // scan number prefix
    TokenType base = TokenType::INTEGER;
    bool (*pred)(char) noexcept = isDecimal;
    // scan digits
    scanDigits(pred);
    bool flt = false;
    if (peekc() == '.') {
        getc();
        if (pred(peekc())) {
            scanDigits(pred);
            flt = true;
        } else {
            ungetc('.');
        }
    }
    if (peekc() == 'e' || peekc() == 'E') {
        flt = true;
        getc();
        if (peekc() == '+' || peekc() == '-') getc();
        scanDigits(isDecimal);
    }
    // classification
    TokenType type = base;
    if (flt) {
        type = TokenType::FLOATING_POINT;
    }
    add(type);
}

void LineTokenizer::add(TokenType type) {
    context.tokens.back().push_back(make(type));
    step();
    switch (type) {
        case TokenType::LPAREN:
        case TokenType::LBRACKET:
        case TokenType::LBRACE:
            context.greedy.push_back(context.tokens.back().back());
            break;
        case TokenType::RPAREN:
            checkGreedy("(", ")", TokenType::LPAREN);
            break;
        case TokenType::RBRACKET:
            checkGreedy("[", "]", TokenType::LBRACKET);
            break;
        case TokenType::RBRACE:
            checkGreedy("{", "}", TokenType::LBRACE);
            break;
    }
}

void LineTokenizer::checkGreedy(const char* left, const char* right, TokenType match) {
    if (context.greedy.empty()) {
        Error().with(
                ErrorMessage().error(context.tokens.back().back())
                .text("stray").quote(right).text("without").quote(left).text("to match")
                ).raise();
    }
    if (context.greedy.back().type != match) {
        Error error;
        error.with(ErrorMessage().error(context.tokens.back().back()).quote(right).text("mismatch"));
        error.with(ErrorMessage().note(context.greedy.back()).quote(left).text("expected here"));
        for (auto it = context.greedy.rbegin(); it != context.greedy.rend(); ++it) {
            if (it->type == match) {
                error.with(ErrorMessage().note(*it).text("nearest matching").quote(left).text("is here"));
                break;
            }
        }
        if (error.messages.size() < 3) {
            error.with(ErrorMessage().note().text("stray").quote(right).text("without").quote(left).text("to match"));
        }
        error.raise();
    }
    context.greedy.pop_back();
}

int64_t parseInt(Source& source, Token token) try {
    std::string literal(source.of(token));
    return std::stoll(literal, nullptr, 10);
} catch (std::out_of_range& e) {
    raise("int literal out of range", token);
}

double parseFloat(Source& source, Token token) try {
    std::string literal(source.of(token));
    std::erase(literal, '_');
    return std::stod(literal);
} catch (std::out_of_range& e) {
    raise("float literal out of range", token);
}

}