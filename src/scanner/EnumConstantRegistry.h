#ifndef ENUMCONSTANTREGISTRY_H_
#define ENUMCONSTANTREGISTRY_H_

#include <map>
#include <string>

namespace scanner {

// Enumerators visible while building the current translation unit, so that
// later enumerators (and const expressions) can fold references to earlier ones.
class EnumConstantRegistry {
public:
    static void clear();
    static void add(const std::string& name, long value);
    static bool lookup(const std::string& name, long& value);

private:
    static std::map<std::string, long>& table();
};

} // namespace scanner

#endif // ENUMCONSTANTREGISTRY_H_
