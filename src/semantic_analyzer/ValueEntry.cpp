#include "ValueEntry.h"

#include <iostream>
#include <sstream>

#include "types/FundamentalType.h"

namespace semantic_analyzer {

ValueEntry::ValueEntry(std::string name, const ast::FundamentalType& type, bool tmp, translation_unit::Context context, int index) :
        name { name },
        type { type.clone() },
        context { context },
        index { index },
        temp { tmp }
{
}

ValueEntry::ValueEntry(const ValueEntry& rhs) :
        name { rhs.name },
        type { rhs.type->clone() },
        context { rhs.context },
        index { rhs.index },
        temp { rhs.temp }
{
}

ValueEntry::ValueEntry(ValueEntry&& rhs) :
        name { std::move(rhs.name) },
        type { std::move(rhs.type) },
        context { std::move(rhs.context) },
        index { std::move(rhs.index) },
        temp { std::move(rhs.temp) }
{
}

ValueEntry& ValueEntry::operator =(const ValueEntry& rhs) {
    this->name = rhs.name;
    this->type = std::unique_ptr<ast::FundamentalType> { rhs.type->clone() };
    this->context = rhs.context;
    this->index = rhs.index;
    this->temp = rhs.temp;
    return *this;
}

ValueEntry& ValueEntry::operator =(ValueEntry&& rhs) {
    this->name = std::move(rhs.name);
    this->type = std::move(rhs.type);
    this->context = std::move(rhs.context);
    this->index = std::move(rhs.index);
    this->temp = std::move(rhs.temp);
    return *this;
}

const ast::FundamentalType& ValueEntry::getType() const {
    return *type;
}

std::string ValueEntry::to_string() const {
    std::stringstream str;
    str << "\t" << name << "\t" << (temp ? "temp" : "") << "\t" << index << "\t" << type->toString() << std::endl;
    return str.str();
}

translation_unit::Context ValueEntry::getContext() const {
    return context;
}

int ValueEntry::getIndex() const {
    return index;
}

std::string ValueEntry::getName() const {
    return name;
}

} // namespace semantic_analyzer

