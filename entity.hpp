#pragma once

#include <string>
#include <vector>
#include <stdexcept>
#include "inst.hpp"


namespace YAOPT {

struct Entity : Descriptor {
    std::string name;

};

struct GlobalVariable : Entity {

};

struct FunctionDeclare : Entity {

};

struct BasicBlock : Descriptor {
    std::vector<std::unique_ptr<Inst>> insts;
    LabelInst* labelInst;
    TerminatorInst* terminatorInst;
    explicit BasicBlock(std::vector<std::unique_ptr<Inst>> insts): insts(std::move(insts)) {
        if (this->insts.size() < 2) {
            throw std::invalid_argument("invalid basic block");
        }
        labelInst = dynamic_cast<LabelInst*>(this->insts.front().get());
        terminatorInst = dynamic_cast<TerminatorInst*>(this->insts.back().get());
        if (!labelInst || !terminatorInst) {
            throw std::invalid_argument("invalid basic block");
        }
    }

    [[nodiscard]] std::string serialize() const override {
        std::string buf;
        buf += labelInst->label;
        buf += "[\"";
        for (auto&& inst : insts) {
            buf += inst->serialize();
            buf += "\\n";
        }
        buf += "\"]\n";
        buf += terminatorInst->transition(labelInst->label);
        buf += "\n";
        return buf;
    }
};

struct FunctionDefine : Entity {

    std::vector<BasicBlock> bbs;

    [[nodiscard]] std::string serialize() const override {
        std::string buf;
        buf += "## ";
        buf += name;
        buf += "\n";
        buf += "```mermaid\n";
        buf += "graph\n";
        buf += "ENTER-->L0\n";
        for (auto&& bb : bbs) {
            buf += bb.serialize();
        }
        buf += "\n```\n";
        return buf;
    }
};

}