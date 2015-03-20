#ifndef LABEL_H_
#define LABEL_H_

#include <string>

#include "Quadruple.h"

namespace code_generator {

class Label: public Quadruple {
public:
    Label(std::string name);
    virtual ~Label() = default;

    void generateCode(AssemblyGenerator& generator) const override;

    std::string getName() const;

private:
    std::string name;
};

} /* namespace code_generator */

#endif /* LABEL_H_ */
