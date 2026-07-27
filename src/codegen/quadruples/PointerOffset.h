#ifndef POINTEROFFSET_QUADRUPLE_H_
#define POINTEROFFSET_QUADRUPLE_H_

#include <string>

#include "Quadruple.h"

namespace codegen {

// result = base +/- index * elementSizeBytes  (pointer +/- integer, scaled).
// Base holds a pointer value (not an array object home).
class PointerOffset: public Quadruple {
public:
    PointerOffset(std::string base, std::string index, int elementSizeBytes, std::string result,
            bool subtract);
    void generateCode(AssemblyGenerator& generator) const override;

    std::string getBase() const { return base; }
    std::string getIndex() const { return index; }
    int getElementSizeBytes() const { return elementSizeBytes; }
    std::string getResult() const { return result; }
    bool isSubtract() const { return subtract; }

private:
    void print(std::ostream& stream) const override;
    std::string base;
    std::string index;
    int elementSizeBytes;
    std::string result;
    bool subtract;
};

} // namespace codegen

#endif
