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
    void setSuccessor(BasicBlock* successor);

protected:
    void print(std::ostream& stream) const override;

private:
    std::vector<std::unique_ptr<Quadruple>> instructions;
    BasicBlock* successor;
    bool hasTerminator {false};
};

} /* namespace codegen */

#endif /* BASIC_BLOCK_H_ */
