#ifndef PARSETYPETABLE_H_
#define PARSETYPETABLE_H_

#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "scanner/NameIntern.h"
#include "types/Type.h"

namespace scanner {

class ParseTypeTable {
public:
    explicit ParseTypeTable(NameIntern& intern);

    void add(const std::string& name, const type::Type& type);
    std::optional<type::Type> lookup(std::string_view name) const;
    std::optional<int> bindingDepth(std::string_view name) const;
    bool containsInCurrentScope(std::string_view name) const;
    void addPending(const std::string& name, const type::Type& type);
    void clearPending();

    void enterScope();
    void leaveScope();

private:
    void flushPending();

    NameIntern& intern_;
    std::vector<std::unordered_map<int, type::Type>> scopes_ { {} };
    std::unordered_map<int, type::Type> pending_;
};

} // namespace scanner

#endif
