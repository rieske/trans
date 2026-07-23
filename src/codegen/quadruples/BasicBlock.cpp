#include "BasicBlock.h"

#include "codegen/AssemblyGenerator.h"

namespace codegen {

BasicBlock::BasicBlock()
{}

void BasicBlock::generateCode(AssemblyGenerator& generator) const {
    for (const auto& inst : instructions) {
        inst->generateCode(generator);
        // Match packFrameValues ordinals so dead temps do not spill into reused slots.
        generator.finishInstruction();
    }
}

bool BasicBlock::terminates() const {
    return hasTerminator;
}

void BasicBlock::appendInstruction(std::unique_ptr<Quadruple> instruction) {
    if (hasTerminator || (instruction->isLabel() && instructions.size())) {
        throw std::invalid_argument{"Can not start a new basic block"};
    }
    if (instruction->transfersControl()) {
        hasTerminator = true;
    }
    instructions.push_back(std::move(instruction));
}

const std::vector<std::unique_ptr<Quadruple>>& BasicBlock::getInstructions() const {
    return instructions;
}

void BasicBlock::markTerminates() {
    hasTerminator = true;
}

void BasicBlock::print(std::ostream& stream) const {
    for (const auto& inst : instructions) {
        stream << *inst;
    }
}

} // namespace codegen

