#pragma once

#include "util.hpp"
#include "lexer.hpp"
#include "source.hpp"
#include "entity.hpp"
#include "diagnostics.hpp"

namespace YAOPT {

struct LineParser;

struct Parser {
    Source source;
    std::string input;
    std::vector<std::unique_ptr<Entity>> entities;


    explicit Parser(std::string input) : input(std::move(input)) {}

    void tokenize() {
        auto lines = splitLines(this->input);
        for (auto line: lines) {
            source.append(std::string(line));
        }
    }

    void parse();

    using iterator = std::vector<std::vector<Token>>::iterator;

    iterator parseDeclare(iterator it);
    iterator parseDefine(iterator it);
    iterator parseGlobalVariable(iterator it);
};

struct LineParser {
    Source& source;
    std::vector<Token>::const_iterator p;
    std::vector<Token>::const_iterator q;

    LineParser(Source& source, std::vector<Token>& tokens): source(source), p(tokens.begin()), q(tokens.end()) {}

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

    std::string_view nextView() noexcept {
        return source.of(next());
    }


    Token expect(TokenType type, const char* msg) {
        auto token = next();
        if (token.type != type) {
            Error().with(ErrorMessage().error(token).quote(msg).text("is expected")).raise();
        }
        return token;
    }

    std::unique_ptr<FunctionDeclare> parseDeclare();
    std::unique_ptr<FunctionDefine> parseDefine();
    std::unique_ptr<GlobalVariable> parseGlobalVariable();

    std::unique_ptr<Inst> parseInst();


};


}