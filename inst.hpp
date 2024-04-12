#pragma once

#include <string>
#include <utility>
#include <memory>
#include <cassert>
#include <optional>
#include "opcode.hpp"

namespace YAOPT {

struct Descriptor {
    [[nodiscard]] virtual std::string serialize() const = 0;
    virtual ~Descriptor() = default;
};

struct Value {
    std::string literal;

    Value() = default;
    Value(std::string_view literal): literal(literal) {}

    [[nodiscard]] bool is_reg() const { return literal.starts_with('%'); }
    [[nodiscard]] bool is_imm() const { return !is_reg(); }
};

enum class Type {
    VOID, I1, I64, DOUBLE, PTR, LABEL
};

struct Inst : Descriptor {
    std::string code;

    enum class Kind {
        LABEL, INTERMEDIATE, TERMINATOR
    };
    [[nodiscard]] virtual Kind kind() const = 0;
    [[nodiscard]] std::string serialize() const override {
        return code;
    }
};

struct LabelInst : Inst {
    std::string label;
    explicit LabelInst(std::string label): label(std::move(label)) {}

    [[nodiscard]] Kind kind() const override {
        return Inst::Kind::LABEL;
    }
};

struct IntermediateInst : Inst {
    std::optional<std::string> receiver;
    [[nodiscard]] Kind kind() const override {
        return Inst::Kind::INTERMEDIATE;
    }
};

struct OpInst : IntermediateInst {
};

struct UnaryOpInst : OpInst {
    inline static const Type type = Type::DOUBLE;
    Value value;
    explicit UnaryOpInst(Value value): value(std::move(value)) {}
};

struct BinaryOpInst : OpInst {
    Opcode op;
    Type type;
    Value value1, value2;
    BinaryOpInst(Opcode op, Type type, Value value1, Value value2):
        op(op), type(type), value1(std::move(value1)), value2(std::move(value2)) {}
};

struct MemInst : IntermediateInst {};

struct AllocaInst : MemInst {
    Type type;
    explicit AllocaInst(Type type): type(type) {}
};

struct LoadInst : MemInst {
    Type type; Value from;

    LoadInst(Type type, Value from) : type(type), from(std::move(from)) {}
};

struct StoreInst : MemInst {
    Type type; Value from, into;

    StoreInst(Type type, Value from, Value into) : type(type), from(std::move(from)), into(std::move(into)) {}
};

struct GEPInst : MemInst {
    Type type; Value ptr, offset;

    GEPInst(Type type, Value ptr, Value offset) : type(type), ptr(std::move(ptr)), offset(std::move(offset)) {}
};

struct CmpInst : IntermediateInst {
    Type type;
    Value value1, value2;

    CmpInst(Type type, Value value1, Value value2) : type(type), value1(std::move(value1)), value2(std::move(value2)) {}

};

struct IcmpInst : CmpInst {
    enum class Op {
        EQ, NE,
        SLT, ULT,
        SLE, ULE,
        SGT, UGT,
        SGE, UGE,
    };
    Op op;

    inline static const std::unordered_map<std::string_view, Op> OPS {
            {"eq", Op::EQ},
            {"ne", Op::NE},
            {"slt", Op::SLT},
            {"ult", Op::ULT},
            {"sle", Op::SLE},
            {"ule", Op::ULE},
            {"sgt", Op::SGT},
            {"ugt", Op::UGT},
            {"sge", Op::SGE},
            {"uge", Op::UGE},
    };

    IcmpInst(Type type, const Value &value1, const Value &value2, Op op) : CmpInst(type, value1, value2), op(op) {}
};

struct FcmpInst : CmpInst {
    enum class Op {
        FALSE,
        OEQ,
        OGT,
        OGE,
        OLT,
        OLE,
        ONE,
        ORD,
        UEQ,
        UGT,
        UGE,
        ULT,
        ULE,
        UNE,
        UNO,
        TRUE
    };
    Op op;

    inline static const std::unordered_map<std::string_view, Op> OPS {
            {"false", Op::FALSE},
            {"oeq", Op::OEQ},
            {"ogt", Op::OGT},
            {"oge", Op::OGE},
            {"olt", Op::OLT},
            {"ole", Op::OLE},
            {"one", Op::ONE},
            {"ord", Op::ORD},
            {"ueq", Op::UEQ},
            {"ugt", Op::UGT},
            {"uge", Op::UGE},
            {"ult", Op::ULT},
            {"ule", Op::ULE},
            {"une", Op::UNE},
            {"uno", Op::UNO},
            {"true", Op::TRUE},
    };

    FcmpInst(Type type, const Value &value1, const Value &value2, Op op) : CmpInst(type, value1, value2), op(op) {}

};

struct ConvInst : IntermediateInst {
    Opcode op;
    Type type1, type2;
    Value value;

    ConvInst(Opcode op, Type type1, Type type2, Value value) : op(op), type1(type1), type2(type2),
                                                                      value(std::move(value)) {}
};

struct CallInst : IntermediateInst {
    struct TypedValue {
        Type type; Value value;
    };

    Type ret_type; Value function;
    std::vector<TypedValue> args;

    CallInst(Type ret_type, Value function, const std::vector<TypedValue> &args):
            ret_type(ret_type), function(std::move(function)), args(args) {}
};

struct TerminatorInst : Inst {
    [[nodiscard]] Kind kind() const override {
        return Inst::Kind::TERMINATOR;
    }
    [[nodiscard]] virtual std::string transition(const std::string& from) const = 0;
};

struct RetInst : TerminatorInst {
    Type type = Type::VOID;
    Value value;

    [[nodiscard]] std::string transition(const std::string& from) const override {
        return from + "-->EXIT";
    }
};

struct BrLabelInst : TerminatorInst {
    std::string label;
    [[nodiscard]] std::string transition(const std::string& from) const override {
        return from + "-->" + label;
    }
};

struct BrCondInst : TerminatorInst {
    Type type = Type::I1;
    Value cond;
    std::string label1, label2;
    [[nodiscard]] std::string transition(const std::string& from) const override {
        return from + "-->" + label1 + "\n" + from + "-->" + label2;
    }
};

struct UnreachableInst : TerminatorInst {
    [[nodiscard]] std::string transition(const std::string& from) const override {
        return "";
    }
};


}