#ifndef RETRIEVE_H_
#define RETRIEVE_H_

#include <string>

#include "Quadruple.h"

namespace codegen {

class Retrieve: public Quadruple {
public:
    Retrieve(std::string resultName);
    virtual ~Retrieve() = default;

    void generateCode(AssemblyGenerator& generator) const override;

    std::string getResultName() const;

private:
    void print(std::ostream& stream) const override;

    std::string resultName;
};

} // namespace codegen

#endif // RETRIEVE_H_
