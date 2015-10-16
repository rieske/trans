#include "translation_unit/Context.h"

namespace translation_unit {

Context::Context(std::string sourceName, std::size_t offset) :
        sourceName { sourceName },
        offset { offset }
{
}

std::size_t Context::getOffset() const {
    return offset;
}

std::string Context::getSourceName() const {
    return sourceName;
}

std::ostream& operator<<(std::ostream& ostream, const Context& context) {
    ostream << to_string(context);
    return ostream;
}

bool operator ==(const translation_unit::Context& lhs, const Context& rhs) {
    return lhs.getOffset() == rhs.getOffset() && lhs.getSourceName() == rhs.getSourceName();
}

bool operator !=(const Context& lhs, const Context& rhs) {
    return !(lhs == rhs);
}

std::string to_string(const Context& context) {
    return context.getSourceName() + ":" + std::to_string(context.getOffset());
}

}

