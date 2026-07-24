#ifndef TYPESPECIFIER_H_
#define TYPESPECIFIER_H_

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "types/Type.h"

namespace ast {

class TypeSpecifier {
public:
    TypeSpecifier(type::Type type, std::string name);
    TypeSpecifier(type::Type type, std::string name, std::vector<std::pair<std::string, long>> enumerators);
    virtual ~TypeSpecifier() = default;

    const std::string& getName() const;
    type::Type getType() const;
    const std::vector<std::pair<std::string, long>>& getEnumerators() const;
    bool hasEnumerators() const;

private:
    std::string name;
    type::Type type;
    std::vector<std::pair<std::string, long>> enumerators;
};

} // namespace ast

#endif // TYPESPECIFIER_H_
