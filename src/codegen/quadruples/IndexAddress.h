#ifndef INDEXADDRESS_QUADRUPLE_H_
#define INDEXADDRESS_QUADRUPLE_H_

#include <string>
#include "Quadruple.h"

namespace codegen {

// result = &base[index] with element stride elementSizeBytes.
// If baseIsArray, base is the array object (use its address); otherwise base holds a pointer.
class IndexAddress: public Quadruple {
public:
    IndexAddress(std::string base, std::string index, int elementSizeBytes, std::string result, bool baseIsArray = false);
    void generateCode(AssemblyGenerator& generator) const override;

    std::string getBase() const { return base; }
    std::string getIndex() const { return index; }
    int getElementSizeBytes() const { return elementSizeBytes; }
    std::string getResult() const { return result; }
    bool baseIsArray() const { return baseIsArray_; }

    void collectSymbolRefs(SymbolRefs& refs) const override;

private:
    void print(std::ostream& stream) const override;
    std::string base;
    std::string index;
    int elementSizeBytes;
    std::string result;
    bool baseIsArray_;
};

} // namespace codegen

#endif
