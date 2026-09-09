#include "CSNB_Internal.h"

#include "AbstractSyntaxTreeBuilderContext.h"
#include "FunctionDefinition.h"

namespace ast {

void braceInitializer(AbstractSyntaxTreeBuilderContext& context) {
    context.popTerminal(); // }
    auto elements = context.popInitializerList();
    context.popTerminal(); // {
    context.pushExpression(std::make_unique<InitializerListExpression>(std::move(elements)));
}

void braceInitializerTrailingComma(AbstractSyntaxTreeBuilderContext& context) {
    context.popTerminal(); // }
    context.popTerminal(); // ,
    auto elements = context.popInitializerList();
    context.popTerminal(); // {
    context.pushExpression(std::make_unique<InitializerListExpression>(std::move(elements)));
}

void initializerListFirst(AbstractSyntaxTreeBuilderContext& context) {
    InitializerElement element { context.popExpression() };
    context.newInitializerList();
    context.addInitializerElement(std::move(element));
}

void designatedInitializerListFirst(AbstractSyntaxTreeBuilderContext& context) {
    InitializerElement element { context.popExpression() };
    context.takePendingDesignator(element.designator);
    context.newInitializerList();
    context.addInitializerElement(std::move(element));
}

void initializerListAppend(AbstractSyntaxTreeBuilderContext& context) {
    context.popTerminal(); // ,
    InitializerElement element { context.popExpression() };
    context.addInitializerElement(std::move(element));
}

void designatedInitializerListAppend(AbstractSyntaxTreeBuilderContext& context) {
    context.popTerminal(); // ,
    InitializerElement element { context.popExpression() };
    context.takePendingDesignator(element.designator);
    context.addInitializerElement(std::move(element));
}

void memberDesignator(AbstractSyntaxTreeBuilderContext& context) {
    auto member = context.popTerminal(); // id
    context.popTerminal(); // .
    context.pushMemberDesignator(member.value);
}

void arrayDesignator(AbstractSyntaxTreeBuilderContext& context) {
    context.popTerminal(); // ]
    auto indexExpression = context.popExpression();
    context.popTerminal(); // [
    context.pushArrayIndexDesignator(std::move(indexExpression));
}

void designatorListSingle(AbstractSyntaxTreeBuilderContext& context) {
    (void)context;
}

void designatorListAppend(AbstractSyntaxTreeBuilderContext& context) {
    // Nested designators (.a.b or .a[0] or [0].x): stack top is the newest segment.
    std::vector<DesignatorStep> suffix;
    context.takePendingDesignator(suffix);
    std::vector<DesignatorStep> base;
    context.takePendingDesignator(base);
    for (auto& step : suffix) {
        base.push_back(std::move(step));
    }
    context.pushPendingDesignator(std::move(base));
}

void designation(AbstractSyntaxTreeBuilderContext& context) {
    context.popTerminal(); // =
}

void initializedDeclarator(AbstractSyntaxTreeBuilderContext& context) {
    auto declarator = context.popDeclarator();
    context.pushInitializedDeclarator(std::make_unique<InitializedDeclarator>(std::move(declarator)));
}

void initializedDeclaratorWithInitializer(AbstractSyntaxTreeBuilderContext& context) {
    // Production: <declarator> '=' <initializer> - consume '=' so it does not
    // poison later reductions when designators also use '='.
    context.popTerminal(); // =
    auto declarator = context.popDeclarator();
    auto initializerExpression = context.popExpression();
    context.pushInitializedDeclarator(std::make_unique<InitializedDeclarator>(std::move(declarator), std::move(initializerExpression)));
}

void initializedDeclaratorList(AbstractSyntaxTreeBuilderContext& context) {
    std::vector<std::unique_ptr<InitializedDeclarator>> declarators;
    declarators.push_back(context.popInitializedDeclarator());
    context.pushInitializedDeclarators(std::move(declarators));
}

void addToInitializedDeclaratorList(AbstractSyntaxTreeBuilderContext& context) {
    context.popTerminal();
    auto initializedDeclarators = context.popInitializedDeclarators();
    initializedDeclarators.push_back(context.popInitializedDeclarator());
    context.pushInitializedDeclarators(std::move(initializedDeclarators));
}

namespace {

// Ordinary identifiers and enumerators share a namespace, so a name declared beside an
// enumerator of the same scope collides with it (C).
void rejectEnumeratorRedefinition(AbstractSyntaxTreeBuilderContext& context,
        const std::string& name, translation_unit::Context where) {
    if (!name.empty() && context.environment().enumeratorInCurrentScope(name)) {
        context.error(where, "redefinition of enumerator `" + name + "`");
    }
}

} // namespace

void initializedDeclaration(AbstractSyntaxTreeBuilderContext& context) {
    context.popTerminal();
    auto declarationSpecifiers = context.popDeclarationSpecifiers();
    auto initializedDeclarators = context.popInitializedDeclarators();
    if (!declarationSpecifiers.resolveTypeofAtParseTime(context.environment())
            && declarationSpecifiers.isTypedef()) {
        context.error({ "", 0 }, "cannot determine type of typeof operand");
        return;
    }
    for (const auto& declarator : initializedDeclarators) {
        rejectEnumeratorRedefinition(context, declarator->getName(), declarator->getContext());
    }
    context.environment().registerInitializedDeclaration(declarationSpecifiers, initializedDeclarators);
    context.pushDeclaration(std::make_unique<Declaration>(std::move(declarationSpecifiers),
            std::move(initializedDeclarators)));
}

void declaration(AbstractSyntaxTreeBuilderContext& context) {
    context.popTerminal();
    auto declarationSpecifiers = context.popDeclarationSpecifiers();
    context.pushDeclaration(std::make_unique<Declaration>(std::move(declarationSpecifiers)));
}

void declarationList(AbstractSyntaxTreeBuilderContext& context) {
    std::vector<std::unique_ptr<Declaration>> declarations;
    declarations.push_back(context.popDeclaration());
    context.pushDeclarationList(std::move(declarations));
}

void addDeclarationToList(AbstractSyntaxTreeBuilderContext& context) {
    auto declarations = context.popDeclarationList();
    declarations.push_back(context.popDeclaration());
    context.pushDeclarationList(std::move(declarations));
}


void functionDefinition(AbstractSyntaxTreeBuilderContext& context) {
    auto declarationSpecifiers = context.popDeclarationSpecifiers();
    auto declarator = context.popDeclarator();
    auto body = context.popBlock();
    if (!body) {
        context.error(declarator->getContext(), "function definition body is not a compound statement");
        return;
    }
    declarationSpecifiers.resolveTypeofAtParseTime(context.environment());
    rejectEnumeratorRedefinition(context, declarator->getName(), declarator->getContext());
    context.environment().tryDefineObject(declarationSpecifiers, *declarator);
    context.pushExternalDeclaration(ExternalDeclaration { std::make_unique<FunctionDefinition>(
            std::move(declarationSpecifiers), std::move(declarator), std::move(body)) });
}

void defaultReturnTypeFunctionDefinition(AbstractSyntaxTreeBuilderContext& context) {
    DeclarationSpecifiers defaultReturnTypeSpecifiers { TypeSpecifier { type::signedInteger(), "int" } };
    auto declarator = context.popDeclarator();
    auto body = context.popBlock();
    if (!body) {
        context.error(declarator->getContext(), "function definition body is not a compound statement");
        return;
    }
    rejectEnumeratorRedefinition(context, declarator->getName(), declarator->getContext());
    context.environment().tryDefineObject(defaultReturnTypeSpecifiers, *declarator);
    context.pushExternalDeclaration(ExternalDeclaration { std::make_unique<FunctionDefinition>(
            std::move(defaultReturnTypeSpecifiers), std::move(declarator), std::move(body)) });
}

void externalDeclaration(AbstractSyntaxTreeBuilderContext& context) {
    context.pushExternalDeclaration(ExternalDeclaration { context.popDeclaration() });
}

void translationUnit(AbstractSyntaxTreeBuilderContext& context) {
    context.addToTranslationUnit(context.popExternalDeclaration());
}

void addToTranslationUnit(AbstractSyntaxTreeBuilderContext& context) {
    context.addToTranslationUnit(context.popExternalDeclaration());
}


} // namespace ast
