#ifndef CALL_H_
#define CALL_H_

#include <string>

#include "Quadruple.h"

namespace codegen {

class Call: public Quadruple {
public:
    // Direct call by procedure label name, or indirect when `indirect` is true
    // (procedureName is then the symbol holding the callee address).
    // memoryReturnDest: when non-empty, System V sret - pass &dest as first arg.
    Call(std::string procedureName, bool indirect = false, std::string memoryReturnDest = "");
    virtual ~Call() = default;

    void generateCode(AssemblyGenerator& generator) const override;

    std::string getProcedureName() const;
    bool isIndirect() const;
    std::string getMemoryReturnDest() const;

    void collectSymbolRefs(SymbolRefs& refs) const override;

private:
    void print(std::ostream& stream) const override;

    std::string procedureName;
    bool indirect { false };
    std::string memoryReturnDest;
};

} // namespace codegen

#endif // CALL_H_
