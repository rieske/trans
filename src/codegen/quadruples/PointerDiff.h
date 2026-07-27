#ifndef POINTERDIFF_QUADRUPLE_H_
#define POINTERDIFF_QUADRUPLE_H_

#include <string>

#include "Quadruple.h"

namespace codegen {

// result = (left - right) / elementSizeBytes  (pointer difference, element count).
class PointerDiff: public Quadruple {
public:
    PointerDiff(std::string left, std::string right, int elementSizeBytes, std::string result);
    void generateCode(AssemblyGenerator& generator) const override;

    std::string getLeft() const { return left; }
    std::string getRight() const { return right; }
    int getElementSizeBytes() const { return elementSizeBytes; }
    std::string getResult() const { return result; }

private:
    void print(std::ostream& stream) const override;
    std::string left;
    std::string right;
    int elementSizeBytes;
    std::string result;
};

} // namespace codegen

#endif
