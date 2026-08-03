#include "AssemblyGenerator.h"

#include <cctype>
#include <set>
#include <stdexcept>

namespace codegen {

namespace {

void collectProcedureSymbols(const IntermediateRepresentation& ir,
        std::set<std::string>& defined,
        std::set<std::string>& staticDefined,
        std::set<std::string>& referenced) {
    for (const auto& procedure : ir.procedures) {
        defined.insert(procedure.name);
        if (procedure.isStatic) {
            staticDefined.insert(procedure.name);
        }
        for (const auto& instruction : procedure.body) {
            if (instruction.op == Op::Call && !instruction.callIndirect) {
                referenced.insert(instruction.arg0);
            } else if (instruction.op == Op::FunctionAddress) {
                referenced.insert(instruction.arg0);
            }
        }
    }
}

// Bare identifier used as a .data operand (function/variable address). Numbers,
// hex literals, and $prefixed local constants are not external symbols.
// Address constants may be symbol+offset (NASM: dq arr+8 for &arr[1]).
bool isBareSymbolOperand(const std::string& word) {
    if (word.empty() || word[0] == '$') {
        return false;
    }
    // Strip trailing +N / -N so arr+8 still names symbol arr for extern.
    std::string base = word;
    auto plus = base.find('+');
    auto minus = base.find('-');
    std::size_t cut = std::string::npos;
    if (plus != std::string::npos) {
        cut = plus;
    }
    if (minus != std::string::npos && (cut == std::string::npos || minus < cut)) {
        cut = minus;
    }
    if (cut != std::string::npos) {
        // Require a non-empty offset of digits only.
        if (cut + 1 >= base.size()) {
            return false;
        }
        for (std::size_t i = cut + 1; i < base.size(); ++i) {
            if (!std::isdigit(static_cast<unsigned char>(base[i]))) {
                return false;
            }
        }
        base = base.substr(0, cut);
    }
    if (base.empty()
            || !(std::isalpha(static_cast<unsigned char>(base[0])) || base[0] == '_')) {
        return false;
    }
    for (unsigned char c : base) {
        if (!(std::isalnum(c) || c == '_')) {
            return false;
        }
    }
    return true;
}

// Function addresses in multi-word / scalar global initializers never produce
// Call/FunctionAddress ops, so collect those bare names for `extern` too
// (git git.c: commands[] holds cmd_add, cmd_am, ...).
void collectGlobalDataSymbolRefs(const std::vector<GlobalVariable>& globalVariables,
        const std::map<std::string, std::string>& /*constants*/,
        const std::set<std::string>& definedLocally,
        std::set<std::string>& referenced) {
    auto maybeRef = [&](const std::string& word) {
        if (!isBareSymbolOperand(word)) {
            return;
        }
        // Use base symbol for arr+N address constants.
        std::string base = word;
        auto plus = base.find('+');
        auto minus = base.find('-');
        std::size_t cut = std::string::npos;
        if (plus != std::string::npos) {
            cut = plus;
        }
        if (minus != std::string::npos && (cut == std::string::npos || minus < cut)) {
            cut = minus;
        }
        if (cut != std::string::npos) {
            base = base.substr(0, cut);
        }
        if (definedLocally.find(base) == definedLocally.end()) {
            referenced.insert(base);
        }
    };
    for (const auto& global : globalVariables) {
        if (global.isExternal) {
            continue;
        }
        if (global.multiWordInitializer) {
            for (const auto& word : *global.multiWordInitializer) {
                maybeRef(word);
            }
        } else if (!global.stringInitializer && isBareSymbolOperand(global.initializerLiteral)) {
            // Scalar address initializer: T *p = other; or T (*fp)() = f;
            maybeRef(global.initializerLiteral);
        }
    }
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
    // Collect defined procedures and direct callees so the preamble can emit
    // `extern` for symbols resolved at link time. Static functions are defined
    // but not exported (no `global` directive).
    std::set<std::string> defined;
    std::set<std::string> staticDefined;
    std::set<std::string> referenced;
    collectProcedureSymbols(ir, defined, staticDefined, referenced);
    // Address constants in .data may name other globals in this TU; do not
    // emit `extern` for those (NASM: label inconsistently redefined).
    std::set<std::string> definedNames = defined;
    for (const auto& global : globalVariables) {
        if (!global.isExternal) {
            definedNames.insert(global.name);
        }
    }
    collectGlobalDataSymbolRefs(globalVariables, constants, definedNames, referenced);
    std::vector<std::string> externalFunctions;
    for (const auto& name : referenced) {
        if (defined.find(name) == defined.end()) {
            externalFunctions.push_back(name);
        }
    }
    // Non-static defined functions are listed as-is; static ones get a '.' prefix
    // so the instruction set can skip the global directive while still knowing
    // the name is defined in this unit.
    std::vector<std::string> definedFunctions;
    for (const auto& name : defined) {
        if (staticDefined.count(name)) {
            definedFunctions.push_back("." + name);
        } else {
            definedFunctions.push_back(name);
        }
    }
    stackMachine->generatePreamble(constants, globalVariables, externalFunctions, definedFunctions);
    for (const auto& procedure : ir.procedures) {
        stackMachine->startProcedure(procedure.name, procedure.frame.locals, procedure.frame.arguments,
                procedure.variadic, procedure.memoryReturn);
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
        stackMachine->compare(instruction.arg0, instruction.arg1);
        return;
    case Op::ZeroCompare:
        stackMachine->zeroCompare(instruction.arg0);
        return;
    case Op::AddressOf:
        stackMachine->addressOf(instruction.arg0, instruction.result);
        return;
    case Op::Dereference:
        stackMachine->dereference(instruction.arg0, instruction.arg1, instruction.result,
                instruction.accessSizeBytes, instruction.signedAccess);
        return;
    case Op::IndexAddress:
        stackMachine->indexAddress(
                instruction.arg0, instruction.arg1, instruction.imm, instruction.result, instruction.baseMode);
        return;
    case Op::FieldAddress:
        stackMachine->fieldAddress(
                instruction.arg0, instruction.imm, instruction.result, instruction.baseMode);
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
    case Op::AssignConstant:
        stackMachine->assignConstant(instruction.arg0, instruction.result);
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
    case Op::BuiltinOp: {
        using OK = symbols::BuiltinOpKind;
        switch (instruction.builtinKind) {
        case OK::Bswap16:
            stackMachine->bswap(instruction.arg0, instruction.result, 2);
            break;
        case OK::Bswap32:
            stackMachine->bswap(instruction.arg0, instruction.result, 4);
            break;
        case OK::Bswap64:
            stackMachine->bswap(instruction.arg0, instruction.result, 8);
            break;
        case OK::Ctz:
            stackMachine->ctz(instruction.arg0, instruction.result);
            break;
        }
        return;
    }
    case Op::VaStart:
        stackMachine->vaStart(instruction.arg0, instruction.arg1);
        return;
    case Op::VaArg:
        stackMachine->vaArg(instruction.arg0, instruction.result, instruction.accessSizeBytes,
                instruction.floatingAccess, instruction.signedAccess);
        return;
    case Op::VaCopy:
        stackMachine->vaCopy(instruction.arg0, instruction.arg1);
        return;
    case Op::VaEnd:
        return;
    }
    throw std::logic_error { "AssemblyGenerator::emit: unhandled Op" };
}

} // namespace codegen
