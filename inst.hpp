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
    Type type;
    std::string value;
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
    std::string cond;
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