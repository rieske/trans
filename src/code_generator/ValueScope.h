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

    int insert(std::string name, ast::Type type, unsigned line);
    void insertFunctionArgument(std::string name, ast::Type type, unsigned line);
    bool hasSymbol(std::string symbolName) const;
    ValueEntry lookup(std::string name) const;
    ValueEntry newTemp(ast::Type type);

    ValueScope* const getParentScope() const;
    unsigned getTableSize() const;

    void print() const;

private:
    std::string generateTempName();

    ValueScope* parentScope;

    std::map<std::string, ValueEntry> values;

    unsigned nextTemp { 0 };
    unsigned offset { 0 };
    unsigned paramOffset { 8 };
};

} /* namespace code_generator */

#endif /* VALUESCOPE_H_ */
