#include "StringLiteral.h"

namespace ast {

StringLiteral::StringLiteral(std::string value, translation_unit::Context context):
    value {value},
    context {context}
{}

StringLiteral::~StringLiteral() = default;

translation_unit::Context StringLiteral::getContext() const {
    return context;
}

std::string StringLiteral::getValue() const {
    return value;
}

} // namespace ast

