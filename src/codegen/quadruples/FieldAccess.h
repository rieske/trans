#ifndef FIELDACCESS_QUADRUPLE_H_
#define FIELDACCESS_QUADRUPLE_H_

#include <string>
#include "Quadruple.h"

namespace codegen {

// result = address of field: (&base) + offset, or (*base) + offset when baseIsPointer (arrow).
class FieldAddress: public Quadruple {
public:
    FieldAddress(std::string base, int offsetBytes, std::string result, bool baseIsPointer = false);
    void generateCode(AssemblyGenerator& generator) const override;
    std::string getBase() const { return base; }
    int getOffsetBytes() const { return offsetBytes; }
    std::string getResult() const { return result; }
    bool baseIsPointer() const { return baseIsPointer_; }
    void collectSymbolRefs(SymbolRefs& refs) const override;

private:
    void print(std::ostream& stream) const override;
    std::string base;
    int offsetBytes;
    std::string result;
    bool baseIsPointer_;
};

} // namespace codegen
#endif
