#include "TranslationUnitContext.h"

#include <iostream>

TranslationUnitContext::TranslationUnitContext(const std::string& sourceName, std::size_t offset) :
        sourceName { sourceName }, offset { offset } {
}

TranslationUnitContext::~TranslationUnitContext() {
}

size_t TranslationUnitContext::getOffset() const {
    return offset;
}

const std::string& TranslationUnitContext::getSourceName() const {
    return sourceName;
}

std::ostream& operator<<(std::ostream& ostream, const TranslationUnitContext& context) {
    ostream << context.getSourceName() << ":" << context.getOffset();
    return ostream;
}
