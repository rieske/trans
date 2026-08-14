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

std::string InstructionSet::asmSymbol(const std::string& name) const {
    return name;
}

std::string InstructionSet::globlDataLine(const std::string& name) const {
    return globl(name) + "\n";
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
        out << dataObjectLines(global);
    }
    out << textSectionHeader();
    return out.str();
}

std::string InstructionSet::dataOperandText(const symbols::StaticInitValue& value) const {
    return std::visit([this](const auto& arm) -> std::string {
        using T = std::decay_t<decltype(arm)>;
        if constexpr (std::is_same_v<T, symbols::StaticWord>) {
            return util::wordImmediate(arm.bits);
        } else if constexpr (std::is_same_v<T, symbols::StaticAddress>) {
            const std::string symbol = asmSymbol(arm.symbol);
            if (arm.addend == 0) {
                return symbol;
            }
            if (arm.addend > 0) {
                return symbol + "+" + std::to_string(arm.addend);
            }
            return symbol + std::to_string(arm.addend);
        } else {
            throw std::logic_error { "dataOperandText expects a storage word or address" };
        }
    }, value);
}

std::string InstructionSet::joinedDataOperands(const GlobalVariable& global) const {
    const auto values = global.initValuesOrZeros();
    std::stringstream out;
    for (std::size_t i = 0; i < values.size(); ++i) {
        if (i > 0) {
            out << ", ";
        }
        out << dataOperandText(values[i]);
    }
    return out.str();
}

} // namespace codegen
