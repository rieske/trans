#include "GenericSelection.h"

#include "AbstractSyntaxTreeVisitor.h"
#include "ParseEnvironment.h"
#include "types/TypeQuery.h"

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

std::optional<type::Type> GenericSelection::typeAtParseTime(const ParseEnvironment& environment) const {
    auto controlling = controlling_->typeAtParseTime(environment);
    if (!controlling) {
        return std::nullopt;
    }
    const type::Type converted = type::afterLvalueConversion(*controlling);
    std::vector<type::Type> resolved(associations_.size(), type::voidType());
    std::vector<type::GenericArmView> arms(associations_.size());
    for (std::size_t i = 0; i < associations_.size(); ++i) {
        const auto& association = associations_[i];
        if (association.isDefault()) {
            arms[i] = { true, nullptr };
            continue;
        }
        auto resolvedType = association.typeName->tryResolve(environment);
        if (!resolvedType) {
            return std::nullopt;
        }
        resolved[i] = *resolvedType;
        arms[i] = { false, &resolved[i] };
    }
    const type::GenericSelectionChoice choice = type::selectGenericAssociation(converted, arms);
    if (choice.status != type::GenericSelectionStatus::Ok || !choice.index) {
        return std::nullopt;
    }
    return associations_[*choice.index].expression->typeAtParseTime(environment);
}

translation_unit::Context GenericSelection::getContext() const {
    return context_;
}

bool GenericSelection::evaluateConstant(type::IntegerConstant& value) const {
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
