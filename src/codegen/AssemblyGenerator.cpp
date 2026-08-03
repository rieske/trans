#include "AssemblyGenerator.h"

#include <set>
#include <stdexcept>
#include <type_traits>
#include <variant>

namespace codegen {

namespace {

void addAddressRef(const symbols::AddressInit& addr, std::set<std::string>& referenced) {
    if (!addr.symbol.empty()) {
        referenced.insert(addr.symbol);
    }
}

// Single extern list: calls, function addresses, Reference objects, and
// address constants in .data, minus defined procedures, defined data, and
// the string pool.
std::vector<std::string> collectExternalSymbols(const IntermediateRepresentation& ir,
        const std::map<std::string, std::string>& constants,
        const std::vector<GlobalVariable>& globalVariables) {
    std::set<std::string> defined;
    std::set<std::string> referenced;
    for (const auto& constant : constants) {
        defined.insert(constant.first);
    }
    for (const auto& procedure : ir.procedures) {
        defined.insert(procedure.name);
        for (const auto& instruction : procedure.body) {
            if (instruction.op == Op::Call && !instruction.callIndirect) {
                referenced.insert(instruction.arg0);
            } else if (instruction.op == Op::FunctionAddress) {
                referenced.insert(instruction.arg0);
            }
        }
    }
    for (const auto& global : globalVariables) {
        if (global.emission == ObjectEmission::Reference) {
            referenced.insert(global.name);
        } else {
            defined.insert(global.name);
        }
        if (!global.initializer) {
            continue;
        }
        std::visit([&](const auto& arm) {
            using T = std::decay_t<decltype(arm)>;
            if constexpr (std::is_same_v<T, symbols::MultiWordInit>) {
                for (const auto& word : arm.words) {
                    if (const auto* addr = std::get_if<symbols::AddressInit>(&word)) {
                        addAddressRef(*addr, referenced);
                    }
                }
            } else if constexpr (std::is_same_v<T, symbols::AddressInit>) {
                addAddressRef(arm, referenced);
            }
        }, *global.initializer);
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

AssemblyGenerator::AssemblyGenerator(std::unique_ptr<StackMachine> stackMachine) :
        stackMachine { std::move(stackMachine) }
{
}

void AssemblyGenerator::generateAssemblyCode(const IntermediateRepresentation& ir,
        const std::map<std::string, std::string>& constants,
        const std::vector<GlobalVariable>& globalVariables)
{
    std::set<std::string> definedProcedures;
    for (const auto& procedure : ir.procedures) {
        definedProcedures.insert(procedure.name);
    }
    std::vector<std::string> definedFunctions(definedProcedures.begin(), definedProcedures.end());
    stackMachine->generatePreamble(constants, globalVariables,
            collectExternalSymbols(ir, constants, globalVariables), definedFunctions);
    for (const auto& procedure : ir.procedures) {
        stackMachine->startProcedure(procedure);
        for (const auto& instruction : procedure.body) {
            emit(instruction);
            // Match packFrameValues ordinals so dead temps do not spill into reused slots.
            stackMachine->finishInstruction();
        }
        stackMachine->endProcedure();
    }
}

void AssemblyGenerator::emit(const Instruction& instruction) {
    switch (instruction.op) {
    case Op::Label:
        stackMachine->label(instruction.arg0);
        return;
    case Op::Jump:
        stackMachine->jump(instruction.cond, instruction.arg0);
        return;
    case Op::ValueCompare:
        stackMachine->compare(instruction.arg0, instruction.arg1, instruction.imm != 0);
        return;
    case Op::ZeroCompare:
        stackMachine->zeroCompare(instruction.arg0);
        return;
    case Op::AddressOf:
        stackMachine->addressOf(instruction.arg0, instruction.result);
        return;
    case Op::Dereference:
        stackMachine->dereference(instruction.arg0, instruction.arg1, instruction.result);
        return;
    case Op::IndexAddress:
        stackMachine->indexAddress(
                instruction.arg0, instruction.arg1, instruction.imm, instruction.result, instruction.baseMode);
        return;
    case Op::FieldAddress:
        stackMachine->fieldAddress(
                instruction.arg0, instruction.imm, instruction.result, instruction.baseMode);
        return;
    case Op::CopyPart:
        stackMachine->copyPart(instruction.arg0, instruction.result, instruction.imm);
        return;
    case Op::FunctionAddress:
        stackMachine->functionAddress(instruction.arg0, instruction.result);
        return;
    case Op::UnaryMinus:
        stackMachine->unaryMinus(instruction.arg0, instruction.result);
        return;
    case Op::UnaryNot:
        stackMachine->bitwiseNot(instruction.arg0, instruction.result);
        return;
    case Op::Assign:
        stackMachine->assign(instruction.arg0, instruction.result);
        return;
    case Op::Widen:
        stackMachine->widenInteger(instruction.arg0, instruction.result, instruction.imm != 0);
        return;
    case Op::AssignConstant:
        stackMachine->assignConstant(instruction.arg0, instruction.result, instruction.arg1);
        return;
    case Op::AssignLabelAddress:
        stackMachine->assignLabelAddress(instruction.arg0, instruction.result);
        return;
    case Op::LvalueAssign:
        stackMachine->lvalueAssign(instruction.arg0, instruction.result, instruction.accessSizeBytes);
        return;
    case Op::Argument:
        stackMachine->procedureArgument(instruction.arg0);
        return;
    case Op::Call:
        if (instruction.callIndirect) {
            stackMachine->callProcedureIndirect(instruction.arg0, instruction.memoryReturnDest);
        } else {
            stackMachine->callProcedure(instruction.arg0, instruction.memoryReturnDest);
        }
        return;
    case Op::Return:
        stackMachine->returnFromProcedure(instruction.arg0);
        return;
    case Op::VoidReturn:
        stackMachine->returnFromProcedure();
        return;
    case Op::Retrieve:
        stackMachine->retrieveProcedureReturnValue(instruction.result, instruction.memoryReturn);
        return;
    case Op::Xor:
        stackMachine->xorCommand(instruction.arg0, instruction.arg1, instruction.result);
        return;
    case Op::Or:
        stackMachine->orCommand(instruction.arg0, instruction.arg1, instruction.result);
        return;
    case Op::And:
        stackMachine->andCommand(instruction.arg0, instruction.arg1, instruction.result);
        return;
    case Op::Add:
        stackMachine->add(instruction.arg0, instruction.arg1, instruction.result);
        return;
    case Op::Sub:
        stackMachine->sub(instruction.arg0, instruction.arg1, instruction.result);
        return;
    case Op::Mul:
        stackMachine->mul(instruction.arg0, instruction.arg1, instruction.result);
        return;
    case Op::Div:
        stackMachine->div(instruction.arg0, instruction.arg1, instruction.result, instruction.unsignedArith);
        return;
    case Op::Mod:
        stackMachine->mod(instruction.arg0, instruction.arg1, instruction.result, instruction.unsignedArith);
        return;
    case Op::Inc:
        stackMachine->inc(instruction.arg0, instruction.imm);
        return;
    case Op::Dec:
        stackMachine->dec(instruction.arg0, instruction.imm);
        return;
    case Op::Shl:
        stackMachine->shl(instruction.arg0, instruction.arg1, instruction.result);
        return;
    case Op::Shr:
        stackMachine->shr(instruction.arg0, instruction.arg1, instruction.result, instruction.logicalShift);
        return;
    case Op::Truncate:
        stackMachine->truncate(instruction.arg0, instruction.imm, instruction.signedAccess);
        return;
    case Op::VaStart:
        stackMachine->vaStart(instruction.arg0);
        return;
    case Op::VaArg:
        stackMachine->vaArg(instruction.arg0, instruction.result);
        return;
    case Op::VaCopy:
        stackMachine->vaCopy(instruction.arg0, instruction.arg1);
        return;
    case Op::Bswap:
        stackMachine->bswap(instruction.arg0, instruction.result, instruction.imm);
        return;
    case Op::Ctz:
        stackMachine->ctz(instruction.arg0, instruction.result, instruction.imm);
        return;
    case Op::Alloca:
        stackMachine->allocaBytes(instruction.arg0, instruction.result);
        return;
    }
    throw std::logic_error { "AssemblyGenerator::emit: unhandled Op" };
}

} // namespace codegen
