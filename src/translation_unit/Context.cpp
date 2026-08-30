#include "Context.h"

#include <set>

namespace translation_unit {

namespace {

const std::string& intern(std::string name) {
    static std::set<std::string> names;
    return *names.insert(std::move(name)).first;
}

} // namespace

Context::Context(std::string sourceName, std::size_t offset) :
        sourceName { &intern(std::move(sourceName)) },
        offset { offset }
{
}

std::size_t Context::getOffset() const {
    return offset;
}

const std::string& Context::getSourceName() const {
    return *sourceName;
}

std::ostream& operator<<(std::ostream& ostream, const Context& context) {
    ostream << to_string(context);
    return ostream;
}

std::string to_string(const Context& context) {
    return context.getSourceName() + ":" + std::to_string(context.getOffset());
}

} // namespace translation_unit

