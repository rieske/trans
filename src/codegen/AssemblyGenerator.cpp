#include "AssemblyGenerator.h"

#include <set>
#include <variant>

namespace codegen {

namespace {

std::vector<std::string> collectExternalSymbols(const IntermediateRepresentation& ir,
        const std::map<std::string, std::string>& constants,
        const std::vector<GlobalVariable>& globalVariables) {
    std::set<std::string> defined;
    std::set<std::string> referenced;
    for (const auto& constant : constants) {
        defined.insert(constant.first);
    }
    for (const auto& procedure : ir.procedures) {
        defined.insert(ir.strings.get(procedure.name));
        for (const auto& instruction : procedure.body) {
            if (instruction.op == Op::Call && !instruction.callIndirect) {
                referenced.insert(ir.strings.get(instruction.arg0));
            } else if (instruction.op == Op::FunctionAddress) {
                referenced.insert(ir.strings.get(instruction.arg0));
            }
        }
    }
    for (const auto& global : globalVariables) {
        if (global.emission == ObjectEmission::Reference) {
            referenced.insert(global.name);
        } else {
            defined.insert(global.name);
        }
        for (const auto& word : global.initValues) {
            if (const auto* addr = std::get_if<symbols::StaticAddress>(&word)) {
                referenced.insert(addr->symbol);
            }
        }
    }
    std::vector<std::string> external;
    for (const auto& name : referenced) {
        if (!defined.count(name)) {
            external.push_back(name);
        }
    }
    return external;
}

} // namespace

AssemblyGenerator::AssemblyGenerator(std::ostream* out, std::unique_ptr<InstructionSet> instructions,
        std::unique_ptr<Amd64Registers> registers) :
        out_ { out },
        instructions_ { std::move(instructions) },
        registers_ { std::move(registers) }
{
}

void AssemblyGenerator::generateAssemblyCode(IntermediateRepresentation& ir,
        const std::map<std::string, std::string>& constants,
        const std::vector<GlobalVariable>& globalVariables)
{
    for (const auto& global : globalVariables) {
        ir.strings.intern(global.name);
    }
    stackMachine = std::make_unique<StackMachine>(out_, *instructions_, *registers_, ir.strings);
    stackMachine->generatePreamble(constants, globalVariables,
            collectExternalSymbols(ir, constants, globalVariables));
    for (const auto& procedure : ir.procedures) {
        stackMachine->registerDefinedProcedure(procedure.name);
    }
    for (const auto& procedure : ir.procedures) {
        stackMachine->startProcedure(procedure);
        for (const auto& instruction : procedure.body) {
            stackMachine->emit(instruction);
        }
        stackMachine->endProcedure();
    }
}

} // namespace codegen
