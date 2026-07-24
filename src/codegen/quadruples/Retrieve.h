#ifndef RETRIEVE_H_
#define RETRIEVE_H_

#include <string>

#include "Quadruple.h"

namespace codegen {

class Retrieve: public Quadruple {
public:
    // memoryReturn: true when the preceding Call used sret into resultName
    // (callee already wrote the object). StackMachine must not reload RAX/RDX.
    Retrieve(std::string resultName, bool memoryReturn = false);
    virtual ~Retrieve() = default;

    void generateCode(AssemblyGenerator& generator) const override;

    std::string getResultName() const;
    bool isMemoryReturn() const;

    void collectSymbolRefs(SymbolRefs& refs) const override;

private:
    void print(std::ostream& stream) const override;

    std::string resultName;
    bool memoryReturn { false };
};

} // namespace codegen

#endif // RETRIEVE_H_
