#ifndef VALUESCOPE_H_
#define VALUESCOPE_H_

#include <map>
#include <string>

#include "ValueEntry.h"

namespace code_generator {

class ValueScope {
public:
    ValueScope(ValueScope* parentScope);
    virtual ~ValueScope();

    bool insertSymbol(std::string name, ast::Type type, translation_unit::Context context);
    void insertFunctionArgument(std::string name, ast::Type type, translation_unit::Context context);
    bool isSymbolDefined(std::string symbolName) const;
    ValueEntry lookup(std::string name) const;
    ValueEntry newTemp(ast::Type type);

    ValueScope* const getParentScope() const;
    unsigned getTableSize() const;

    void print() const;

private:
    std::string generateTempName();

    ValueScope* const parentScope;

    std::map<std::string, ValueEntry> values;

    unsigned nextTemp { 0 };
    unsigned valueIndex { 0 };
    unsigned argumentIndex { 2 };
};

} /* namespace code_generator */

#endif /* VALUESCOPE_H_ */
