#ifndef VALUESCOPE_H_
#define VALUESCOPE_H_

#include <map>
#include <string>

#include "ValueEntry.h"

namespace code_generator {

class ValueScope {
public:
    ValueScope();
    virtual ~ValueScope();

    bool insertSymbol(std::string name, ast::Type type, translation_unit::Context context);
    void insertFunctionArgument(std::string name, ast::Type type, translation_unit::Context context);
    ValueEntry createTemporarySymbol(ast::Type type);
    bool isSymbolDefined(std::string symbolName) const;
    ValueEntry lookup(std::string name) const;

    std::map<std::string, ValueEntry> getSymbols() const;
    std::map<std::string, ValueEntry> getArguments() const;

    void print() const;

private:
    std::map<std::string, ValueEntry> arguments;
    std::map<std::string, ValueEntry> localSymbols;
};

} /* namespace code_generator */

#endif /* VALUESCOPE_H_ */
