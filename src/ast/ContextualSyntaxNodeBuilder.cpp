#include "ContextualSyntaxNodeBuilder.h"
#include "CSNB_Internal.h"

namespace ast {

ContextualSyntaxNodeBuilder::ContextualSyntaxNodeBuilder(const parser::Grammar& grammar) {
    this->grammar = &grammar;

    csnb::registerTypeProductions(grammar, nodeCreatorRegistry);
    csnb::registerDeclaratorProductions(grammar, nodeCreatorRegistry);
    csnb::registerExpressionProductions(grammar, nodeCreatorRegistry);
    csnb::registerBuiltinProductions(grammar, nodeCreatorRegistry);
    csnb::registerInitializerProductions(grammar, nodeCreatorRegistry);
    csnb::registerStatementProductions(grammar, nodeCreatorRegistry);
}

ContextualSyntaxNodeBuilder::~ContextualSyntaxNodeBuilder() = default;

void ContextualSyntaxNodeBuilder::updateContext(const parser::Production& production, AbstractSyntaxTreeBuilderContext& context) const {
    try {
        nodeCreatorRegistry.at(production.getDefiningSymbol()).at(production.producedSequence())(context);
    } catch (std::out_of_range& exception) {
        noCreatorDefined(production);
    }
}

void ContextualSyntaxNodeBuilder::noCreatorDefined(const parser::Production& production) const {
    throw std::runtime_error {
            "language construct not implemented yet (production `" + grammar->str(production) + "`)" };
}

} /* namespace ast */
