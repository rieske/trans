#include "CSNB_Internal.h"

#include "AbstractSyntaxTreeBuilderContext.h"
#include "ArrayDeclarator.h"
#include "Identifier.h"
#include "ParenthesizedDeclarator.h"

namespace ast {

void parenthesizedDeclarator(AbstractSyntaxTreeBuilderContext& context) {
    context.popTerminal();
    context.popTerminal();
    context.pushDirectDeclarator(std::make_unique<ParenthesizedDeclarator>(context.popDeclarator()));
}

void identifierDeclarator(AbstractSyntaxTreeBuilderContext& context) {
    context.pushDirectDeclarator(std::make_unique<Identifier>(context.popTerminal()));
}

void arrayDeclarator(AbstractSyntaxTreeBuilderContext& context) {
    context.popTerminal();
    context.popTerminal();
    context.pushDirectDeclarator(std::make_unique<ArrayDeclarator>(context.popDirectDeclarator(),
            context.popExpression(), &context.environment().vlaExpressions()));
}

void arrayDeclaratorQualified(AbstractSyntaxTreeBuilderContext& context) {
    context.popTypeQualifierList();
    arrayDeclarator(context);
}

namespace {

std::unique_ptr<Identifier> anonymousIdentifier() {
    return std::make_unique<Identifier>(TerminalSymbol { "", translation_unit::Context { "", 0 } });
}

void withAnonymousDirectDeclarator(AbstractSyntaxTreeBuilderContext& context,
        void (*creator)(AbstractSyntaxTreeBuilderContext&)) {
    context.pushDirectDeclarator(anonymousIdentifier());
    creator(context);
}

} // namespace

void abstractArrayDeclarator(AbstractSyntaxTreeBuilderContext& context) {
    context.popTerminal();
    context.popTerminal();
    context.pushDirectDeclarator(std::make_unique<ArrayDeclarator>(context.popDirectDeclarator(), nullptr));
}

void abstractArrayDeclaratorQualified(AbstractSyntaxTreeBuilderContext& context) {
    context.popTypeQualifierList();
    abstractArrayDeclarator(context);
}

void abstractArrayOnlySized(AbstractSyntaxTreeBuilderContext& context) {
    withAnonymousDirectDeclarator(context, arrayDeclarator);
}

void abstractArrayOnlyUnsized(AbstractSyntaxTreeBuilderContext& context) {
    withAnonymousDirectDeclarator(context, abstractArrayDeclarator);
}

void abstractArrayOnlyQualifiedSized(AbstractSyntaxTreeBuilderContext& context) {
    context.popTypeQualifierList();
    abstractArrayOnlySized(context);
}

void abstractArrayOnlyQualifiedUnsized(AbstractSyntaxTreeBuilderContext& context) {
    context.popTypeQualifierList();
    abstractArrayOnlyUnsized(context);
}

void abstractFuncOnly(AbstractSyntaxTreeBuilderContext& context) {
    withAnonymousDirectDeclarator(context, functionDeclarator);
}

void abstractNoargOnly(AbstractSyntaxTreeBuilderContext& context) {
    withAnonymousDirectDeclarator(context, noargFunctionDeclarator);
}

void functionDeclarator(AbstractSyntaxTreeBuilderContext& context) {
    context.popTerminal();
    context.popTerminal();
    auto argumentsDeclaration = context.popArgumentsDeclaration();
    auto arguments = std::move(argumentsDeclaration.first);
    const bool variadic = argumentsDeclaration.second;
    // `(void)` is an empty parameter list, not a single void parameter.
    if (arguments.size() == 1 && arguments.front().isVoid()) {
        arguments.clear();
    }
    context.pushDirectDeclarator(std::make_unique<FunctionDeclarator>(
            context.popDirectDeclarator(), std::move(arguments), variadic));
}

void noargFunctionDeclarator(AbstractSyntaxTreeBuilderContext& context) {
    context.popTerminal();
    context.popTerminal();
    context.pushDirectDeclarator(std::make_unique<FunctionDeclarator>(context.popDirectDeclarator()));
}

void pointerToDeclarator(AbstractSyntaxTreeBuilderContext& context) {
    context.pushDeclarator(std::make_unique<Declarator>(context.popDirectDeclarator(), context.popPointers()));
}

void declarator(AbstractSyntaxTreeBuilderContext& context) {
    context.pushDeclarator(std::make_unique<Declarator>(context.popDirectDeclarator()));
}

void parameterDeclaration(AbstractSyntaxTreeBuilderContext& context) {
    auto declarator = context.popDeclarator();
    auto specs = context.popDeclarationSpecifiers();
    specs.resolveTypeofAtParseTime(context.environment());
    FormalArgument argument { std::move(specs), std::move(declarator) };
    context.environment().maybeRegisterParameterShadow(argument.getName());
    context.environment().maybeDefineParameter(argument);
    context.pushFormalArgument(std::move(argument));
}

void abstractParameterDeclaration(AbstractSyntaxTreeBuilderContext& context) {
    // `<decl_specs> <abstract_declarator>` — e.g. `int *` as a parameter type.
    context.pushFormalArgument(FormalArgument { context.popDeclarationSpecifiers(), context.popDeclarator() });
}

// abstract_declarator ::= <pointer>  (unnamed pointer parameter / type name)
void abstractPointerDeclarator(AbstractSyntaxTreeBuilderContext& context) {
    context.pushDeclarator(std::make_unique<Declarator>(
            anonymousIdentifier(),
            context.popPointers()));
}

void parameterBaseTypeDeclaration(AbstractSyntaxTreeBuilderContext& context) {
    context.pushFormalArgument(FormalArgument { context.popDeclarationSpecifiers() });
}

void formalArguments(AbstractSyntaxTreeBuilderContext& context) {
    FormalArguments formalArguments;
    formalArguments.push_back(context.popFormalArgument());
    context.pushFormalArguments(std::move(formalArguments));
}

void addFormalArgument(AbstractSyntaxTreeBuilderContext& context) {
    context.popTerminal();
    auto formalArguments = context.popFormalArguments();
    formalArguments.push_back(context.popFormalArgument());
    context.pushFormalArguments(std::move(formalArguments));
}

void formalArgumentsDeclaration(AbstractSyntaxTreeBuilderContext& context) {
    context.pushArgumentsDeclaration(std::make_pair(context.popFormalArguments(), false));
}

void formalArgumentsWithVararg(AbstractSyntaxTreeBuilderContext& context) {
    context.popTerminal();
    context.popTerminal();
    context.pushArgumentsDeclaration(std::make_pair(context.popFormalArguments(), true));
}

void pointer(AbstractSyntaxTreeBuilderContext& context) {
    context.popTerminal();
    context.newPointer(Pointer { });
}

void pointerToPointer(AbstractSyntaxTreeBuilderContext& context) {
    context.popTerminal();
    context.pointerToPointer(Pointer { });
}

void qualifiedPointer(AbstractSyntaxTreeBuilderContext& context) {
    context.popTerminal();
    context.newPointer(context.popTypeQualifierList());
}

void qualifiedPointerToPointer(AbstractSyntaxTreeBuilderContext& context) {
    context.popTerminal();
    context.pointerToPointer( { context.popTypeQualifierList() });
}


} // namespace ast
