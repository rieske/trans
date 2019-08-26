#ifndef AST_CONSTANT_H_
#define AST_CONSTANT_H_

#include <string>

#include "../translation_unit/Context.h"
#include "../types/IntegralType.h"

namespace ast {

class Constant {
public:
    Constant(std::string value, IntegralType type, translation_unit::Context context);
    virtual ~Constant() = default;

    translation_unit::Context getContext() const;
    std::string getValue() const;
    std::unique_ptr<FundamentalType> getType() const;

private:
    std::string value;
    IntegralType type;
    translation_unit::Context context;
};

} /* namespace ast */

#endif /* AST_CONSTANT_H_ */
