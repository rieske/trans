#ifndef VALUESCOPE_H_
#define VALUESCOPE_H_

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "ValueEntry.h"

namespace ast {
class StoredType;
} /* namespace ast */

namespace semantic_analyzer {

class ValueScope {
public:
    bool insertSymbol(std::string name, const ast::FundamentalType& type, translation_unit::Context context);
    void insertFunctionArgument(std::string name, const ast::FundamentalType& type, translation_unit::Context context);
    ValueEntry createTemporarySymbol(std::unique_ptr<ast::FundamentalType> type);
    bool isSymbolDefined(std::string symbolName) const;
    ValueEntry lookup(std::string name) const;

    std::map<std::string, ValueEntry> getSymbols() const;
    std::vector<ValueEntry> getArguments() const;

    void print() const;

private:
    std::vector<ValueEntry> arguments;
    std::map<std::string, ValueEntry> localSymbols;
};

} /* namespace semantic_analyzer */

#endif /* VALUESCOPE_H_ */
