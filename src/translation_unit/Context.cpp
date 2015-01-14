#include "Context.h"

#include <iostream>

namespace translation_unit {

Context::Context(const std::string& sourceName, std::size_t offset) :
        sourceName { sourceName },
        offset { offset }
{
}

Context::~Context() {
}

size_t Context::getOffset() const {
    return offset;
}

const std::string& Context::getSourceName() const {
    return sourceName;
}

}

bool operator ==(const translation_unit::Context& lhs, const translation_unit::Context& rhs) {
    return lhs.getOffset() == rhs.getOffset() && lhs.getSourceName() == rhs.getSourceName();
}

bool operator !=(const translation_unit::Context& lhs, const translation_unit::Context& rhs) {
    return !(lhs == rhs);
}

std::ostream& operator<<(std::ostream& ostream, const translation_unit::Context& context) {
    ostream << to_string(context);
    return ostream;
}

std::string to_string(const translation_unit::Context& context) {
    return context.getSourceName() + ":" + std::to_string(context.getOffset());
}
