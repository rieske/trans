#include "Instructions.h"



namespace codegen {

Mov::Mov(std::string_view source, std::string_view destination):
source { source },
destination { destination }
{}

MovRegReg::MovRegReg(const Register& source, const Register& destination):
Mov { source.getName(), destination.getName() }
{}

void MovRegReg::visit(InstructionSet& instructionSet) const {
	//instructionSet.mov(source, destination);
}

}
