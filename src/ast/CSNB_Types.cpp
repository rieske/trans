#include "CSNB_Internal.h"

#include "AbstractSyntaxTreeBuilderContext.h"

#include <string>

namespace ast {

void doNothing(AbstractSyntaxTreeBuilderContext&) {
}

// Grammar covers more of C than the AST builder implements. Register explicit stubs so
// unsupported constructs fail with a clear message instead of "no AST creator defined".
std::function<void(AbstractSyntaxTreeBuilderContext&)> notImplementedYet(const char* feature) {
    return [feature](AbstractSyntaxTreeBuilderContext& context) {
        context.error({ "", 0 }, std::string(feature) + " is not implemented yet");
    };
}

void shortType(AbstractSyntaxTreeBuilderContext& context) {
    context.pushTypeSpecifier( { type::signedShort(), context.popTerminal().value });
}

void integerType(AbstractSyntaxTreeBuilderContext& context) {
    context.pushTypeSpecifier( { type::signedInteger(), context.popTerminal().value });
}

void longType(AbstractSyntaxTreeBuilderContext& context) {
    context.pushTypeSpecifier( { type::signedLong(), context.popTerminal().value });
}

void characterType(AbstractSyntaxTreeBuilderContext& context) {
    context.pushTypeSpecifier( { type::signedCharacter(), context.popTerminal().value });
}

void boolType(AbstractSyntaxTreeBuilderContext& context) {
    context.pushTypeSpecifier( { type::boolean(), context.popTerminal().value });
}

void complexType(AbstractSyntaxTreeBuilderContext& context) {
    context.pushTypeSpecifier( { type::complexDouble(), context.popTerminal().value });
}

void voidType(AbstractSyntaxTreeBuilderContext& context) {
    context.pushTypeSpecifier( { type::voidType(), context.popTerminal().value });
}

void floatType(AbstractSyntaxTreeBuilderContext& context) {
    context.pushTypeSpecifier( { type::floating(), context.popTerminal().value });
}

void doubleType(AbstractSyntaxTreeBuilderContext& context) {
    context.pushTypeSpecifier( { type::doubleFloating(), context.popTerminal().value });
}

void signedType(AbstractSyntaxTreeBuilderContext& context) {
    // bare `signed` means signed int
    context.pushTypeSpecifier( { type::signedInteger(), context.popTerminal().value });
}

void unsignedType(AbstractSyntaxTreeBuilderContext& context) {
    context.pushTypeSpecifier( { type::unsignedInteger(), context.popTerminal().value });
}

void typedefName(AbstractSyntaxTreeBuilderContext& context) {
    auto name = context.popTerminal();
    auto type = context.environment().lookupTypedef(name.value);
    if (!type) {
        context.error(name.context, "unknown typedef name: " + name.value);
        return;
    }
    context.pushTypeSpecifier(TypeSpecifier { *type, name.value });
}

void constQualifier(AbstractSyntaxTreeBuilderContext& context) {
    context.popTerminal();
    context.pushTypeQualifier(type::Qualifier::CONST);
}

void volatileQualifier(AbstractSyntaxTreeBuilderContext& context) {
    context.popTerminal();
    context.pushTypeQualifier(type::Qualifier::VOLATILE);
}

void restrictQualifier(AbstractSyntaxTreeBuilderContext& context) {
    context.popTerminal();
    context.pushTypeQualifier(type::Qualifier::RESTRICT);
}

void inlineFunctionSpecifier(AbstractSyntaxTreeBuilderContext& context) {
    context.pushFunctionSpecifier(FunctionSpecifier::INLINE(context.popTerminal().context));
}

void noreturnFunctionSpecifier(AbstractSyntaxTreeBuilderContext& context) {
    context.pushFunctionSpecifier(FunctionSpecifier::NORETURN(context.popTerminal().context));
}

void functionSpecifierOnly(AbstractSyntaxTreeBuilderContext& context) {
    context.pushDeclarationSpecifiers({ context.popFunctionSpecifier() });
}

void addDeclarationFunctionSpecifier(AbstractSyntaxTreeBuilderContext& context) {
    auto specs = context.popDeclarationSpecifiers();
    specs.add(context.popFunctionSpecifier());
    context.pushDeclarationSpecifiers(std::move(specs));
}

void autoStorageClass(AbstractSyntaxTreeBuilderContext& context) {
    context.pushStorageSpecifier(StorageSpecifier::AUTO(context.popTerminal().context));
}

void registerStorageClass(AbstractSyntaxTreeBuilderContext& context) {
    context.pushStorageSpecifier(StorageSpecifier::REGISTER(context.popTerminal().context));
}

void staticStorageClass(AbstractSyntaxTreeBuilderContext& context) {
    context.pushStorageSpecifier(StorageSpecifier::STATIC(context.popTerminal().context));
}

void externStorageClass(AbstractSyntaxTreeBuilderContext& context) {
    context.pushStorageSpecifier(StorageSpecifier::EXTERN(context.popTerminal().context));
}

void typedefStorageClass(AbstractSyntaxTreeBuilderContext& context) {
    context.pushStorageSpecifier(StorageSpecifier::TYPEDEF(context.popTerminal().context));
}

void typeQualifierList(AbstractSyntaxTreeBuilderContext& context) {
    context.newTypeQualifierList(context.popTypeQualifier());
}

void addTypeQualifierToList(AbstractSyntaxTreeBuilderContext& context) {
    context.addToTypeQualifierList(context.popTypeQualifier());
}

void declarationTypeSpecifier(AbstractSyntaxTreeBuilderContext& context) {
    context.pushDeclarationSpecifiers( { context.popTypeSpecifier() });
}

void addDeclarationTypeSpecifier(AbstractSyntaxTreeBuilderContext& context) {
    auto specs = context.popDeclarationSpecifiers();
    specs.add(context.popTypeSpecifier());
    context.pushDeclarationSpecifiers(std::move(specs));
}

DeclarationSpecifiers popResolvedSpecQualifiers(AbstractSyntaxTreeBuilderContext& context) {
    auto specs = context.popDeclarationSpecifiers();
    if (!specs.resolveTypeofAtParseTime(context.environment())) {
        context.error({ "", 0 }, "cannot determine type of typeof operand");
        return specs;
    }
    if (specs.getTypeSpecifiers().empty()) {
        context.error({ "", 0 }, "cannot determine type of spec-qualifier-list");
        return specs;
    }
    return specs;
}

void specQualifierListTypeName(AbstractSyntaxTreeBuilderContext& context) {
    auto specs = popResolvedSpecQualifiers(context);
    if (context.failed()) {
        return;
    }
    context.pushTypeSpecifier(specs.toTypeSpecifier());
}

void declarationStorageClassSpecifier(AbstractSyntaxTreeBuilderContext& context) {
    context.pushDeclarationSpecifiers( { context.popStorageSpecifier() });
}

void addDeclarationStorageClassSpecifier(AbstractSyntaxTreeBuilderContext& context) {
    auto specs = context.popDeclarationSpecifiers();
    specs.add(context.popStorageSpecifier());
    context.pushDeclarationSpecifiers(std::move(specs));
}

void declarationTypeQualifier(AbstractSyntaxTreeBuilderContext& context) {
    context.pushDeclarationSpecifiers( { context.popTypeQualifier() });
}

void addDeclarationTypeQualifier(AbstractSyntaxTreeBuilderContext& context) {
    auto specs = context.popDeclarationSpecifiers();
    specs.add(context.popTypeQualifier());
    context.pushDeclarationSpecifiers(std::move(specs));
}


} // namespace ast
