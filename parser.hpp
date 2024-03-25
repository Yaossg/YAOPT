#pragma once

#include "util.hpp"
#include "lexer.hpp"
#include "source.hpp"
#include "entity.hpp"
#include "diagnostics.hpp"

namespace YAOPT {

struct Parser {
    Source source;
    std::string input;
    std::vector<std::unique_ptr<Entity>> entities;

    std::vector<Token>::const_iterator p;
    std::vector<Token>::const_iterator q;

    explicit Parser(std::string input): input(std::move(input)) {}

    void tokenize() {
        auto lines = splitLines(this->input);
        for (auto line : lines) {
            source.append(std::string(line));
        }
        p = source.tokens.begin();
        q = source.tokens.end();
    }

    Token next() {
        if (p != q) {
            return *p++;
        } else {
            raise("unexpected termination of tokens", rewind());
        }
    }
    [[nodiscard]] Token peek() const noexcept {
        return *p;
    }
    [[nodiscard]] Token rewind() const noexcept {
        return *std::prev(p);
    }
    [[nodiscard]] bool remains() const noexcept {
        return p != q;
    }


    Token expect(TokenType type, const char* msg) {
        auto token = next();
        if (token.type != type) {
            Error().with(ErrorMessage().error(token).quote(msg).text("is expected")).raise();
        }
        return token;
    }

    void parse();

    void parseDeclare();
    void parseDefine();
    void parseGlobalVariable();
    std::unique_ptr<Inst> parseInst(std::vector<Token> const& tokens) const;
    Operand parseOperand(std::vector<Token> const& tokens, size_t index) const;

    void parseType();


};


}