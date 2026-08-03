#ifndef ASSEMBLYGENERATOR_H_
#define ASSEMBLYGENERATOR_H_

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "Instruction.h"
#include "StackMachine.h"

namespace codegen {

class AssemblyGenerator {
public:
    explicit AssemblyGenerator(std::unique_ptr<StackMachine> stackMachine);

    void generateAssemblyCode(const IntermediateRepresentation& ir,
            const std::map<std::string, std::string>& constants,
            const std::vector<GlobalVariable>& globalVariables);

private:
    void emit(const Instruction& instruction);

    std::unique_ptr<StackMachine> stackMachine;
};

} // namespace codegen

#endif // ASSEMBLYGENERATOR_H_
