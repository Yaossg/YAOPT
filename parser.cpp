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
    entities.push_back(LineParser{source, *it}.parseDeclare());
    return ++it;
}

auto Parser::parseGlobalVariable(iterator it) -> iterator {
    entities.push_back(LineParser{source, *it}.parseGlobalVariable());
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
    auto inst = nextView();
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
        inst = nextView();
    }
    auto OPCODES_it = OPCODES.find(inst);
    assert(OPCODES_it != OPCODES.end());
    auto opcode = OPCODES_it->second;
    if (opcode == Opcode::UNREACHABLE) {
        return std::make_unique<UnreachableInst>();
    } else if (opcode == Opcode::BR) {
        auto head = nextView();
        if (head == "label") {
            auto br = std::make_unique<BrLabelInst>();
            br->label = nextView().substr(1);
            return br;
        } else if (head == "i1") {
            auto br = std::make_unique<BrCondInst>();
            br->cond = nextView();
            expect(TokenType::OP_COMMA, "comma");
            next();
            br->label1 = nextView().substr(1);
            expect(TokenType::OP_COMMA, "comma");
            next();
            br->label2 = nextView().substr(1);
            return br;
        }
        assert(false);
    } else if (opcode == Opcode::RET) {
        auto ret = std::make_unique<RetInst>();
        ret->type = parseType(nextView());
        if (ret->type != Type::VOID) ret->value = nextView();
        return ret;
    }
    std::unique_ptr<IntermediateInst> ret;
    switch (opcode) {
        case Opcode::FNEG:
            assert(nextView() == "double");
            ret = std::make_unique<UnaryOpInst>(nextView());
            break;
        case Opcode::ADD:
        case Opcode::FADD:
        case Opcode::SUB:
        case Opcode::FSUB:
        case Opcode::MUL:
        case Opcode::FMUL:
        case Opcode::UDIV:
        case Opcode::SDIV:
        case Opcode::FDIV:
        case Opcode::UREM:
        case Opcode::SREM:
        case Opcode::FREM:
        case Opcode::SHL:
        case Opcode::LSHR:
        case Opcode::ASHR:
        case Opcode::AND:
        case Opcode::OR:
        case Opcode::XOR: {
            auto type = parseType(nextView());
            auto value1 = nextView();
            expect(TokenType::OP_COMMA, "comma");
            auto value2 = nextView();
            ret = std::make_unique<BinaryOpInst>(opcode, type, value1, value2);
            break;
        }
        case Opcode::ALLOCA:
            ret = std::make_unique<AllocaInst>(parseType(nextView()));
            break;
        case Opcode::LOAD: {
            auto type = parseType(nextView());
            expect(TokenType::OP_COMMA, "comma");
            assert(nextView() == "ptr");
            auto from = nextView();
            ret = std::make_unique<LoadInst>(type, from);
            break;
        }
        case Opcode::STORE: {
            auto type = parseType(nextView());
            auto from = nextView();
            expect(TokenType::OP_COMMA, "comma");
            assert(nextView() == "ptr");
            auto into = nextView();
            ret = std::make_unique<StoreInst>(type, from, into);
            break;
        }
        case Opcode::GETELEMENTPTR: {
            assert(nextView() == "inbounds");
            auto type = parseType(nextView());
            expect(TokenType::OP_COMMA, "comma");
            assert(nextView() == "ptr");
            auto ptr = nextView();
            expect(TokenType::OP_COMMA, "comma");
            assert(nextView() == "i64");
            auto offset = nextView();
            ret = std::make_unique<GEPInst>(type, ptr, offset);
            break;
        }
        case Opcode::ICMP: {
            auto op = IcmpInst::OPS.at(nextView());
            auto type = parseType(nextView());
            auto value1 = nextView();
            expect(TokenType::OP_COMMA, "comma");
            auto value2 = nextView();
            ret = std::make_unique<IcmpInst>(type, value1, value2, op);
            break;
        }
        case Opcode::FCMP: {
            auto op = FcmpInst::OPS.at(nextView());
            auto type = parseType(nextView());
            auto value1 = nextView();
            expect(TokenType::OP_COMMA, "comma");
            auto value2 = nextView();
            ret = std::make_unique<FcmpInst>(type, value1, value2, op);
            break;
        }
        case Opcode::SITOFP:
        case Opcode::FPTOSI:
        case Opcode::INTTOPTR:
        case Opcode::PTRTOINT: {
            auto type1 = parseType(nextView());
            auto value = nextView();
            assert(nextView() == "to");
            auto type2 = parseType(nextView());
            ret = std::make_unique<ConvInst>(opcode, type1, type2, value);
            break;
        }
        case Opcode::CALL: {
            auto ret_type = parseType(nextView());
            auto function = nextView();
            std::vector<CallInst::TypedValue> args;
            expect(TokenType::LPAREN, "(");
            while (peek().type != TokenType::RPAREN) {
                if (!args.empty()) expect(TokenType::OP_COMMA, "comma");
                auto type = parseType(nextView());
                auto value = nextView();
                args.push_back({type, value});
            }
            ret = std::make_unique<CallInst>(ret_type, function, std::move(args));
            break;
        }
    }
    ret->receiver = receiver;
    return ret;
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
    next(); // define
    next(); // return type
    std::string name(source.of(expect(TokenType::IDENTIFIER, "identifier")));
    auto declare = std::make_unique<FunctionDeclare>();
    declare->name = std::move(name);
    return declare;
}

std::unique_ptr<GlobalVariable> LineParser::parseGlobalVariable() {
    std::string name(source.of(expect(TokenType::IDENTIFIER, "identifier")));
    auto gv = std::make_unique<GlobalVariable>();
    gv->name = std::move(name);
    return gv;
}

}