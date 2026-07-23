#ifndef LVALUEASSIGN_H_
#define LVALUEASSIGN_H_

#include "SingleOperandQuadruple.h"

namespace codegen {

class LvalueAssign: public SingleOperandQuadruple {
public:
    LvalueAssign(std::string operand, std::string result, int accessSizeBytes = 8);
    virtual ~LvalueAssign() = default;

    void generateCode(AssemblyGenerator& generator) const override;

    int getAccessSizeBytes() const { return accessSizeBytes; }

    void collectSymbolRefs(SymbolRefs& refs) const override;

private:
    void print(std::ostream& stream) const override;

    int accessSizeBytes;
};

} // namespace codegen

#endif // LVALUEASSIGN_H_
