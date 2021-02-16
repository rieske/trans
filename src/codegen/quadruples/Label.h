#ifndef LABEL_H_
#define LABEL_H_

#include <string>

#include "Quadruple.h"

namespace codegen {

class Label: public Quadruple {
public:
    Label(std::string name);
    virtual ~Label() = default;

    void generateCode(AssemblyGenerator& generator) const override;

    bool isLabel() const override;

    std::string getName() const;

private:
    void print(std::ostream& stream) const override;

    std::string name;
};

} // namespace codegen

#endif // LABEL_H_
