#ifndef ASSEMBLYGENERATOR_H_
#define ASSEMBLYGENERATOR_H_

#include <map>
#include <memory>
#include <ostream>
#include <string>
#include <vector>

#include "Amd64Registers.h"
#include "Instruction.h"
#include "InstructionSet.h"
#include "StackMachine.h"

namespace codegen {

class AssemblyGenerator {
public:
    AssemblyGenerator(std::ostream* out, std::unique_ptr<InstructionSet> instructions,
            std::unique_ptr<Amd64Registers> registers);

    void generateAssemblyCode(IntermediateRepresentation& ir,
            const std::map<std::string, std::string>& constants,
            const std::vector<GlobalVariable>& globalVariables);

private:
    void emit(const Instruction& instruction);

    std::ostream* out_;
    std::unique_ptr<InstructionSet> instructions_;
    std::unique_ptr<Amd64Registers> registers_;
    std::unique_ptr<StackMachine> stackMachine;
};

} // namespace codegen

#endif // ASSEMBLYGENERATOR_H_
