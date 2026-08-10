#include "GenericSelection.h"

#include "AbstractSyntaxTreeVisitor.h"

#include <stdexcept>

namespace ast {

GenericSelection::GenericSelection(translation_unit::Context context,
        std::unique_ptr<Expression> controlling,
        std::vector<GenericAssociation> associations) :
        context_ { std::move(context) },
        controlling_ { std::move(controlling) },
        associations_ { std::move(associations) } {
}

void GenericSelection::accept(AbstractSyntaxTreeVisitor& visitor) {
    visitor.visit(*this);
}

translation_unit::Context GenericSelection::getContext() const {
    return context_;
}

bool GenericSelection::evaluateConstant(long& value) const {
    if (!selectedIndex_ || *selectedIndex_ >= associations_.size()) {
        return false;
    }
    return associations_[*selectedIndex_].expression->evaluateConstant(value);
}

void GenericSelection::select(std::size_t index, symbols::AnnotationStore& store) {
    selectedIndex_ = index;
    takeValueFrom(selectedExpression(), store);
}

Expression& GenericSelection::selectedExpression() {
    if (!selectedIndex_ || *selectedIndex_ >= associations_.size()) {
        throw std::logic_error { "generic selection has no selected association" };
    }
    return *associations_[*selectedIndex_].expression;
}

const Expression& GenericSelection::selectedExpression() const {
    if (!selectedIndex_ || *selectedIndex_ >= associations_.size()) {
        throw std::logic_error { "generic selection has no selected association" };
    }
    return *associations_[*selectedIndex_].expression;
}

} // namespace ast
