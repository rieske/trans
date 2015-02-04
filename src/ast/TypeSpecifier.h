#ifndef TYPESPECIFIER_H_
#define TYPESPECIFIER_H_

#include <memory>
#include <string>

#include "ast/types/BaseType.h"

namespace ast {

class TypeSpecifier {
public:
    TypeSpecifier(std::unique_ptr<BaseType> type, std::string name);
    TypeSpecifier(const TypeSpecifier& copyFrom);
    virtual ~TypeSpecifier() = default;

    const std::string& getName() const;
    std::unique_ptr<BaseType> getType() const;

private:
    const std::string name;
    const std::unique_ptr<const BaseType> type;
};

} /* namespace ast */

#endif /* TYPESPECIFIER_H_ */
