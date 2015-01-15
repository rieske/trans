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

    std::string getName() const;
    ast::Type getType() const;
    translation_unit::Context getContext() const;
    int getIndex() const;

    bool isFunctionArgument() const;

    void print() const;

    bool isStored() const;

    void setParam();

    void update(std::string reg);
    void removeReg(std::string reg);


    std::string getValue() const;

private:
    std::string name;
    ast::Type type;
    translation_unit::Context context;
    int index;

    bool temp;
    bool param;

    std::vector<std::string> value;
};

} /* namespace code_generator */

#endif /* VALUEENTRY_H_ */
