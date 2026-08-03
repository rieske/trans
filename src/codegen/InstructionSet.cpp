#include "InstructionSet.h"

#include "util/ImmediateFormat.h"

#include <sstream>
#include <stdexcept>
#include <type_traits>
#include <variant>

namespace codegen {

std::string InstructionSet::preamblePrefix() const {
    return {};
}

std::string InstructionSet::globlDataLine(const std::string& name) const {
    return globl(name) + "\n";
}

std::string InstructionSet::asmSymbol(const std::string& name) const {
    return name;
}

std::vector<std::string> InstructionSet::formattedDataOperands(const GlobalVariable& global) const {
    std::vector<std::string> operands;
    for (const auto& word : global.dataWords()) {
        operands.push_back(std::visit([&](const auto& arm) {
            using T = std::decay_t<decltype(arm)>;
            if constexpr (std::is_same_v<T, symbols::ConstantInit>) {
                return util::wordImmediate(static_cast<unsigned long long>(arm.value));
            } else {
                std::string s = asmSymbol(arm.symbol);
                if (arm.offset > 0) {
                    s += "+" + std::to_string(arm.offset);
                } else if (arm.offset < 0) {
                    s += std::to_string(arm.offset);
                }
                return s;
            }
        }, word));
    }
    return operands;
}

std::string InstructionSet::preamble(const std::map<std::string, std::string>& constants,
        const std::vector<GlobalVariable>& globalVariables,
        const std::vector<std::string>& externalSymbols) const {
    std::stringstream out;
    out << preamblePrefix();
    for (const auto& name : externalSymbols) {
        out << externDirective(name) << "\n";
    }
    out << dataSectionHeader();
    for (const auto& constant : constants) {
        out << constantLine(constant.first, constant.second);
    }
    for (const auto& global : globalVariables) {
        if (global.emission == ObjectEmission::Reference) {
            continue;
        }
        if (global.emission == ObjectEmission::DefineExternal) {
            out << globlDataLine(global.name);
        }
        if (global.alignBytes > 1) {
            out << alignDirective(global.alignBytes);
        }
        out << dataObjectLines(global);
    }
    out << textSectionHeader();
    return out.str();
}

std::string InstructionSet::call(std::string procedureName) const {
    return "call " + asmSymbol(procedureName);
}

std::string InstructionSet::label(std::string name) const {
    return asmSymbol(name) + ":";
}

std::string InstructionSet::jmp(std::string label) const {
    return "jmp " + asmSymbol(label);
}

std::string InstructionSet::je(std::string label) const {
    return "je " + asmSymbol(label);
}

std::string InstructionSet::jne(std::string label) const {
    return "jne " + asmSymbol(label);
}

std::string InstructionSet::jg(std::string label) const {
    return "jg " + asmSymbol(label);
}

std::string InstructionSet::jl(std::string label) const {
    return "jl " + asmSymbol(label);
}

std::string InstructionSet::jge(std::string label) const {
    return "jge " + asmSymbol(label);
}

std::string InstructionSet::jle(std::string label) const {
    return "jle " + asmSymbol(label);
}

std::string InstructionSet::ja(std::string label) const {
    return "ja " + asmSymbol(label);
}

std::string InstructionSet::jb(std::string label) const {
    return "jb " + asmSymbol(label);
}

std::string InstructionSet::jae(std::string label) const {
    return "jae " + asmSymbol(label);
}

std::string InstructionSet::jbe(std::string label) const {
    return "jbe " + asmSymbol(label);
}

std::string InstructionSet::leave() const {
    return "leave";
}

std::string InstructionSet::ret() const {
    return "ret";
}

std::string InstructionSet::fchs() const {
    return "fchs";
}

std::string InstructionSet::fldz() const {
    return "fldz";
}

namespace {

AccessWidth widthFromBytes(const char* op, int sizeBytes) {
    if (sizeBytes == 1) {
        return AccessWidth::B1;
    }
    if (sizeBytes == 2) {
        return AccessWidth::B2;
    }
    if (sizeBytes == 4) {
        return AccessWidth::B4;
    }
    if (sizeBytes == 8) {
        return AccessWidth::B8;
    }
    throw std::runtime_error {
            std::string(op) + ": unsupported size " + std::to_string(sizeBytes) };
}

} // namespace

std::string InstructionSet::load(const MemoryOperand& source, const Register& dest, int sizeBytes,
        bool isSigned) const {
    return loadW(source, dest, widthFromBytes("load", sizeBytes), isSigned);
}

std::string InstructionSet::store(const Register& source, const MemoryOperand& dest, int sizeBytes) const {
    return storeW(source, dest, widthFromBytes("store", sizeBytes));
}

std::string InstructionSet::inc(const MemoryOperand& operand, int sizeBytes) const {
    return incW(operand, widthFromBytes("inc", sizeBytes));
}

std::string InstructionSet::dec(const MemoryOperand& operand, int sizeBytes) const {
    return decW(operand, widthFromBytes("dec", sizeBytes));
}

std::string InstructionSet::extend(const Register& reg, int sizeBytes, bool isSigned) const {
    const AccessWidth w = widthFromBytes("extend", sizeBytes);
    if (w == AccessWidth::B8) {
        throw std::runtime_error { "extend: unsupported size 8" };
    }
    return extendW(reg, w, isSigned);
}

std::string InstructionSet::storeImm(const MemoryOperand& dest, long long imm, int sizeBytes) const {
    return storeImmW(dest, imm, widthFromBytes("storeImm", sizeBytes));
}

std::vector<std::string> InstructionSet::bswap(const Register& operand, int widthBytes) const {
    const AccessWidth w = widthFromBytes("bswap", widthBytes);
    if (w == AccessWidth::B1) {
        throw std::runtime_error { "bswap: unsupported size 1" };
    }
    return bswapW(operand, w);
}

} // namespace codegen
