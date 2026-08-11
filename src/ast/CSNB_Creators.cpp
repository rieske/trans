#include "CSNB_Internal.h"

#include "util/FloatingLiteral.h"
#include "util/IntegerLiteral.h"

namespace ast {


void doNothing(AbstractSyntaxTreeBuilderContext&) {
}

// Grammar covers more of C than the AST builder implements. Register explicit stubs so
// unsupported constructs fail with a clear message instead of "no AST creator defined".
std::function<void(AbstractSyntaxTreeBuilderContext&)> notImplementedYet(const char* feature) {
    return [feature](AbstractSyntaxTreeBuilderContext&) {
        throw std::runtime_error { std::string(feature) + " is not implemented yet" };
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

void voidType(AbstractSyntaxTreeBuilderContext& context) {
    context.pushTypeSpecifier( { type::voidType(), context.popTerminal().value });
}

void floatType(AbstractSyntaxTreeBuilderContext& context) {
    context.pushTypeSpecifier( { type::floating(), context.popTerminal().value });
}

void doubleType(AbstractSyntaxTreeBuilderContext& context) {
    // Phase 3: full float/double codegen; front-end accepts the type specifier.
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
        throw std::runtime_error { "unknown typedef name: " + name.value };
    }
    context.pushTypeSpecifier(TypeSpecifier { *type, name.value });
}

void structOrUnionType(AbstractSyntaxTreeBuilderContext& context) {
    // type_spec -> struct_or_union_spec: TypeSpecifier already pushed.
}

void enumType(AbstractSyntaxTreeBuilderContext& context) {
    // type_spec -> enum_spec: TypeSpecifier already pushed by enum_spec productions.
    (void)context;
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

void functionSpecifier(AbstractSyntaxTreeBuilderContext& context) {
    context.popTerminal();
}

void functionSpecifierOnly(AbstractSyntaxTreeBuilderContext& context) {
    context.pushDeclarationSpecifiers(DeclarationSpecifiers::none());
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

void parenthesizedExpression(AbstractSyntaxTreeBuilderContext& context) {
    context.popTerminal();
    context.popTerminal();
}

void parenthesizedDeclarator(AbstractSyntaxTreeBuilderContext& context) {
    context.popTerminal();
    context.popTerminal();
    context.pushDirectDeclarator(std::make_unique<ParenthesizedDeclarator>(context.popDeclarator()));
}

void declarationTypeSpecifier(AbstractSyntaxTreeBuilderContext& context) {
    context.pushDeclarationSpecifiers( { context.popTypeSpecifier() });
}

void addDeclarationTypeSpecifier(AbstractSyntaxTreeBuilderContext& context) {
    auto declarationSpecifiers = context.popDeclarationSpecifiers();
    auto typeSpecifier = context.popTypeSpecifier();
    context.pushDeclarationSpecifiers( { typeSpecifier, declarationSpecifiers });
}

DeclarationSpecifiers popResolvedSpecQualifiers(AbstractSyntaxTreeBuilderContext& context) {
    auto specs = context.popDeclarationSpecifiers();
    if (!specs.resolveTypeofAtParseTime(context.environment())) {
        throw std::runtime_error { "cannot determine type of typeof operand" };
    }
    if (specs.getTypeSpecifiers().empty()) {
        throw std::runtime_error { "cannot determine type of spec-qualifier-list" };
    }
    return specs;
}

void specQualifierListTypeName(AbstractSyntaxTreeBuilderContext& context) {
    context.pushTypeSpecifier(popResolvedSpecQualifiers(context).toTypeSpecifier());
}

void declarationStorageClassSpecifier(AbstractSyntaxTreeBuilderContext& context) {
    context.pushDeclarationSpecifiers( { context.popStorageSpecifier() });
}

void addDeclarationStorageClassSpecifier(AbstractSyntaxTreeBuilderContext& context) {
    auto declarationSpecifiers = context.popDeclarationSpecifiers();
    auto storageSpecifier = context.popStorageSpecifier();
    context.pushDeclarationSpecifiers( { storageSpecifier, declarationSpecifiers });
}

void declarationTypeQualifier(AbstractSyntaxTreeBuilderContext& context) {
    context.pushDeclarationSpecifiers( { context.popTypeQualifier() });
}

void addDeclarationTypeQualifier(AbstractSyntaxTreeBuilderContext& context) {
    auto declarationSpecifiers = context.popDeclarationSpecifiers();
    auto typeQualifier = context.popTypeQualifier();
    context.pushDeclarationSpecifiers( { typeQualifier, declarationSpecifiers });
}

void identifierDeclarator(AbstractSyntaxTreeBuilderContext& context) {
    context.pushDirectDeclarator(std::make_unique<Identifier>(context.popTerminal()));
}

void arrayDeclarator(AbstractSyntaxTreeBuilderContext& context) {
    context.popTerminal();
    context.popTerminal();
    context.pushDirectDeclarator(std::make_unique<ArrayDeclarator>(context.popDirectDeclarator(), context.popExpression()));
}

void arrayDeclaratorQualified(AbstractSyntaxTreeBuilderContext& context) {
    context.popTypeQualifierList();
    arrayDeclarator(context);
}

namespace {

std::unique_ptr<Identifier> anonymousIdentifier() {
    return std::make_unique<Identifier>(TerminalSymbol { "id", "", translation_unit::Context { "", 0 } });
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
    FormalArgument argument { specs, std::move(declarator) };
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

namespace {

type::Type integerLiteralType(const std::string& token) {
    util::IntegerLiteral lit;
    if (!util::parseIntegerLiteral(token, lit)) {
        return type::signedInteger();
    }
    const util::WideUInt u64max = ~0ull;
    if (lit.value > u64max) {
        const util::WideUInt i128max = (((util::WideUInt)1) << 127) - 1;
        if (lit.uns || lit.value > i128max) {
            return type::unsignedInt128();
        }
        return type::signedInt128();
    }
    if (lit.lng) {
        return lit.uns ? type::unsignedLong() : type::signedLong();
    }
    return lit.uns ? type::unsignedInteger() : type::signedInteger();
}

} // namespace

void integerConstant(AbstractSyntaxTreeBuilderContext& context) {
    auto constant = context.popTerminal();
    context.pushConstant( { constant.value, integerLiteralType(constant.value), constant.context });
}

void characterConstant(AbstractSyntaxTreeBuilderContext& context) {
    auto constant = context.popTerminal();
    context.pushConstant( { constant.value, type::signedInteger(), constant.context });
}

void floatConstant(AbstractSyntaxTreeBuilderContext& context) {
    auto constant = context.popTerminal();
    const int size = util::floatingLiteralSizeBytes(constant.value);
    type::Type t = type::doubleFloating();
    if (size == 4) {
        t = type::floating();
    } else if (size == 16) {
        t = type::longDoubleFloating();
    }
    context.pushConstant( { constant.value, t, constant.context });
}

void enumerationConstant(AbstractSyntaxTreeBuilderContext& context) {
    // Grammar reserves enumeration_const; the scanner emits plain id for
    // enumerators, so this reduction is not used on the product path.
    (void)context;
    throw std::logic_error { "enumeration_const reduction is unused (scanner emits id)" };
}

void identifierExpression(AbstractSyntaxTreeBuilderContext& context) {
    auto identifier = context.popTerminal();
    auto expr = std::make_unique<IdentifierExpression>(identifier.value, identifier.context);
    // Fold parse-time enumerators so evaluateConstant works without process globals.
    long enumValue = 0;
    if (context.environment().lookupEnumConstant(identifier.value, enumValue)) {
        expr->setFoldedConstant(enumValue);
    }
    context.pushExpression(std::move(expr));
}

void constantExpression(AbstractSyntaxTreeBuilderContext& context) {
    context.pushExpression(std::make_unique<ConstantExpression>(context.popConstant()));
}

void stringLiteralExpression(AbstractSyntaxTreeBuilderContext& context) {
    auto literal = context.popTerminal();
    context.pushExpression(std::make_unique<StringLiteralExpression>(literal.value, literal.context));
}

void arrayAccess(AbstractSyntaxTreeBuilderContext& context) {
    context.popTerminal(); // ]
    context.popTerminal(); // [
    auto subscriptExpression = context.popExpression();
    auto postfixExpression = context.popExpression();
    context.pushExpression(std::make_unique<ArrayAccess>(std::move(postfixExpression), std::move(subscriptExpression)));
}

void functionCall(AbstractSyntaxTreeBuilderContext& context) {
    context.popTerminal(); // )
    context.popTerminal(); // (
    context.pushExpression(std::make_unique<FunctionCall>(context.popExpression(), context.popActualArgumentsList()));
}

void noargFunctionCall(AbstractSyntaxTreeBuilderContext& context) {
    context.popTerminal(); // )
    context.popTerminal(); // (
    context.pushExpression(std::make_unique<FunctionCall>(context.popExpression()));
}

void directMemberAccess(AbstractSyntaxTreeBuilderContext& context) {
    auto member = context.popTerminal(); // id
    context.popTerminal(); // .
    auto base = context.popExpression();
    context.pushExpression(std::make_unique<MemberAccess>(
            std::move(base), member.value, false, member.context));
}

void pointeeMemberAccess(AbstractSyntaxTreeBuilderContext& context) {
    auto member = context.popTerminal(); // id
    context.popTerminal(); // ->
    auto base = context.popExpression();
    context.pushExpression(std::make_unique<MemberAccess>(
            std::move(base), member.value, true, member.context));
}

void postfixIncrementDecrement(AbstractSyntaxTreeBuilderContext& context) {
    context.pushExpression(std::make_unique<PostfixExpression>(context.popExpression(), std::make_unique<Operator>(context.popTerminal().type)));
}

void prefixIncrementDecrement(AbstractSyntaxTreeBuilderContext& context) {
    context.pushExpression(std::make_unique<PrefixExpression>(std::make_unique<Operator>(context.popTerminal().value), context.popExpression()));
}

void unaryExpression(AbstractSyntaxTreeBuilderContext& context) {
    context.pushExpression(std::make_unique<UnaryExpression>(std::make_unique<Operator>(context.popTerminal().value), context.popExpression()));
}

void sizeofExpression(AbstractSyntaxTreeBuilderContext& context) {
    context.popTerminal(); // sizeof
    context.pushExpression(std::make_unique<UnaryExpression>(
            std::make_unique<Operator>("sizeof"), context.popExpression()));
}

void typeofTypeName(AbstractSyntaxTreeBuilderContext& context) {
    context.popTerminal(); // )
    context.popTerminal(); // (
    context.popTerminal(); // typeof
    auto typeSpec = context.popTypeSpecifier();
    typeSpec.dropSpelling();
    context.pushTypeSpecifier(std::move(typeSpec));
}

void typeofExpression(AbstractSyntaxTreeBuilderContext& context) {
    context.popTerminal(); // )
    auto expr = context.popExpression();
    context.popTerminal(); // (
    context.popTerminal(); // typeof
    if (expr->hasExpressionType()) {
        context.pushTypeSpecifier(TypeSpecifier { expr->expressionType(), "" });
        return;
    }
    context.pushTypeSpecifier(TypeSpecifier { std::shared_ptr<Expression> { std::move(expr) } });
}

void genericAssociationTyped(AbstractSyntaxTreeBuilderContext& context) {
    auto expr = context.popExpression();
    context.popTerminal(); // :
    context.pushGenericAssociation(GenericAssociation { context.popTypeSpecifier(), std::move(expr) });
}

void genericAssociationDefault(AbstractSyntaxTreeBuilderContext& context) {
    auto expr = context.popExpression();
    context.popTerminal(); // :
    context.popTerminal(); // default
    context.pushGenericAssociation(GenericAssociation { std::nullopt, std::move(expr) });
}

void genericAssocListFirst(AbstractSyntaxTreeBuilderContext& context) {
    context.newGenericAssocList(context.popGenericAssociation());
}

void genericAssocListAppend(AbstractSyntaxTreeBuilderContext& context) {
    context.popTerminal();
    context.addGenericAssociation(context.popGenericAssociation());
}

void genericSelection(AbstractSyntaxTreeBuilderContext& context) {
    context.popTerminal(); // )
    auto associations = context.popGenericAssocList();
    context.popTerminal(); // ,
    auto controlling = context.popExpression();
    context.popTerminal(); // (
    auto kw = context.popTerminal(); // _Generic
    context.pushExpression(std::make_unique<GenericSelection>(
            kw.context, std::move(controlling), std::move(associations)));
}

void nullptrExpression(AbstractSyntaxTreeBuilderContext& context) {
    auto kw = context.popTerminal();
    context.pushExpression(std::make_unique<ConstantExpression>(
            Constant { "0", type::pointer(type::voidType()), kw.context }));
}

namespace {

void boolConstantExpression(AbstractSyntaxTreeBuilderContext& context, const char* digits) {
    auto kw = context.popTerminal();
    context.pushExpression(std::make_unique<ConstantExpression>(
            Constant { digits, type::boolean(), kw.context }));
}

} // namespace

void trueExpression(AbstractSyntaxTreeBuilderContext& context) {
    boolConstantExpression(context, "1");
}

void falseExpression(AbstractSyntaxTreeBuilderContext& context) {
    boolConstantExpression(context, "0");
}

void sizeofTypeExpression(AbstractSyntaxTreeBuilderContext& context) {
    context.popTerminal(); // )
    context.popTerminal(); // (
    // type_name left a TypeSpecifier for simple types (int, char, long, pointers via abstract decl later).
    auto typeSpec = context.popTypeSpecifier();
    auto sizeofKw = context.popTerminal(); // sizeof
    if (!typeSpec.resolveTypeofAtParseTime(context.environment())) {
        throw std::runtime_error { "cannot determine type of typeof operand" };
    }
    const type::Type& namedType = typeSpec.getType();
    // sizeof(void) / bare function / incomplete records are invalid. Pointers
    // (incl. pointer-to-function and pointer-to-void) remain complete.
    // Empty complete records size 0 are valid; only incomplete tags are rejected.
    if (type::isIncompleteObjectType(namedType)) {
        throw std::runtime_error {
                "invalid application of ‘sizeof’ to incomplete type ‘" + namedType.to_string() + "’" };
    }
    const int size = namedType.getSize();
    context.pushExpression(std::make_unique<ConstantExpression>(
            Constant { std::to_string(size), type::signedInteger(), sizeofKw.context }));
}

void typeNameWithAbstractDeclarator(AbstractSyntaxTreeBuilderContext& context) {
    auto declarator = context.popDeclarator();
    auto typeSpec = popResolvedSpecQualifiers(context).toTypeSpecifier();
    typeSpec.deferAbstractDeclarator(std::move(declarator));
    context.pushTypeSpecifier(std::move(typeSpec));
}

void typeCast(AbstractSyntaxTreeBuilderContext& context) {
    context.popTerminal(); // )
    auto castExpression = context.popExpression();
    context.popTerminal(); // (
    auto typeSpec = context.popTypeSpecifier();
    context.pushExpression(std::make_unique<TypeCast>(std::move(typeSpec), std::move(castExpression)));
}

void arithmeticExpression(AbstractSyntaxTreeBuilderContext& context) {
    auto rightHandSide = context.popExpression();
    auto leftHandSide = context.popExpression();
    auto arithmeticOperator = std::make_unique<Operator>(context.popTerminal().value);
    context.pushExpression(std::make_unique<ArithmeticExpression>(std::move(leftHandSide), std::move(arithmeticOperator), std::move(rightHandSide)));
}

void shiftExpression(AbstractSyntaxTreeBuilderContext& context) {
    auto additionExpression = context.popExpression();
    auto shiftExpression = context.popExpression();
    auto shiftOperator = std::make_unique<Operator>(context.popTerminal().value);
    context.pushExpression(std::make_unique<ShiftExpression>(std::move(shiftExpression), std::move(shiftOperator), std::move(additionExpression)));
}

void relationalExpression(AbstractSyntaxTreeBuilderContext& context) {
    auto rightHandSide = context.popExpression();
    auto leftHandSide = context.popExpression();
    auto comparisonOperator = std::make_unique<Operator>(context.popTerminal().value);
    context.pushExpression(std::make_unique<ComparisonExpression>(std::move(leftHandSide), std::move(comparisonOperator), std::move(rightHandSide)));
}

void bitwiseExpression(AbstractSyntaxTreeBuilderContext& context) {
    auto rightHandSide = context.popExpression();
    auto leftHandSide = context.popExpression();
    auto bitwiseOperator = std::make_unique<Operator>(context.popTerminal().value);
    context.pushExpression(std::make_unique<BitwiseExpression>(std::move(leftHandSide), std::move(bitwiseOperator), std::move(rightHandSide)));
}

void logicalAndExpression(AbstractSyntaxTreeBuilderContext& context) {
    context.popTerminal();
    auto rightHandSide = context.popExpression();
    auto leftHandSide = context.popExpression();
    context.pushExpression(std::make_unique<LogicalAndExpression>(std::move(leftHandSide), std::move(rightHandSide)));
}

void logicalOrExpression(AbstractSyntaxTreeBuilderContext& context) {
    context.popTerminal();
    auto rightHandSide = context.popExpression();
    auto leftHandSide = context.popExpression();
    context.pushExpression(std::make_unique<LogicalOrExpression>(std::move(leftHandSide), std::move(rightHandSide)));
}

void conditionalExpression(AbstractSyntaxTreeBuilderContext& context) {
    // Production: <logical_or_exp> '?' <exp> ':' <conditional_exp>
    // Expressions reduce LIFO: false arm, true arm, then condition; then '?' / ':'.
    context.popTerminal(); // :
    context.popTerminal(); // ?
    auto falseExpression = context.popExpression();
    auto trueExpression = context.popExpression();
    auto condition = context.popExpression();
    context.pushExpression(std::make_unique<ConditionalExpression>(
            std::move(condition), std::move(trueExpression), std::move(falseExpression)));
}

void assignmentExpression(AbstractSyntaxTreeBuilderContext& context) {
    auto rightHandSide = context.popExpression();
    auto leftHandSide = context.popExpression();
    auto assignmentOperator = std::make_unique<Operator>(context.popTerminal().value);
    context.pushExpression(
            std::make_unique<AssignmentExpression>(std::move(leftHandSide), std::move(assignmentOperator), std::move(rightHandSide)));
}

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

void initializedDeclaration(AbstractSyntaxTreeBuilderContext& context) {
    context.popTerminal();
    auto declarationSpecifiers = context.popDeclarationSpecifiers();
    auto initializedDeclarators = context.popInitializedDeclarators();
    if (!declarationSpecifiers.resolveTypeofAtParseTime(context.environment())
            && declarationSpecifiers.isTypedef()) {
        throw std::runtime_error { "cannot determine type of typeof operand" };
    }
    context.environment().registerInitializedDeclaration(declarationSpecifiers, initializedDeclarators);
    context.pushDeclaration(std::make_unique<Declaration>(declarationSpecifiers, std::move(initializedDeclarators)));
}

void declaration(AbstractSyntaxTreeBuilderContext& context) {
    context.popTerminal();
    auto declarationSpecifiers = context.popDeclarationSpecifiers();
    context.pushDeclaration(std::make_unique<Declaration>(declarationSpecifiers));
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

void expressionList(AbstractSyntaxTreeBuilderContext& context) {
    context.popTerminal();
    auto rightHandSide = context.popExpression();
    auto leftHandSide = context.popExpression();
    context.pushExpression(std::make_unique<ExpressionList>(std::move(leftHandSide), std::move(rightHandSide)));
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

void ifStatement(AbstractSyntaxTreeBuilderContext& context) {
    context.popTerminal();
    context.popTerminal();
    context.popTerminal();
    context.pushStatement(std::make_unique<IfStatement>(context.popExpression(), context.popStatement()));
}

void ifElseStatement(AbstractSyntaxTreeBuilderContext& context) {
    context.popTerminal();
    context.popTerminal();
    context.popTerminal();
    context.popTerminal();
    auto falsyStatement = context.popStatement();
    auto truthyStatement = context.popStatement();
    context.pushStatement(std::make_unique<IfElseStatement>(context.popExpression(), std::move(truthyStatement), std::move(falsyStatement)));
}

void whileLoopStatement(AbstractSyntaxTreeBuilderContext& context) {
    context.popTerminal();
    context.popTerminal();
    context.popTerminal();
    auto loopHeader = std::make_unique<WhileLoopHeader>(context.popExpression());
    auto body = context.popStatement();
    context.pushStatement(std::make_unique<LoopStatement>(std::move(loopHeader), std::move(body)));
}

void namedLabel(AbstractSyntaxTreeBuilderContext& context) {
    context.popTerminal(); // :
    auto labelName = context.popTerminal(); // id
    auto statement = context.popStatement();
    context.pushStatement(std::make_unique<LabeledStatement>(labelName, std::move(statement)));
}

void switchStatement(AbstractSyntaxTreeBuilderContext& context) {
    context.popTerminal(); // )
    context.popTerminal(); // (
    context.popTerminal(); // switch
    auto body = context.popStatement();
    context.pushStatement(std::make_unique<SwitchStatement>(context.popExpression(), std::move(body)));
}

void caseLabel(AbstractSyntaxTreeBuilderContext& context) {
    context.popTerminal(); // :
    context.popTerminal(); // case
    auto statement = context.popStatement();
    context.pushStatement(std::make_unique<CaseLabel>(context.popExpression(), std::move(statement)));
}

void defaultLabel(AbstractSyntaxTreeBuilderContext& context) {
    context.popTerminal(); // :
    auto defaultKeyword = context.popTerminal(); // default
    auto statement = context.popStatement();
    context.pushStatement(std::make_unique<DefaultLabel>(defaultKeyword, std::move(statement)));
}

void gotoStatement(AbstractSyntaxTreeBuilderContext& context) {
    context.popTerminal(); // ;
    auto labelName = context.popTerminal(); // id
    auto gotoKeyword = context.popTerminal(); // goto
    context.pushStatement(std::make_unique<GotoStatement>(gotoKeyword, labelName));
}

void doWhileLoopStatement(AbstractSyntaxTreeBuilderContext& context) {
    // Production: 'do' <stat> 'while' '(' <exp> ')' ';'
    context.popTerminal(); // ;
    context.popTerminal(); // )
    context.popTerminal(); // (
    context.popTerminal(); // while
    context.popTerminal(); // do
    auto clause = context.popExpression();
    auto body = context.popStatement();
    auto loopHeader = std::make_unique<DoWhileLoopHeader>(std::move(clause));
    context.pushStatement(std::make_unique<LoopStatement>(std::move(loopHeader), std::move(body)));
}

void statementList(AbstractSyntaxTreeBuilderContext& context) {
    context.newStatementList(context.popStatement());
}

void addToStatementList(AbstractSyntaxTreeBuilderContext& context) {
    context.addToStatementList(context.popStatement());
}

void returnExpressionStatement(AbstractSyntaxTreeBuilderContext& context) {
    context.popTerminal();
    context.popTerminal();
    context.pushStatement(std::make_unique<ReturnStatement>(context.popExpression()));
}

void returnVoidStatement(AbstractSyntaxTreeBuilderContext& context) {
    context.popTerminal();
    context.popTerminal();
    context.pushStatement(std::make_unique<VoidReturnStatement>());
}

void createActualArgumentsList(AbstractSyntaxTreeBuilderContext& context) {
    context.newActualArgumentsList(context.popExpression());
}

void addToActualArgumentsList(AbstractSyntaxTreeBuilderContext& context) {
    context.popTerminal();
    context.addToActualArgumentsList(context.popExpression());
}

void emptyCompound(AbstractSyntaxTreeBuilderContext& context) {
    context.popTerminal();
    context.popTerminal();
    context.pushStatement(std::make_unique<Block>(std::vector<std::unique_ptr<AbstractSyntaxTreeNode>> {}));
}

void blockItemListCompound(AbstractSyntaxTreeBuilderContext& context) {
    context.popTerminal(); // }
    context.popTerminal(); // {
    context.pushStatement(std::make_unique<Block>(context.popStatementList()));
}

void blockItemDeclaration(AbstractSyntaxTreeBuilderContext& context) {
    // Declaration becomes a block item on the shared statement/item stack.
    context.pushStatement(context.popDeclaration());
}

void expressionStatement(AbstractSyntaxTreeBuilderContext& context) {
    context.popTerminal();
    context.pushStatement(context.popExpression());
}

void emptyStatement(AbstractSyntaxTreeBuilderContext& context) {
    context.popTerminal();
    // Null statement `;` still occupies a statement slot so parents (if/while/for/stat_list)
    // can pop a body without under-flowing the AST statement stack.
    context.pushStatement(std::make_unique<Block>(std::vector<std::unique_ptr<AbstractSyntaxTreeNode>> {}));
}

void functionDefinition(AbstractSyntaxTreeBuilderContext& context) {
    auto declarationSpecifiers = context.popDeclarationSpecifiers();
    auto declarator = context.popDeclarator();
    auto statement = context.popStatement();
    context.pushStatement(std::make_unique<FunctionDefinition>(std::move(declarationSpecifiers), std::move(declarator), std::move(statement)));
}

void defaultReturnTypeFunctionDefinition(AbstractSyntaxTreeBuilderContext& context) {
    DeclarationSpecifiers defaultReturnTypeSpecifiers { TypeSpecifier { type::signedInteger(), "int" } };
    context.pushStatement(std::make_unique<FunctionDefinition>(defaultReturnTypeSpecifiers, context.popDeclarator(), context.popStatement()));
}

void externalFunctionDefinition(AbstractSyntaxTreeBuilderContext& context) {
    context.pushExternalDeclaration(context.popStatement());
}

void externalDeclaration(AbstractSyntaxTreeBuilderContext& context) {
    context.pushExternalDeclaration(context.popDeclaration());
}

void translationUnit(AbstractSyntaxTreeBuilderContext& context) {
    context.addToTranslationUnit(context.popExternalDeclaration());
}

void addToTranslationUnit(AbstractSyntaxTreeBuilderContext& context) {
    context.addToTranslationUnit(context.popExternalDeclaration());
}


} // namespace ast
