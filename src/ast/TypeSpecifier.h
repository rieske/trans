#ifndef TYPESPECIFIER_H_
#define TYPESPECIFIER_H_

#include <memory>
#include <string>

#include "types/Type.h"

namespace ast {

class TypeSpecifier {
public:
    TypeSpecifier(type::Type type, std::string name);
    virtual ~TypeSpecifier() = default;

    const std::string& getName() const;
    type::Type getType() const;

private:
    std::string name;
    type::Type type;
};

} // namespace ast

#endif // TYPESPECIFIER_H_
