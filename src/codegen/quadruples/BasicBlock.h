#ifndef BASIC_BLOCK_H_
#define BASIC_BLOCK_H_

#include "Quadruple.h"

#include <vector>
#include <memory>

namespace codegen {

class BasicBlock : public Quadruple {
public:
    BasicBlock();
    virtual ~BasicBlock() = default;
    void generateCode(AssemblyGenerator& generator) const override;

    bool terminates() const;

    void appendInstruction(std::unique_ptr<Quadruple> instruction);
    // Mark this block as ending a CFG edge (fall-through into the next block).
    void markTerminates();

    // Instructions nested inside this block (Call/FunctionAddress live here).
    const std::vector<std::unique_ptr<Quadruple>>& getInstructions() const;

protected:
    void print(std::ostream& stream) const override;

private:
    std::vector<std::unique_ptr<Quadruple>> instructions;
    bool hasTerminator {false};
};

} // namespace codegen

#endif // BASIC_BLOCK_H_
