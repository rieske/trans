#include "AssemblyGenerator.h"

#include "quadruples/BasicBlock.h"

#include <cctype>
#include <set>

namespace codegen {

namespace {

// Walk top-level quads and instructions nested in BasicBlocks. After lowering,
// Call/FunctionAddress sit inside BasicBlock, not at the outer list.
void collectProcedureSymbols(const std::vector<std::unique_ptr<Quadruple>>& quadruples,
        std::set<std::string>& defined,
        std::set<std::string>& staticDefined,
        std::set<std::string>& referenced) {
    for (const auto& quadruple : quadruples) {
        if (auto* start = dynamic_cast<const StartProcedure*>(quadruple.get())) {
            defined.insert(start->getName());
            if (start->isStatic()) {
                staticDefined.insert(start->getName());
            }
        } else if (auto* call = dynamic_cast<const Call*>(quadruple.get())) {
            if (!call->isIndirect()) {
                referenced.insert(call->getProcedureName());
            }
        } else if (auto* fnAddr = dynamic_cast<const FunctionAddress*>(quadruple.get())) {
            referenced.insert(fnAddr->getOperand());
        } else if (auto* block = dynamic_cast<const BasicBlock*>(quadruple.get())) {
            collectProcedureSymbols(block->getInstructions(), defined, staticDefined, referenced);
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
// Call/FunctionAddress quads, so collect those bare names for `extern` too
// (git git.c: commands[] holds cmd_add, cmd_am, ...).
void collectGlobalDataSymbolRefs(const std::vector<GlobalVariable>& globalVariables,
        const std::map<std::string, std::string>& constants,
        const std::set<std::string>& definedProcedures,
        std::set<std::string>& referenced) {
    std::set<std::string> definedLocally = definedProcedures;
    for (const auto& global : globalVariables) {
        if (!global.isExternal) {
            definedLocally.insert(global.name);
        }
    }
    for (const auto& constant : constants) {
        definedLocally.insert(constant.first);
    }
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

void AssemblyGenerator::generateAssemblyCode(
        std::vector<std::unique_ptr<Quadruple> > quadruples,
        const std::map<std::string, std::string>& constants,
        const std::vector<GlobalVariable>& globalVariables)
{
    // Collect defined procedures and direct callees so the preamble can emit
    // `extern` for symbols resolved at link time. Static functions are defined
    // but not exported (no `global` directive).
    std::set<std::string> defined;
    std::set<std::string> staticDefined;
    std::set<std::string> referenced;
    collectProcedureSymbols(quadruples, defined, staticDefined, referenced);
    collectGlobalDataSymbolRefs(globalVariables, constants, defined, referenced);
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
    for (const auto& quadruple : quadruples) {
        quadruple->generateCode(*this);
    }
}

void AssemblyGenerator::generateCodeFor(const StartProcedure& startProcedure) {
    stackMachine->startProcedure(startProcedure.getName(), startProcedure.getValues(), startProcedure.getArguments(),
            startProcedure.isVariadic(), startProcedure.hasMemoryReturn());
}

void AssemblyGenerator::finishInstruction() {
    stackMachine->finishInstruction();
}

void AssemblyGenerator::generateCodeFor(const EndProcedure&) {
    stackMachine->endProcedure();
}

void AssemblyGenerator::generateCodeFor(const Label& label) {
    stackMachine->label(label.getName());
}

void AssemblyGenerator::generateCodeFor(const Jump& jump) {
    stackMachine->jump(jump.getCondition(), jump.getLabel());
}

void AssemblyGenerator::generateCodeFor(const ValueCompare& valueCompare) {
    stackMachine->compare(valueCompare.getLeftSymbolName(), valueCompare.getRightSymbolName());
}

void AssemblyGenerator::generateCodeFor(const ZeroCompare& zeroCompare) {
    stackMachine->zeroCompare(zeroCompare.getSymbolName());
}

void AssemblyGenerator::generateCodeFor(const AddressOf& addressOf) {
    stackMachine->addressOf(addressOf.getOperand(), addressOf.getResult());
}

void AssemblyGenerator::generateCodeFor(const FunctionAddress& functionAddress) {
    stackMachine->functionAddress(functionAddress.getOperand(), functionAddress.getResult());
}

void AssemblyGenerator::generateCodeFor(const Dereference& dereference) {
    stackMachine->dereference(dereference.getOperand(), dereference.getLvalue(), dereference.getResult(),
            dereference.getAccessSizeBytes(), dereference.isSignedAccess());
}

void AssemblyGenerator::generateCodeFor(const UnaryMinus& unaryMinus) {
    stackMachine->unaryMinus(unaryMinus.getOperand(), unaryMinus.getResult());
}

void AssemblyGenerator::generateCodeFor(const BitwiseNot& bitwiseNot) {
    stackMachine->bitwiseNot(bitwiseNot.getOperand(), bitwiseNot.getResult());
}

void AssemblyGenerator::generateCodeFor(const Assign& assign) {
    stackMachine->assign(assign.getOperand(), assign.getResult());
}

void AssemblyGenerator::generateCodeFor(const AssignConstant& assignConstant) {
    stackMachine->assignConstant(assignConstant.getConstant(), assignConstant.getResult());
}

void AssemblyGenerator::generateCodeFor(const LvalueAssign& lvalueAssign) {
    stackMachine->lvalueAssign(lvalueAssign.getOperand(), lvalueAssign.getResult(),
            lvalueAssign.getAccessSizeBytes());
}

void AssemblyGenerator::generateCodeFor(const Argument& argument) {
    stackMachine->procedureArgument(argument.getArgumentName());
}

void codegen::AssemblyGenerator::generateCodeFor(const Call& call) {
    if (call.isIndirect()) {
        stackMachine->callProcedureIndirect(call.getProcedureName(), call.getMemoryReturnDest());
    } else {
        stackMachine->callProcedure(call.getProcedureName(), call.getMemoryReturnDest());
    }
}

void codegen::AssemblyGenerator::generateCodeFor(const Return& returnCommand) {
    stackMachine->returnFromProcedure(returnCommand.getReturnSymbolName());
}

void codegen::AssemblyGenerator::generateCodeFor(const VoidReturn& returnCommand) {
    stackMachine->returnFromProcedure();
}

void AssemblyGenerator::generateCodeFor(const Retrieve& retrieve) {
    stackMachine->retrieveProcedureReturnValue(retrieve.getResultName(), retrieve.isMemoryReturn());
}

void AssemblyGenerator::generateCodeFor(const Xor& xorCommand) {
    stackMachine->xorCommand(xorCommand.getLeftOperandName(), xorCommand.getRightOperandName(), xorCommand.getResultName());
}

void AssemblyGenerator::generateCodeFor(const Or& orCommand) {
    stackMachine->orCommand(orCommand.getLeftOperandName(), orCommand.getRightOperandName(), orCommand.getResultName());
}

void AssemblyGenerator::generateCodeFor(const And& andCommand) {
    stackMachine->andCommand(andCommand.getLeftOperandName(), andCommand.getRightOperandName(), andCommand.getResultName());
}

void AssemblyGenerator::generateCodeFor(const Add& add) {
    stackMachine->add(add.getLeftOperandName(), add.getRightOperandName(), add.getResultName());
}

void AssemblyGenerator::generateCodeFor(const Sub& sub) {
    stackMachine->sub(sub.getLeftOperandName(), sub.getRightOperandName(), sub.getResultName());
}

void AssemblyGenerator::generateCodeFor(const Mul& mul) {
    stackMachine->mul(mul.getLeftOperandName(), mul.getRightOperandName(), mul.getResultName());
}

void AssemblyGenerator::generateCodeFor(const Div& div) {
    stackMachine->div(div.getLeftOperandName(), div.getRightOperandName(), div.getResultName(), div.isUnsigned());
}

void AssemblyGenerator::generateCodeFor(const Mod& mod) {
    stackMachine->mod(mod.getLeftOperandName(), mod.getRightOperandName(), mod.getResultName(), mod.isUnsigned());
}

void AssemblyGenerator::generateCodeFor(const Inc& inc) {
    stackMachine->inc(inc.getOperandName(), inc.getAmount());
}

void AssemblyGenerator::generateCodeFor(const Dec& dec) {
    stackMachine->dec(dec.getOperandName(), dec.getAmount());
}

void AssemblyGenerator::generateCodeFor(const Shl& shl) {
    stackMachine->shl(shl.getLeftOperandName(), shl.getRightOperandName(), shl.getResultName());
}

void AssemblyGenerator::generateCodeFor(const Shr& shr) {
    stackMachine->shr(shr.getLeftOperandName(), shr.getRightOperandName(), shr.getResultName(), shr.isLogical());
}





void AssemblyGenerator::generateCodeFor(const FieldAddress& fieldAddress) {
    stackMachine->fieldAddress(fieldAddress.getBase(), fieldAddress.getOffsetBytes(), fieldAddress.getResult(), fieldAddress.baseIsPointer());
}

void AssemblyGenerator::generateCodeFor(const IndexAddress& indexAddress) {
    stackMachine->indexAddress(indexAddress.getBase(), indexAddress.getIndex(), indexAddress.getElementSizeBytes(),
            indexAddress.getResult(), indexAddress.baseIsArray());
}

void AssemblyGenerator::generateCodeFor(const Truncate& truncate) {
    stackMachine->truncate(truncate.getOperandName(), truncate.getSizeBytes(), truncate.isSigned());
}

void AssemblyGenerator::generateCodeFor(const BuiltinOp& builtinOp) {
    switch (builtinOp.getKind()) {
    case BuiltinOp::Kind::Bswap16:
        stackMachine->bswap(builtinOp.getOperand(), builtinOp.getResult(), 2);
        break;
    case BuiltinOp::Kind::Bswap32:
        stackMachine->bswap(builtinOp.getOperand(), builtinOp.getResult(), 4);
        break;
    case BuiltinOp::Kind::Bswap64:
        stackMachine->bswap(builtinOp.getOperand(), builtinOp.getResult(), 8);
        break;
    case BuiltinOp::Kind::Ctz:
        stackMachine->ctz(builtinOp.getOperand(), builtinOp.getResult());
        break;
    }
}

void AssemblyGenerator::generateCodeFor(const VaOp& vaOp) {
    switch (vaOp.getKind()) {
    case VaOp::Kind::Start:
        stackMachine->vaStart(vaOp.getApPtr(), vaOp.getLastAddr());
        break;
    case VaOp::Kind::Arg:
        stackMachine->vaArg(vaOp.getApPtr(), vaOp.getResult(), vaOp.getAccessSizeBytes(),
                vaOp.isFloating(), vaOp.isSignedAccess());
        break;
    case VaOp::Kind::Copy:
        stackMachine->vaCopy(vaOp.getApPtr(), vaOp.getSrcPtr());
        break;
    case VaOp::Kind::End:
        break;
    }
}

} // namespace codegen
