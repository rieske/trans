#ifndef _SYMBOL_ENTRY_H_
#define _SYMBOL_ENTRY_H_

#include <string>
#include <vector>

#include "../semantic_analyzer/BasicType.h"

using std::string;
using std::vector;

/**
 * @author Vaidotas Valuckas
 * Simbolių lentelės įrašas
 **/

class SymbolEntry
{
    public:
        SymbolEntry(string name, semantic_analyzer::BasicType basicType, string et, bool tmp, unsigned l);
        ~SymbolEntry();

        string getName() const;
        semantic_analyzer::BasicType getBasicType() const;
        string getExtendedType() const;
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

        void setBasicType(semantic_analyzer::BasicType bt);
        void setExtendedType(string et);

        void setParam(SymbolEntry *);
        void setParam();
        void setOffset(unsigned offset);

        bool isParam() const;

        void update(string reg);
        void removeReg(string reg);
        
        void print() const;

    private:
        string name;
        semantic_analyzer::BasicType basicType;
        string extended_type;
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
