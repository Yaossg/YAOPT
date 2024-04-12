#pragma once

#include <unordered_map>
#include <string_view>

enum class Opcode {
    FNEG,
    ADD,
    FADD,
    SUB,
    FSUB,
    MUL,
    FMUL,
    UDIV,
    SDIV,
    FDIV,
    UREM,
    SREM,
    FREM,
    SHL,
    LSHR,
    ASHR,
    AND,
    OR,
    XOR,
    ALLOCA,
    LOAD,
    STORE,
    GETELEMENTPTR,
    ICMP,
    FCMP,
    SITOFP,
    FPTOSI,
    INTTOPTR,
    PTRTOINT,
    CALL,
    UNREACHABLE,
    RET,
    BR,
};

constexpr std::string_view OPCODE_NAME[] = {
    "fneg",
    "add",
    "fadd",
    "sub",
    "fsub",
    "mul",
    "fmul",
    "udiv",
    "sdiv",
    "fdiv",
    "urem",
    "srem",
    "frem",
    "shl",
    "lshr",
    "ashr",
    "and",
    "or",
    "xor",
    "alloca",
    "load",
    "store",
    "getelementptr",
    "icmp",
    "fcmp",
    "sitofp",
    "fptosi",
    "inttoptr",
    "ptrtoint",
    "call",
    "unreachable",
    "ret",
    "br",
};

inline const std::unordered_map<std::string_view, Opcode> OPCODES {
    {"fneg", Opcode::FNEG},
    {"add", Opcode::ADD},
    {"fadd", Opcode::FADD},
    {"sub", Opcode::SUB},
    {"fsub", Opcode::FSUB},
    {"mul", Opcode::MUL},
    {"fmul", Opcode::FMUL},
    {"udiv", Opcode::UDIV},
    {"sdiv", Opcode::SDIV},
    {"fdiv", Opcode::FDIV},
    {"urem", Opcode::UREM},
    {"srem", Opcode::SREM},
    {"frem", Opcode::FREM},
    {"shl", Opcode::SHL},
    {"lshr", Opcode::LSHR},
    {"ashr", Opcode::ASHR},
    {"and", Opcode::AND},
    {"or", Opcode::OR},
    {"xor", Opcode::XOR},
    {"alloca", Opcode::ALLOCA},
    {"load", Opcode::LOAD},
    {"store", Opcode::STORE},
    {"getelementptr", Opcode::GETELEMENTPTR},
    {"icmp", Opcode::ICMP},
    {"fcmp", Opcode::FCMP},
    {"sitofp", Opcode::SITOFP},
    {"fptosi", Opcode::FPTOSI},
    {"inttoptr", Opcode::INTTOPTR},
    {"ptrtoint", Opcode::PTRTOINT},
    {"call", Opcode::CALL},
    {"unreachable", Opcode::UNREACHABLE},
    {"ret", Opcode::RET},
    {"br", Opcode::BR},
};