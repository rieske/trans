#ifndef CONSTANT_H_
#define CONSTANT_H_

#include <string>

#include "../translation_unit/Context.h"
#include "types/NumericType.h"

namespace ast {
class Type;
} /* namespace ast */

namespace ast {

class Constant {
public:
    Constant(std::string value, NumericType type, translation_unit::Context context);
    virtual ~Constant();

    translation_unit::Context getContext() const;
    std::string getValue() const;
    Type getType() const;

private:
    std::string value;
    NumericType type;
    translation_unit::Context context;
};

} /* namespace ast */

#endif /* CONSTANT_H_ */
