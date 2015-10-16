#ifndef VALUEENTRY_H_
#define VALUEENTRY_H_

#include <memory>
#include <string>

#include "translation_unit/Context.h"
#include "types/FundamentalType.h"

namespace semantic_analyzer {

class ValueEntry {
public:
    ValueEntry(std::string name, const ast::FundamentalType& type, bool tmp, translation_unit::Context context, int index);
    ValueEntry(const ValueEntry& rhs);
    ValueEntry(ValueEntry&& rhs);
    ~ValueEntry() = default;

    ValueEntry& operator=(const ValueEntry& rhs);
    ValueEntry& operator=(ValueEntry && rhs);

    std::string getName() const;
    const ast::FundamentalType& getType() const;
    translation_unit::Context getContext() const;
    int getIndex() const;

    bool isFunctionArgument() const;

    void print() const;

    void setParam();

private:
    std::string name;
    std::unique_ptr<ast::FundamentalType> type;
    translation_unit::Context context;
    int index;

    bool temp;
    bool param;
};

} /* namespace semantic_analyzer */

#endif /* VALUEENTRY_H_ */
