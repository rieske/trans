#ifndef TYPESPECIFIER_H_
#define TYPESPECIFIER_H_

#include <memory>
#include <string>

#include "types/FundamentalType.h"

namespace ast {

class TypeSpecifier {
public:
    TypeSpecifier(FundamentalType& type, std::string name);
    TypeSpecifier(const TypeSpecifier& copyFrom);
    virtual ~TypeSpecifier() = default;

    const std::string& getName() const;
    std::unique_ptr<FundamentalType> getType() const;

private:
    std::string name;
    std::unique_ptr<FundamentalType> type;
};

} // namespace ast

#endif // TYPESPECIFIER_H_
