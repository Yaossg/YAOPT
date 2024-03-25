#include "parser.hpp"

#include <cassert>
#include <optional>

namespace YAOPT {

void Parser::parse() {
    for (auto it = source.tokens.begin(); it != source.tokens.end(); ) {
        assert(!it->empty());
        auto id = source.of(it->front());
        if (id == "define") {
            it = parseDefine(it);
        } else if (id == "declare") {
            it = parseDeclare(it);
        } else if (id.starts_with("@")) {
            it = parseGlobalVariable(it);
        }
    }
}


auto Parser::parseDefine(iterator it) -> iterator {
    auto define = LineParser{source, *it}.parseDefine();
    ++it;
    std::vector<std::unique_ptr<Inst>> insts;
    while (it->front().type != TokenType::RBRACE) { // assume *it never overflows
        insts.push_back(LineParser{source, *it}.parseInst());
        auto segment = range(it->front(), it->back());
        Token token{.line = segment.line1, .column = segment.column1, .width = segment.column2 - segment.column1};
        std::string code(source.of(token));
        insts.back()->code = code;
        ++it;
    }
    ++it;
    std::vector<std::unique_ptr<Inst>> bb;
    auto inst = insts.begin();
    while (inst != insts.end()) {
        assert((*inst)->kind() == Inst::Kind::LABEL);
        bb.push_back(std::move(*inst++));
        while (inst != insts.end() && (*inst)->kind() != Inst::Kind::TERMINATOR) {
            bb.push_back(std::move(*inst++));
        }
        assert(inst != insts.end());
        bb.push_back(std::move(*inst++));
        std::vector<std::unique_ptr<Inst>> t; t.swap(bb); // suppressing use-after-move warning'
        define->bbs.emplace_back(std::move(t));
    }
    entities.push_back(std::move(define));
    return it;
}

auto Parser::parseDeclare(iterator it) -> iterator {
    //entities.push_back(LineParser{source, *it}.parseDeclare());
    return ++it;
}

auto Parser::parseGlobalVariable(iterator it) -> iterator {
    //entities.push_back(LineParser{source, *it}.parseGlobalVariable());
    return ++it;
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

std::unique_ptr<Inst> LineParser::parseInst() {
    assert(remains());
    auto inst = source.of(next());
    if (remains() && peek().type == TokenType::OP_COLON) {
        next();
        assert(!remains());
        return std::make_unique<LabelInst>(std::string(inst));
    }
    std::optional<std::string_view> receiver;
    if (remains() && peek().type == TokenType::OP_ASSIGN) {
        next();
        receiver = inst;
        assert(remains());
        inst = source.of(next());
    }
    if (inst == "unreachable") {
        return std::make_unique<UnreachableInst>();
    } else if (inst == "br") {
        auto head = source.of(next());
        if (head == "label") {
            auto br = std::make_unique<BrLabelInst>();
            br->label = source.of(next()).substr(1);
            return br;
        } else {
            auto br = std::make_unique<BrCondInst>();
            // br i1 %cond, label %L1, label %L2
            br->cond = source.of(next());
            expect(TokenType::OP_COMMA, "comma");
            next();
            br->label1 = source.of(next()).substr(1);
            expect(TokenType::OP_COMMA, "comma");
            next();
            br->label2 = source.of(next()).substr(1);
            return br;
        }
    } else if (inst == "ret") {
        auto ret = std::make_unique<RetInst>();
        ret->type = parseType(source.of(next()));
        if (ret->type != Type::VOID) ret->value = source.of(next());
        return ret;
    } else {
        return std::make_unique<IntermediateInst>();
    }
}

std::unique_ptr<FunctionDefine> LineParser::parseDefine() {
    next(); // define
    next(); // return type
    std::string name(source.of(expect(TokenType::IDENTIFIER, "identifier")));
    auto define = std::make_unique<FunctionDefine>();
    define->name = std::move(name);
    return define;
}

std::unique_ptr<FunctionDeclare> LineParser::parseDeclare() {
    return std::unique_ptr<FunctionDeclare>();
}

std::unique_ptr<GlobalVariable> LineParser::parseGlobalVariable() {
    return std::unique_ptr<GlobalVariable>();
}

}