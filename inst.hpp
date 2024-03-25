#pragma once

#include <string>
#include <utility>
#include <memory>
#include <cassert>

namespace YAOPT {

struct Descriptor {
    [[nodiscard]] virtual std::string serialize() const = 0;
    virtual ~Descriptor() = default;
};


enum class Type {
    VOID, I1, I64, DOUBLE, PTR, LABEL
};

struct Operand {
    Type type;
    std::string param;

    [[nodiscard]] bool is_void() const {
        return type == Type::VOID;
    }

    [[nodiscard]] bool is_reg() const {
        return param.starts_with("%");
    }

    [[nodiscard]] bool is_imm() const {
        return !is_reg();
    }

    [[nodiscard]] std::string label() const {
        assert(type == Type::LABEL && is_reg());
        return param.substr(1);
    }
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
//    std::size_t index;
    [[nodiscard]] Kind kind() const override {
        return Inst::Kind::INTERMEDIATE;
    }
};

//struct OpInst : IntermediateInst {};
//
//struct UnaryOpInst : OpInst {};
//
//struct BinaryOpInst : OpInst {};
//
//struct MemInst : IntermediateInst {};
//
//struct AllocaInst : MemInst {};
//
//struct LoadInst : MemInst {};
//
//struct StoreInst : MemInst {};
//
//struct GEPInst : MemInst {};
//
//struct ConvInst : IntermediateInst {};
//
//struct CmpInst : IntermediateInst {};
//
//struct CallInst : IntermediateInst {};
//
//struct PhiInst : IntermediateInst {}; // Unused

struct TerminatorInst : Inst {
    [[nodiscard]] Kind kind() const override {
        return Inst::Kind::TERMINATOR;
    }
    [[nodiscard]] virtual std::string transition(const std::string& from) const = 0;
};

struct RetInst : TerminatorInst {
    Operand operand;
    [[nodiscard]] std::string transition(const std::string& from) const override {
        return from + "-->EXIT";
    }
};

struct BrLabelInst : TerminatorInst {
    Operand label;
    [[nodiscard]] std::string transition(const std::string& from) const override {
        return from + "-->" + label.label();
    }
};

struct BrCondInst : TerminatorInst {
    Operand cond, label1, label2;
    [[nodiscard]] std::string transition(const std::string& from) const override {
        return from + "-->" + label1.label() + "\n" + from + "-->" + label2.label();
    }
};

struct UnreachableInst : TerminatorInst {
    [[nodiscard]] std::string transition(const std::string& from) const override {
        return "";
    }
};


}