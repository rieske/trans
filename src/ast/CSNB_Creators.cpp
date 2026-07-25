#include "CSNB_Internal.h"

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
    throw std::runtime_error { "short type is not implemented yet" };
}

void integerType(AbstractSyntaxTreeBuilderContext& context) {
    context.pushTypeSpecifier( { type::signedInteger(), context.popTerminal().value });
}

void longType(AbstractSyntaxTreeBuilderContext& context) {
    throw std::runtime_error { "long type is not implemented yet" };
}

void characterType(AbstractSyntaxTreeBuilderContext& context) {
    context.pushTypeSpecifier( { type::signedCharacter(), context.popTerminal().value });
}

void voidType(AbstractSyntaxTreeBuilderContext& context) {
    context.pushTypeSpecifier( { type::voidType(), context.popTerminal().value });
}

void floatType(AbstractSyntaxTreeBuilderContext& context) {
    context.pushTypeSpecifier( { type::floating(), context.popTerminal().value });
}

void doubleType(AbstractSyntaxTreeBuilderContext& context) {
    throw std::runtime_error { "double type is not implemented yet" };
}

void signedType(AbstractSyntaxTreeBuilderContext& context) {
    throw std::runtime_error { "signed type is not implemented yet" };
}

void unsignedType(AbstractSyntaxTreeBuilderContext& context) {
    throw std::runtime_error { "unsigned type is not implemented yet" };
}

void typedefName(AbstractSyntaxTreeBuilderContext& context) {
    throw std::runtime_error { "typedefName type is not implemented yet" };
}

void structOrUnionType(AbstractSyntaxTreeBuilderContext& context) {
    // type_spec -> struct_or_union_spec: TypeSpecifier already pushed.
}

void enumType(AbstractSyntaxTreeBuilderContext& context) {
    throw std::runtime_error { "enumType type is not implemented yet" };
}

void constQualifier(AbstractSyntaxTreeBuilderContext& context) {
    context.pushTypeQualifier(type::Qualifier::CONST);
}

void volatileQualifier(AbstractSyntaxTreeBuilderContext& context) {
    context.pushTypeQualifier(type::Qualifier::VOLATILE);
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

void abstractArrayDeclarator(AbstractSyntaxTreeBuilderContext& context) {
    context.popTerminal();
    context.popTerminal();
    //context.pushDeclarator(std::make_unique<ArrayDeclarator>(context.popDirectDeclarator()));
    throw std::runtime_error { "abstract array declarator is not implemented yet" };
}

void functionDeclarator(AbstractSyntaxTreeBuilderContext& context) {
    context.popTerminal();
    context.popTerminal();
    auto arguments = context.popArgumentsDeclaration().first;
    // `(void)` is an empty parameter list, not a single void parameter.
    if (arguments.size() == 1 && arguments.front().isVoid()) {
        arguments.clear();
    }
    context.pushDirectDeclarator(std::make_unique<FunctionDeclarator>(context.popDirectDeclarator(), std::move(arguments)));
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
    context.pushFormalArgument(FormalArgument { context.popDeclarationSpecifiers(), context.popDeclarator() });
}

void abstractParameterDeclaration(AbstractSyntaxTreeBuilderContext& context) {
    // `<decl_specs> <abstract_declarator>` — e.g. `int *` as a parameter type.
    context.pushFormalArgument(FormalArgument { context.popDeclarationSpecifiers(), context.popDeclarator() });
}

// abstract_declarator ::= <pointer>  (unnamed pointer parameter / type name)
void abstractPointerDeclarator(AbstractSyntaxTreeBuilderContext& context) {
    context.pushDeclarator(std::make_unique<Declarator>(
            std::make_unique<Identifier>(TerminalSymbol { "id", "", translation_unit::Context { "", 0 } }),
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
    //context.pushArgumentsDeclaration(std::make_pair(context.popFormalArguments(), true));
    throw std::runtime_error { "formalArgumentsWithVararg is not implemented yet" };
}

void integerConstant(AbstractSyntaxTreeBuilderContext& context) {
    auto constant = context.popTerminal();
    context.pushConstant( { constant.value, type::signedInteger(), constant.context });
}

void characterConstant(AbstractSyntaxTreeBuilderContext& context) {
    auto constant = context.popTerminal();
    context.pushConstant( { constant.value, type::signedCharacter(), constant.context });
}

void floatConstant(AbstractSyntaxTreeBuilderContext& context) {
    auto constant = context.popTerminal();
    throw std::runtime_error { "floating constants not implemented yet" };
    // context.pushConstant( { constant.value, type::floating(), constant.context });
}

void enumerationConstant(AbstractSyntaxTreeBuilderContext& context) {
    auto constant = context.popTerminal();
    throw std::runtime_error { "enumerationConstant is not implemented yet" };
}

void identifierExpression(AbstractSyntaxTreeBuilderContext& context) {
    auto identifier = context.popTerminal();
    context.pushExpression(std::make_unique<IdentifierExpression>(identifier.value, identifier.context));
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

void sizeofTypeExpression(AbstractSyntaxTreeBuilderContext& context) {
    context.popTerminal(); // )
    context.popTerminal(); // (
    // type_name left a TypeSpecifier for simple types (int, char, long, pointers via abstract decl later).
    auto typeSpec = context.popTypeSpecifier();
    auto sizeofKw = context.popTerminal(); // sizeof
    const type::Type& namedType = typeSpec.getType();
    // sizeof(void) / bare function types are invalid. Pointers (incl. pointer-to-function
    // and pointer-to-void) remain complete.
    if (namedType.isVoid() || type::isBareFunction(namedType)) {
        throw std::runtime_error {
                "invalid application of ‘sizeof’ to incomplete type ‘" + namedType.to_string() + "’" };
    }
    const int size = namedType.getSize();
    context.pushExpression(std::make_unique<ConstantExpression>(
            Constant { std::to_string(size), type::signedInteger(), sizeofKw.context }));
}

void typeNameWithAbstractDeclarator(AbstractSyntaxTreeBuilderContext& context) {
    auto declarator = context.popDeclarator();
    auto typeSpec = context.popTypeSpecifier();
    auto combined = declarator->getFundamentalType(typeSpec.getType());
    context.pushTypeSpecifier(TypeSpecifier { combined, typeSpec.getName() });
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

void initializerListFirst(AbstractSyntaxTreeBuilderContext& context) {
    context.newInitializerList();
    context.addInitializerElement(context.popExpression());
}

void initializerListAppend(AbstractSyntaxTreeBuilderContext& context) {
    context.popTerminal(); // ,
    context.addInitializerElement(context.popExpression());
}

void initializedDeclarator(AbstractSyntaxTreeBuilderContext& context) {
    auto declarator = context.popDeclarator();
    context.pushInitializedDeclarator(std::make_unique<InitializedDeclarator>(std::move(declarator)));
}

void initializedDeclaratorWithInitializer(AbstractSyntaxTreeBuilderContext& context) {
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
