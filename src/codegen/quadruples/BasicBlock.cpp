#include "BasicBlock.h"

namespace codegen {

BasicBlock::BasicBlock()
{}

void BasicBlock::generateCode(AssemblyGenerator& generator) const {
    for (const auto& inst : instructions) {
        inst->generateCode(generator);
    }
};

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

void BasicBlock::setSuccessor(BasicBlock* successor) {
    hasTerminator = true;
    this->successor = successor;
}

void BasicBlock::print(std::ostream& stream) const {
    for (const auto& inst : instructions) {
        stream << *inst;
    }
}

} /* namespace codegen */
