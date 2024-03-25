#include "parser.hpp"

#include <cassert>

namespace YAOPT {

void Parser::parse() {
    while (remains()) {
        while (remains() && peek().type == TokenType::LINEBREAK) next();
        if (!remains()) break;
        auto token = expect(TokenType::IDENTIFIER, "identifier");
        auto id = source.of(token);
        if (id == "define") {
            parseDefine();
        } else if (id == "declare") {
            parseDeclare();
        } else if (id.starts_with("@")) {
            parseGlobalVariable();
        }
    }
}

void Parser::parseDeclare() {
    while (remains() && next().type != TokenType::LINEBREAK);
}

Type parseType(std::string_view type) {
    static const std::unordered_map<std::string_view, Type> TYPE {
            {"void", Type::VOID},
            {"i1", Type::I1},
            {"i64", Type::I64},
            {"double", Type::DOUBLE},
            {"ptr", Type::PTR},
            {"label", Type::LABEL}
    };
    return TYPE.at(type);
}

Operand Parser::parseOperand(std::vector<Token> const& tokens, size_t index) const {
    auto type = YAOPT::parseType(source.of(tokens[index]));
    if (type != Type::VOID) {
        return Operand{type, std::string(source.of(tokens[index + 1]))};
    }
    return Operand{Type::VOID};
}

std::unique_ptr<Inst> Parser::parseInst(std::vector<Token> const& tokens) const {
    auto inst = source.of(tokens.front());
    if (inst == "unreachable") {
        return std::make_unique<UnreachableInst>();
    } else if (inst == "br") {
        auto head = source.of(tokens.at(1));
        if (head == "label") {
            auto br = std::make_unique<BrLabelInst>();
            br->label = parseOperand(tokens, 1);
            return br;
        } else {
            auto br = std::make_unique<BrCondInst>();
            // br i1 %cond, label L1, label L2
            // 0  1  2    3 4     5 6 7     8
            br->cond = parseOperand(tokens, 1);
            br->label1 = parseOperand(tokens, 4);
            br->label2 = parseOperand(tokens, 7);
            return br;
        }
    } else if (inst == "ret") {
        auto ret = std::make_unique<RetInst>();
        ret->operand = parseOperand(tokens, 1);
        return ret;
    } else {
        return std::make_unique<IntermediateInst>();
    }
}

void Parser::parseDefine() {
    std::vector<std::unique_ptr<Inst>> insts;
    parseType();
    std::string f_name(source.of(expect(TokenType::IDENTIFIER, "identifier")));
    while (remains() && next().type != TokenType::LINEBREAK); // ignore first line
    while (remains() && peek().type != TokenType::RBRACE) {
        std::vector<Token> tokens;
        while (remains() && peek().type != TokenType::LINEBREAK) {
            tokens.push_back(next());
        }
        next();
        if (tokens.empty()) continue;
        if (tokens.front().type != TokenType::IDENTIFIER) raise("instruction or label expected", tokens.front());
        if (tokens.size() == 2 && tokens.back().type == TokenType::OP_COLON) {
            insts.push_back(std::make_unique<LabelInst>(std::string(source.of(tokens.front()))));
        } else {
            insts.push_back(parseInst(tokens));
        }

        auto segment = range(tokens.front(), tokens.back());
        Token token{.line = segment.line1, .column = segment.column1, .width = segment.column2 - segment.column1};
        std::string code(source.of(token));
        insts.back()->code = code;
    }
    expect(TokenType::RBRACE, "brace");
    std::vector<std::unique_ptr<Inst>> bb;
    auto define = std::make_unique<FunctionDefine>();
    auto it = insts.begin();
    while (it != insts.end()) {
        assert((*it)->kind() == Inst::Kind::LABEL);
        bb.push_back(std::move(*it++));
        while (it != insts.end() && (*it)->kind() != Inst::Kind::TERMINATOR) {
            bb.push_back(std::move(*it++));
        }
        assert(it != insts.end());
        bb.push_back(std::move(*it++));
        define->bbs.emplace_back(std::move(bb));
    }
    define->name = f_name;
    entities.push_back(std::move(define));
}

void Parser::parseGlobalVariable() {
    while (remains() && next().type != TokenType::LINEBREAK);
}

void Parser::parseType() {
    expect(TokenType::IDENTIFIER, "type");
}


}