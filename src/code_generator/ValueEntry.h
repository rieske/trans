#ifndef VALUEENTRY_H_
#define VALUEENTRY_H_

#include <string>
#include <vector>

#include "../ast/types/Type.h"
#include "../translation_unit/Context.h"

namespace code_generator {

class ValueEntry {
public:
    ValueEntry(std::string name, ast::Type type, bool tmp, translation_unit::Context context, int index);
    virtual ~ValueEntry();

    ast::Type getType() const;

    bool isFunctionArgument() const;

    void print() const;

    bool isTemp() const;
    bool isStored() const;
    unsigned getIndex() const;
    translation_unit::Context getContext() const;

    void setParam();

    void update(std::string reg);
    void removeReg(std::string reg);

    std::string getName() const;

    std::string getValue() const;

private:
    ast::Type type;

    translation_unit::Context context;

    bool temp;
    bool param;
    int index;

    std::string name;

    std::vector<std::string> value;
};

} /* namespace code_generator */

#endif /* VALUEENTRY_H_ */
