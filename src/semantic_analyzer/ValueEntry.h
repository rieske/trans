#ifndef VALUEENTRY_H_
#define VALUEENTRY_H_

#include <memory>
#include <string>

#include "translation_unit/Context.h"
#include "types/Type.h"

namespace semantic_analyzer {

class ValueEntry {
public:
    ValueEntry(std::string name, const type::Type& type, bool tmp, translation_unit::Context context, int index);

    std::string getName() const;
    type::Type getType() const;
    translation_unit::Context getContext() const;
    int getIndex() const;

    std::string to_string() const;

private:
    std::string name;
    type::Type type;
    translation_unit::Context context;
    int index;

    bool temp;
};

} // namespace semantic_analyzer

#endif // VALUEENTRY_H_
