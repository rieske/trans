#ifndef VALUEENTRY_H_
#define VALUEENTRY_H_

#include <string>
#include <vector>

#include "../ast/types/Type.h"

namespace code_generator {

class ValueEntry {
public:
    ValueEntry(std::string name, ast::Type type, bool tmp, unsigned l);
    virtual ~ValueEntry();

    ast::Type getType() const;

    void addParam(ValueEntry*);
    std::vector<ValueEntry*> getParams() const;
    unsigned getParamCount() const;

    void print() const;

    bool isTemp() const;
    bool isStored() const;
    unsigned getSize() const;
    unsigned getOffset() const;
    std::string getOffsetReg() const;
    unsigned getLine() const;

    std::string store();

    void setParam();
    void setOffset(unsigned offset);

    bool isParam() const;

    void update(std::string reg);
    void removeReg(std::string reg);

    std::string getStorage() const;

    std::string getName() const;

    std::string getValue() const;

private:
    ast::Type type;

    unsigned size;

    bool temp;
    bool param;
    unsigned line;
    unsigned offset;

    bool stored;

    std::vector<ValueEntry*> params;

    std::string name;

    std::vector<std::string> value;
};

} /* namespace code_generator */

#endif /* VALUEENTRY_H_ */
