#ifndef ENUMCONSTANTREGISTRY_H_
#define ENUMCONSTANTREGISTRY_H_

#include <map>
#include <string>

namespace scanner {

// Enumerators visible while building one translation unit, so that later
// enumerators (and const expressions) can fold references to earlier ones.
// Instance-owned (LexicalSession); never process-static.
// Product limit: TU-flat (not C block scope). SA import keeps the same map.
// add() is last-wins; redefinition diagnostics belong at the PE/SA layer.
// Sessions are single-shot per compile; no reset API (fresh LexicalSession each TU).
class EnumConstantRegistry {
public:
    void add(const std::string& name, long value);
    bool lookup(const std::string& name, long& value) const;
    const std::map<std::string, long>& entries() const { return table_; }

private:
    std::map<std::string, long> table_;
};

} // namespace scanner

#endif // ENUMCONSTANTREGISTRY_H_
