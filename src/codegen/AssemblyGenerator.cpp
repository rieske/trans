#include "AssemblyGenerator.h"

#include <algorithm>
#include <string>
#include <vector>

#include "Value.h"
#include "InstructionSet.h"

namespace codegen {

AssemblyGenerator::AssemblyGenerator(std::unique_ptr<StackMachine> stackMachine) :
        stackMachine { std::move(stackMachine) }
{
}

void AssemblyGenerator::generateAssemblyCode(std::vector<std::unique_ptr<Quadruple> > quadruples) {
    stackMachine->generatePreamble();
    for (const auto& quadruple : quadruples) {
        quadruple->generateCode(*this);
    }
}

void AssemblyGenerator::generateCodeFor(const StartScope& startScope) {
    if (startScope.getValues().size() != 0) {
        stackMachine->allocateStack(startScope.getValues(), startScope.getArguments());
    }
}

void AssemblyGenerator::generateCodeFor(const EndScope& endScope) {
    if (endScope.getScopeSize() != 0) {
        stackMachine->deallocateStack();
    }
}

void AssemblyGenerator::generateCodeFor(const Input& input) {
    stackMachine->callInputProcedure(input.getInputSymbolName());
}

void AssemblyGenerator::generateCodeFor(const Output& output) {
    stackMachine->callOutputProcedure(output.getOutputSymbolName());
}

void AssemblyGenerator::generateCodeFor(const StartProcedure& startProcedure) {
    stackMachine->startProcedure(startProcedure.getName());
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

void AssemblyGenerator::generateCodeFor(const Dereference& dereference) {
    stackMachine->dereference(dereference.getOperand(), dereference.getLvalue(), dereference.getResult());
}

void AssemblyGenerator::generateCodeFor(const UnaryMinus& unaryMinus) {
    stackMachine->unaryMinus(unaryMinus.getOperand(), unaryMinus.getResult());
}

void AssemblyGenerator::generateCodeFor(const Assign& assign) {
    stackMachine->assign(assign.getOperand(), assign.getResult());
}

void AssemblyGenerator::generateCodeFor(const AssignConstant& assignConstant) {
    stackMachine->assignConstant(assignConstant.getConstant(), assignConstant.getResult());
}

void AssemblyGenerator::generateCodeFor(const LvalueAssign& lvalueAssign) {
    stackMachine->lvalueAssign(lvalueAssign.getOperand(), lvalueAssign.getResult());
}

void AssemblyGenerator::generateCodeFor(const Argument& argument) {
    stackMachine->procedureArgument(argument.getArgumentName());
}

void codegen::AssemblyGenerator::generateCodeFor(const Call& call) {
    stackMachine->callProcedure(call.getProcedureName());
}

void codegen::AssemblyGenerator::generateCodeFor(const Return& returnCommand) {
    stackMachine->returnFromProcedure(returnCommand.getReturnSymbolName());
}

void AssemblyGenerator::generateCodeFor(const Retrieve& retrieve) {
    stackMachine->retrieveProcedureReturnValue(retrieve.getResultName());
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
    stackMachine->div(div.getLeftOperandName(), div.getRightOperandName(), div.getResultName());
}

void AssemblyGenerator::generateCodeFor(const Mod& mod) {
    stackMachine->mod(mod.getLeftOperandName(), mod.getRightOperandName(), mod.getResultName());
}

void AssemblyGenerator::generateCodeFor(const Inc& inc) {
    stackMachine->inc(inc.getOperandName());
}

void AssemblyGenerator::generateCodeFor(const Dec& dec) {
    stackMachine->dec(dec.getOperandName());
}

} /* namespace code_generator */
