#ifndef _SYMBOL_ENTRY_H_
#define _SYMBOL_ENTRY_H_

#include <string>
#include <vector>

#include "../ast/BasicType.h"
#include "../ast/TypeInfo.h"

using std::string;
using std::vector;

class SymbolEntry {
public:
    SymbolEntry(string name, ast::TypeInfo typeInfo, bool tmp, unsigned l);
    ~SymbolEntry();

    string getName() const;
    ast::TypeInfo getTypeInfo() const;
    bool isTemp() const;
    bool isStored() const;
    unsigned getSize() const;
    unsigned getOffset() const;
    string getOffsetReg() const;
    unsigned getLine() const;
    unsigned getParamCount() const;
    vector<SymbolEntry *> getParams() const;

    string getStorage() const;
    string getValue() const;

    string store();

    void setParam(SymbolEntry *);
    void setParam();
    void setOffset(unsigned offset);

    bool isParam() const;

    void update(string reg);
    void removeReg(string reg);

    void print() const;

private:
    string name;
    ast::TypeInfo typeInfo;
    unsigned size;

    bool temp;
    bool param;
    unsigned line;
    vector<SymbolEntry *> params;
    unsigned offset;

    bool stored;
    vector<string> value;
};

#endif // _SYMBOL_ENTRY_H_
