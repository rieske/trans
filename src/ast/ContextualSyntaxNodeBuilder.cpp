#include "CSNB_Internal.h"

#include <limits>

namespace ast {
namespace {

int foldBitFieldWidth(AbstractSyntaxTreeBuilderContext& context) {
    auto widthExpr = context.popExpression();
    context.popTerminal();
    long width = 0;
    if (!widthExpr || !widthExpr->evaluateConstant(width) || width < 0
            || width > static_cast<long>(std::numeric_limits<int>::max())) {
        throw std::runtime_error { "bit-field width is not a constant" };
    }
    return static_cast<int>(width);
}

} // namespace

ContextualSyntaxNodeBuilder::ContextualSyntaxNodeBuilder(const parser::Grammar& grammar) {
    this->grammar = &grammar;

    int s_type_specifier = grammar.symbolId("<type_spec>");
    nodeCreatorRegistry[s_type_specifier][{ grammar.symbolId("short") }] = shortType;
    nodeCreatorRegistry[s_type_specifier][{ grammar.symbolId("int") }] = integerType;
    nodeCreatorRegistry[s_type_specifier][{ grammar.symbolId("long") }] = longType;
    nodeCreatorRegistry[s_type_specifier][{ grammar.symbolId("char") }] = characterType;
    nodeCreatorRegistry[s_type_specifier][{ grammar.symbolId("void") }] = voidType;
    nodeCreatorRegistry[s_type_specifier][{ grammar.symbolId("float") }] = floatType;
    nodeCreatorRegistry[s_type_specifier][{ grammar.symbolId("double") }] = doubleType;
    nodeCreatorRegistry[s_type_specifier][{ grammar.symbolId("signed") }] = signedType;
    nodeCreatorRegistry[s_type_specifier][{ grammar.symbolId("unsigned") }] = unsignedType;
    nodeCreatorRegistry[s_type_specifier][{ grammar.symbolId("bool") }] = boolType;
    nodeCreatorRegistry[s_type_specifier][{ grammar.symbolId("_Complex") }] = complexType;
    nodeCreatorRegistry[s_type_specifier][{ grammar.symbolId("typedef_name") }] = typedefName;
    nodeCreatorRegistry[s_type_specifier][{ grammar.symbolId("<struct_or_union_spec>") }] = structOrUnionType;
    nodeCreatorRegistry[s_type_specifier][{ grammar.symbolId("<enum_spec>") }] = enumType;

    int s_type_qualifier = grammar.symbolId("<type_qualifier>");
    nodeCreatorRegistry[s_type_qualifier][{ grammar.symbolId("const") }] = constQualifier;
    nodeCreatorRegistry[s_type_qualifier][{ grammar.symbolId("volatile") }] = volatileQualifier;
    nodeCreatorRegistry[s_type_qualifier][{ grammar.symbolId("restrict") }] = restrictQualifier;

    int s_type_qualifier_list = grammar.symbolId("<type_qualifier_list>");
    nodeCreatorRegistry[s_type_qualifier_list][{ s_type_qualifier }] = typeQualifierList;
    nodeCreatorRegistry[s_type_qualifier_list][{ s_type_qualifier_list, s_type_qualifier }] = addTypeQualifierToList;

    int s_storage_class_spec = grammar.symbolId("<storage_class_spec>");
    nodeCreatorRegistry[s_storage_class_spec][{ grammar.symbolId("auto") }] = autoStorageClass;
    nodeCreatorRegistry[s_storage_class_spec][{ grammar.symbolId("register") }] = registerStorageClass;
    nodeCreatorRegistry[s_storage_class_spec][{ grammar.symbolId("static") }] = staticStorageClass;
    nodeCreatorRegistry[s_storage_class_spec][{ grammar.symbolId("extern") }] = externStorageClass;
    nodeCreatorRegistry[s_storage_class_spec][{ grammar.symbolId("typedef") }] = typedefStorageClass;

    int s_function_spec = grammar.symbolId("<function_spec>");
    nodeCreatorRegistry[s_function_spec][{ grammar.symbolId("inline") }] = functionSpecifier;
    nodeCreatorRegistry[s_function_spec][{ grammar.symbolId("noreturn") }] = functionSpecifier;

    int s_decl_specs = grammar.symbolId("<decl_specs>");
    nodeCreatorRegistry[s_decl_specs][{ s_type_specifier }] = declarationTypeSpecifier;
    nodeCreatorRegistry[s_decl_specs][{ s_type_specifier, s_decl_specs }] = addDeclarationTypeSpecifier;
    nodeCreatorRegistry[s_decl_specs][{ s_storage_class_spec }] = declarationStorageClassSpecifier;
    nodeCreatorRegistry[s_decl_specs][{ s_storage_class_spec, s_decl_specs }] = addDeclarationStorageClassSpecifier;
    nodeCreatorRegistry[s_decl_specs][{ s_type_qualifier }] = declarationTypeQualifier;
    nodeCreatorRegistry[s_decl_specs][{ s_type_qualifier, s_decl_specs }] = addDeclarationTypeQualifier;
    nodeCreatorRegistry[s_decl_specs][{ s_function_spec }] = functionSpecifierOnly;
    nodeCreatorRegistry[s_decl_specs][{ s_function_spec, s_decl_specs }] = doNothing;

    int s_direct_declarator = grammar.symbolId("<direct_declarator>");
    int s_declarator = grammar.symbolId("<declarator>");
    int s_param_type_list = grammar.symbolId("<param_type_list>");
    int s_identifier = grammar.symbolId("id");
    int s_open_paren = grammar.symbolId("(");
    int s_close_paren = grammar.symbolId(")");
    int s_open_bracket = grammar.symbolId("[");
    int s_close_bracket = grammar.symbolId("]");

    nodeCreatorRegistry[s_direct_declarator][{ s_identifier }] = identifierDeclarator;
    nodeCreatorRegistry[s_direct_declarator][{ s_open_paren, s_declarator, s_close_paren }] = parenthesizedDeclarator;
    nodeCreatorRegistry[s_direct_declarator][{ s_direct_declarator, s_open_bracket, grammar.symbolId("<const_exp>"), s_close_bracket }] = arrayDeclarator;
    nodeCreatorRegistry[s_direct_declarator][{ s_direct_declarator, s_open_bracket, s_close_bracket }] = abstractArrayDeclarator;
    nodeCreatorRegistry[s_direct_declarator][{
            s_direct_declarator, s_open_bracket, s_type_qualifier_list, s_close_bracket }] =
            abstractArrayDeclaratorQualified;
    nodeCreatorRegistry[s_direct_declarator][{
            s_direct_declarator, s_open_bracket, s_type_qualifier_list, grammar.symbolId("<const_exp>"), s_close_bracket }] =
            arrayDeclaratorQualified;
    nodeCreatorRegistry[s_direct_declarator][{ s_direct_declarator, s_open_paren, s_param_type_list, s_close_paren }] = functionDeclarator;
    nodeCreatorRegistry[s_direct_declarator][{ s_direct_declarator, s_open_paren, s_close_paren }] = noargFunctionDeclarator;

    int s_pointer = grammar.symbolId("<pointer>" );
    nodeCreatorRegistry[s_pointer][{ grammar.symbolId("*"), s_type_qualifier_list }] = qualifiedPointer;
    nodeCreatorRegistry[s_pointer][{ grammar.symbolId("*") }] = pointer;
    nodeCreatorRegistry[s_pointer][{ grammar.symbolId("*"), s_type_qualifier_list, s_pointer }] = qualifiedPointerToPointer;
    nodeCreatorRegistry[s_pointer][{ grammar.symbolId("*"), s_pointer }] = pointerToPointer;
    nodeCreatorRegistry[s_declarator][{ s_pointer, s_direct_declarator }] = pointerToDeclarator;
    nodeCreatorRegistry[s_declarator][{ s_direct_declarator }] = declarator;

    int s_param_decl = grammar.symbolId("<param_decl>");
    int s_abstract_declarator = grammar.symbolId("<abstract_declarator>");
    nodeCreatorRegistry[s_param_decl][{ s_decl_specs, s_declarator }] = parameterDeclaration;
    nodeCreatorRegistry[s_param_decl][{ s_decl_specs, s_abstract_declarator }] = abstractParameterDeclaration;
    nodeCreatorRegistry[s_param_decl][{ s_decl_specs }] = parameterBaseTypeDeclaration;

    // Abstract declarators share named-declarator creators once a DirectDeclarator exists.
    // Bare `[N]` / `[]` / `(params)` / `()` inject an anonymous Identifier first.
    nodeCreatorRegistry[s_abstract_declarator][{ s_pointer }] = abstractPointerDeclarator;
    int s_direct_abstract_declarator = grammar.symbolId("<direct_abstract_declarator>");
    nodeCreatorRegistry[s_abstract_declarator][{ s_direct_abstract_declarator }] = declarator;
    nodeCreatorRegistry[s_abstract_declarator][{ s_pointer, s_direct_abstract_declarator }] = pointerToDeclarator;

    nodeCreatorRegistry[s_direct_abstract_declarator][{ s_open_paren, s_abstract_declarator, s_close_paren }] =
            parenthesizedDeclarator;
    nodeCreatorRegistry[s_direct_abstract_declarator][{
            s_direct_abstract_declarator, s_open_bracket, grammar.symbolId("<const_exp>"), s_close_bracket }] =
            arrayDeclarator;
    nodeCreatorRegistry[s_direct_abstract_declarator][{
            s_open_bracket, grammar.symbolId("<const_exp>"), s_close_bracket }] = abstractArrayOnlySized;
    nodeCreatorRegistry[s_direct_abstract_declarator][{
            s_direct_abstract_declarator, s_open_bracket, s_close_bracket }] = abstractArrayDeclarator;
    nodeCreatorRegistry[s_direct_abstract_declarator][{ s_open_bracket, s_close_bracket }] = abstractArrayOnlyUnsized;
    nodeCreatorRegistry[s_direct_abstract_declarator][{
            s_direct_abstract_declarator, s_open_bracket, s_type_qualifier_list, s_close_bracket }] =
            abstractArrayDeclaratorQualified;
    nodeCreatorRegistry[s_direct_abstract_declarator][{
            s_open_bracket, s_type_qualifier_list, s_close_bracket }] = abstractArrayOnlyQualifiedUnsized;
    nodeCreatorRegistry[s_direct_abstract_declarator][{
            s_direct_abstract_declarator, s_open_bracket, s_type_qualifier_list, grammar.symbolId("<const_exp>"),
            s_close_bracket }] = arrayDeclaratorQualified;
    nodeCreatorRegistry[s_direct_abstract_declarator][{
            s_open_bracket, s_type_qualifier_list, grammar.symbolId("<const_exp>"), s_close_bracket }] =
            abstractArrayOnlyQualifiedSized;
    nodeCreatorRegistry[s_direct_abstract_declarator][{
            s_direct_abstract_declarator, s_open_paren, s_param_type_list, s_close_paren }] = functionDeclarator;
    nodeCreatorRegistry[s_direct_abstract_declarator][{
            s_open_paren, s_param_type_list, s_close_paren }] = abstractFuncOnly;
    nodeCreatorRegistry[s_direct_abstract_declarator][{
            s_direct_abstract_declarator, s_open_paren, s_close_paren }] = noargFunctionDeclarator;
    nodeCreatorRegistry[s_direct_abstract_declarator][{ s_open_paren, s_close_paren }] = abstractNoargOnly;

    int s_param_list = grammar.symbolId("<param_list>");
    int s_comma = grammar.symbolId(",");
    nodeCreatorRegistry[s_param_list][{ s_param_decl }] = formalArguments;
    nodeCreatorRegistry[s_param_list][{ s_param_list, s_comma, s_param_decl }] = addFormalArgument;

    nodeCreatorRegistry[s_param_type_list][{ s_param_list }] = formalArgumentsDeclaration;
    nodeCreatorRegistry[s_param_type_list][{ s_param_list, s_comma, grammar.symbolId("...") }] = formalArgumentsWithVararg;

    // K&R identifier parameter list: `f(a, b)` — not the modern `f(int a, int b)`.
    int s_id_list = grammar.symbolId("<id_list>");
    nodeCreatorRegistry[s_id_list][{ s_identifier }] = notImplementedYet("K&R identifier parameter lists");
    nodeCreatorRegistry[s_id_list][{ s_id_list, s_comma, s_identifier }] = notImplementedYet("K&R identifier parameter lists");
    nodeCreatorRegistry[s_direct_declarator][{ s_direct_declarator, s_open_paren, s_id_list, s_close_paren }] =
            notImplementedYet("K&R identifier parameter lists");

    int s_constant = grammar.symbolId("<const>");
    nodeCreatorRegistry[s_constant][{ grammar.symbolId("int_const") }] = integerConstant;
    nodeCreatorRegistry[s_constant][{ grammar.symbolId("char_const") }] = characterConstant;
    nodeCreatorRegistry[s_constant][{ grammar.symbolId("float_const") }] = floatConstant;
    nodeCreatorRegistry[s_constant][{ grammar.symbolId("enumeration_const") }] = enumerationConstant;

    int s_exp = grammar.symbolId("<exp>");
    int s_primary_exp = grammar.symbolId("<primary_exp>");
    nodeCreatorRegistry[s_primary_exp][{ s_identifier }] = identifierExpression;
    nodeCreatorRegistry[s_primary_exp][{ s_constant }] = constantExpression;
    nodeCreatorRegistry[s_primary_exp][{ grammar.symbolId("string") }] = stringLiteralExpression;
    nodeCreatorRegistry[s_primary_exp][{ s_open_paren, s_exp, s_close_paren }] = parenthesizedExpression;
    nodeCreatorRegistry[s_primary_exp][{ grammar.symbolId("nullptr") }] = nullptrExpression;
    nodeCreatorRegistry[s_primary_exp][{ grammar.symbolId("true") }] = trueExpression;
    nodeCreatorRegistry[s_primary_exp][{ grammar.symbolId("false") }] = falseExpression;
    int s_generic_assoc_list = grammar.symbolId("<generic_assoc_list>");
    int s_generic_association = grammar.symbolId("<generic_association>");
    int s_assignment_exp = grammar.symbolId("<assignment_exp>");
    nodeCreatorRegistry[s_primary_exp][{ grammar.symbolId("_Generic"), s_open_paren, s_assignment_exp,
            s_comma, s_generic_assoc_list, s_close_paren }] = genericSelection;
    nodeCreatorRegistry[s_generic_assoc_list][{ s_generic_association }] = genericAssocListFirst;
    nodeCreatorRegistry[s_generic_assoc_list][{ s_generic_assoc_list, s_comma, s_generic_association }] =
            genericAssocListAppend;
    nodeCreatorRegistry[s_generic_association][{ grammar.symbolId("<type_name>"), grammar.symbolId(":"),
            s_assignment_exp }] = genericAssociationTyped;
    nodeCreatorRegistry[s_generic_association][{ grammar.symbolId("default"), grammar.symbolId(":"),
            s_assignment_exp }] = genericAssociationDefault;

    int s_argument_exp_list = grammar.symbolId("<argument_exp_list>");
    int s_postfix_exp = grammar.symbolId("<postfix_exp>");
    nodeCreatorRegistry[s_postfix_exp][{ s_primary_exp }] = doNothing;
    nodeCreatorRegistry[s_postfix_exp][{ s_postfix_exp, s_open_bracket, s_exp, s_close_bracket }] = arrayAccess;
    nodeCreatorRegistry[s_postfix_exp][{ s_postfix_exp, s_open_paren, s_argument_exp_list, s_close_paren }] = functionCall;
    nodeCreatorRegistry[s_postfix_exp][{ s_postfix_exp, s_open_paren, s_close_paren }] = noargFunctionCall;
    nodeCreatorRegistry[s_postfix_exp][{ s_postfix_exp, grammar.symbolId("."), s_identifier }] = directMemberAccess;
    nodeCreatorRegistry[s_postfix_exp][{ s_postfix_exp, grammar.symbolId("->"), s_identifier }] = pointeeMemberAccess;
    nodeCreatorRegistry[s_postfix_exp][{ s_postfix_exp, grammar.symbolId("++") }] = postfixIncrementDecrement;
    nodeCreatorRegistry[s_postfix_exp][{ s_postfix_exp, grammar.symbolId("--") }] = postfixIncrementDecrement;

    int s_cast_exp = grammar.symbolId("<cast_exp>");
    int s_unary_exp = grammar.symbolId("<unary_exp>");
    int s_unary_operator = grammar.symbolId("<unary_operator>");
    nodeCreatorRegistry[s_unary_exp][{ s_postfix_exp }] = doNothing;
    nodeCreatorRegistry[s_unary_exp][{ grammar.symbolId("++"), s_unary_exp }] = prefixIncrementDecrement;
    nodeCreatorRegistry[s_unary_exp][{ grammar.symbolId("--"), s_unary_exp }] = prefixIncrementDecrement;
    nodeCreatorRegistry[s_unary_exp][{ s_unary_operator, s_cast_exp }] = unaryExpression;
    nodeCreatorRegistry[s_unary_exp][{ grammar.symbolId("sizeof"), s_unary_exp }] = sizeofExpression;
    nodeCreatorRegistry[s_unary_exp][{ grammar.symbolId("sizeof"), s_open_paren, grammar.symbolId("<type_name>"), s_close_paren }] = sizeofTypeExpression;

    nodeCreatorRegistry[s_cast_exp][{ s_unary_exp }] = doNothing;
    nodeCreatorRegistry[s_cast_exp][{ s_open_paren, grammar.symbolId("<type_name>"), s_close_paren, s_cast_exp }] = typeCast;

    // type_name / spec_qualifier_list for sizeof(type) and casts (casts still stubbed at cast_exp).
    int s_spec_qualifier_list = grammar.symbolId("<spec_qualifier_list>");
    int s_type_name = grammar.symbolId("<type_name>");
    int s_abstract_declarator_sym = grammar.symbolId("<abstract_declarator>");
    // Same accumulator as decl_specs: type specs + cv (no storage class).
    nodeCreatorRegistry[s_spec_qualifier_list][{ s_type_specifier }] = declarationTypeSpecifier;
    nodeCreatorRegistry[s_spec_qualifier_list][{ s_type_specifier, s_spec_qualifier_list }] =
            addDeclarationTypeSpecifier;
    nodeCreatorRegistry[s_spec_qualifier_list][{ s_type_qualifier }] = declarationTypeQualifier;
    nodeCreatorRegistry[s_spec_qualifier_list][{ s_type_qualifier, s_spec_qualifier_list }] =
            addDeclarationTypeQualifier;
    nodeCreatorRegistry[s_type_name][{ s_spec_qualifier_list }] = specQualifierListTypeName;
    nodeCreatorRegistry[s_type_name][{ s_spec_qualifier_list, s_abstract_declarator_sym }] =
            typeNameWithAbstractDeclarator;

    int s_mult_exp = grammar.symbolId("<mult_exp>");
    nodeCreatorRegistry[s_mult_exp][{ s_cast_exp }] = doNothing;
    nodeCreatorRegistry[s_mult_exp][{ s_mult_exp, grammar.symbolId("*"), s_cast_exp }] = arithmeticExpression;
    nodeCreatorRegistry[s_mult_exp][{ s_mult_exp, grammar.symbolId("/"), s_cast_exp }] = arithmeticExpression;
    nodeCreatorRegistry[s_mult_exp][{ s_mult_exp, grammar.symbolId("%"), s_cast_exp }] = arithmeticExpression;

    int s_additive_exp = grammar.symbolId("<additive_exp>");
    nodeCreatorRegistry[s_additive_exp][{ s_mult_exp }] = doNothing;
    nodeCreatorRegistry[s_additive_exp][{ s_additive_exp, grammar.symbolId("+"), s_mult_exp }] = arithmeticExpression;
    nodeCreatorRegistry[s_additive_exp][{ s_additive_exp, grammar.symbolId("-"), s_mult_exp }] = arithmeticExpression;

    int s_shift_exp = grammar.symbolId("<shift_expression>");
    nodeCreatorRegistry[s_shift_exp][{ s_additive_exp }] = doNothing;
    nodeCreatorRegistry[s_shift_exp][{ s_shift_exp, grammar.symbolId("<<"), s_additive_exp }] = shiftExpression;
    nodeCreatorRegistry[s_shift_exp][{ s_shift_exp, grammar.symbolId(">>"), s_additive_exp }] = shiftExpression;

    int s_relational_exp = grammar.symbolId("<relational_exp>");
    nodeCreatorRegistry[s_relational_exp][{ s_shift_exp }] = doNothing;
    nodeCreatorRegistry[s_relational_exp][{ s_relational_exp, grammar.symbolId("<"), s_shift_exp }] = relationalExpression;
    nodeCreatorRegistry[s_relational_exp][{ s_relational_exp, grammar.symbolId(">"), s_shift_exp }] = relationalExpression;
    nodeCreatorRegistry[s_relational_exp][{ s_relational_exp, grammar.symbolId("<="), s_shift_exp }] = relationalExpression;
    nodeCreatorRegistry[s_relational_exp][{ s_relational_exp, grammar.symbolId(">="), s_shift_exp }] = relationalExpression;

    int s_equality_exp = grammar.symbolId("<equality_exp>");
    nodeCreatorRegistry[s_equality_exp][{ s_relational_exp }] = doNothing;
    nodeCreatorRegistry[s_equality_exp][{ s_equality_exp, grammar.symbolId("=="), s_relational_exp }] = relationalExpression;
    nodeCreatorRegistry[s_equality_exp][{ s_equality_exp, grammar.symbolId("!="), s_relational_exp }] = relationalExpression;

    int s_and_exp = grammar.symbolId("<and_exp>");
    nodeCreatorRegistry[s_and_exp][{ s_equality_exp }] = doNothing;
    nodeCreatorRegistry[s_and_exp][{ s_and_exp, grammar.symbolId("&"), s_equality_exp }] = bitwiseExpression;

    int s_exclusive_or_exp = grammar.symbolId("<exclusive_or_exp>");
    nodeCreatorRegistry[s_exclusive_or_exp][{ s_and_exp }] = doNothing;
    nodeCreatorRegistry[s_exclusive_or_exp][{ s_exclusive_or_exp, grammar.symbolId("^"), s_and_exp }] = bitwiseExpression;

    int s_inclusive_or_exp = grammar.symbolId("<inclusive_or_exp>");
    nodeCreatorRegistry[s_inclusive_or_exp][{ s_exclusive_or_exp }] = doNothing;
    nodeCreatorRegistry[s_inclusive_or_exp][{ s_inclusive_or_exp, grammar.symbolId("|"), s_exclusive_or_exp }] = bitwiseExpression;

    int s_logical_and_exp = grammar.symbolId("<logical_and_exp>");
    nodeCreatorRegistry[s_logical_and_exp][{ s_inclusive_or_exp }] = doNothing;
    nodeCreatorRegistry[s_logical_and_exp][{ s_logical_and_exp, grammar.symbolId("&&"), s_inclusive_or_exp }] = logicalAndExpression;

    int s_logical_or_exp = grammar.symbolId("<logical_or_exp>");
    nodeCreatorRegistry[s_logical_or_exp][{ s_logical_and_exp }] = doNothing;
    nodeCreatorRegistry[s_logical_or_exp][{ s_logical_or_exp, grammar.symbolId("||"), s_logical_and_exp }] = logicalOrExpression;

    int s_conditional_exp = grammar.symbolId("<conditional_exp>");
    nodeCreatorRegistry[s_conditional_exp][{ s_logical_or_exp }] = doNothing;
    nodeCreatorRegistry[s_conditional_exp][{ s_logical_or_exp, grammar.symbolId("?"), s_exp, grammar.symbolId(":"), s_conditional_exp }] = conditionalExpression;

    // Identity: const_exp is a conditional_exp (array bounds, enum values, case labels, bit-fields).
    int s_const_exp = grammar.symbolId("<const_exp>");
    nodeCreatorRegistry[s_const_exp][{ s_conditional_exp }] = doNothing;

    int s_assignment = grammar.symbolId("<assignment_exp>");
    int s_assignment_operator = grammar.symbolId("<assignment_operator>");
    nodeCreatorRegistry[s_assignment][{ s_conditional_exp }] = doNothing;
    nodeCreatorRegistry[s_assignment][{ s_unary_exp, s_assignment_operator, s_assignment }] = assignmentExpression;

    nodeCreatorRegistry[s_type_specifier][{ grammar.symbolId("typeof"), s_open_paren,
            grammar.symbolId("<type_name>"), s_close_paren }] = typeofTypeName;
    nodeCreatorRegistry[s_type_specifier][{ grammar.symbolId("typeof"), s_open_paren, s_assignment,
            s_close_paren }] = typeofExpression;

    int s_initializer = grammar.symbolId("<initializer>");
    int s_open_brace = grammar.symbolId("{");
    int s_close_brace = grammar.symbolId("}");
    int s_initializer_list = grammar.symbolId("<initializer_list>");
    nodeCreatorRegistry[s_initializer][{ s_assignment }] = doNothing;
    nodeCreatorRegistry[s_initializer][{ s_open_brace, s_initializer_list, s_close_brace }] = braceInitializer;
    nodeCreatorRegistry[s_initializer][{ s_open_brace, s_initializer_list, s_comma, s_close_brace }] =
            braceInitializerTrailingComma;

    int s_designator = grammar.symbolId("<designator>");
    int s_designator_list = grammar.symbolId("<designator_list>");
    int s_designation = grammar.symbolId("<designation>");
    nodeCreatorRegistry[s_designator][{ grammar.symbolId("."), grammar.symbolId("id") }] = memberDesignator;
    nodeCreatorRegistry[s_designator][{ grammar.symbolId("["), s_const_exp, grammar.symbolId("]") }] = arrayDesignator;
    nodeCreatorRegistry[s_designator_list][{ s_designator }] = designatorListSingle;
    nodeCreatorRegistry[s_designator_list][{ s_designator_list, s_designator }] = designatorListAppend;
    nodeCreatorRegistry[s_designation][{ s_designator_list, grammar.symbolId("=") }] = designation;

    nodeCreatorRegistry[s_initializer_list][{ s_initializer }] = initializerListFirst;
    nodeCreatorRegistry[s_initializer_list][{ s_designation, s_initializer }] = designatedInitializerListFirst;
    nodeCreatorRegistry[s_initializer_list][{ s_initializer_list, s_comma, s_initializer }] = initializerListAppend;
    nodeCreatorRegistry[s_initializer_list][{ s_initializer_list, s_comma, s_designation, s_initializer }] =
            designatedInitializerListAppend;

    int s_init_declarator = grammar.symbolId("<init_declarator>");
    nodeCreatorRegistry[s_init_declarator][{ s_declarator }] = initializedDeclarator;
    nodeCreatorRegistry[s_init_declarator][{ s_declarator, grammar.symbolId("="), s_initializer }] = initializedDeclaratorWithInitializer;

    int s_init_declarator_list = grammar.symbolId("<init_declarator_list>");
    nodeCreatorRegistry[s_init_declarator_list][{ s_init_declarator }] = initializedDeclaratorList;
    nodeCreatorRegistry[s_init_declarator_list][{ s_init_declarator_list, s_comma, s_init_declarator }] = addToInitializedDeclaratorList;

    int s_decl = grammar.symbolId("<decl>");
    int s_semicolon = grammar.symbolId(";");
    nodeCreatorRegistry[s_decl][{ s_decl_specs, s_init_declarator_list, s_semicolon }] = initializedDeclaration;
    nodeCreatorRegistry[s_decl][{ s_decl_specs, s_semicolon }] = declaration;

    int s_decl_list = grammar.symbolId("<decl_list>");
    nodeCreatorRegistry[s_decl_list][{ s_decl }] = declarationList;
    nodeCreatorRegistry[s_decl_list][{ s_decl_list, s_decl }] = addDeclarationToList;

    nodeCreatorRegistry[s_exp][{ s_assignment }] = doNothing;
    nodeCreatorRegistry[s_exp][{ s_exp, s_comma, s_assignment }] = expressionList;

    nodeCreatorRegistry[s_unary_operator][{ grammar.symbolId("&") }] = doNothing;
    nodeCreatorRegistry[s_unary_operator][{ grammar.symbolId("*") }] = doNothing;
    nodeCreatorRegistry[s_unary_operator][{ grammar.symbolId("+") }] = doNothing;
    nodeCreatorRegistry[s_unary_operator][{ grammar.symbolId("-") }] = doNothing;
    nodeCreatorRegistry[s_unary_operator][{ grammar.symbolId("~") }] = doNothing;
    nodeCreatorRegistry[s_unary_operator][{ grammar.symbolId("!") }] = doNothing;

    nodeCreatorRegistry[s_assignment_operator][{ grammar.symbolId("=") }] = doNothing;
    nodeCreatorRegistry[s_assignment_operator][{ grammar.symbolId("*=") }] = doNothing;
    nodeCreatorRegistry[s_assignment_operator][{ grammar.symbolId("/=") }] = doNothing;
    nodeCreatorRegistry[s_assignment_operator][{ grammar.symbolId("%=") }] = doNothing;
    nodeCreatorRegistry[s_assignment_operator][{ grammar.symbolId("+=") }] = doNothing;
    nodeCreatorRegistry[s_assignment_operator][{ grammar.symbolId("-=") }] = doNothing;
    nodeCreatorRegistry[s_assignment_operator][{ grammar.symbolId("<<=") }] = doNothing;
    nodeCreatorRegistry[s_assignment_operator][{ grammar.symbolId(">>=") }] = doNothing;
    nodeCreatorRegistry[s_assignment_operator][{ grammar.symbolId("&=") }] = doNothing;
    nodeCreatorRegistry[s_assignment_operator][{ grammar.symbolId("^=") }] = doNothing;
    nodeCreatorRegistry[s_assignment_operator][{ grammar.symbolId("|=") }] = doNothing;

    int s_exp_stat = grammar.symbolId("<exp_stat>");
    nodeCreatorRegistry[s_exp_stat][{ s_exp, s_semicolon }] = expressionStatement;
    nodeCreatorRegistry[s_exp_stat][{ s_semicolon }] = emptyStatement;

    int s_matched = grammar.symbolId("<matched>");
    int s_unmatched = grammar.symbolId("<unmatched>");
    int s_stat = grammar.symbolId("<stat>");
    int s_compound_stat = grammar.symbolId("<compound_stat>");
    int s_jump_stat = grammar.symbolId("<jump_stat>");
    int s_if = grammar.symbolId("if");
    int s_colon = grammar.symbolId(":");
    int s_labeled_stat_matched = grammar.symbolId("<labeled_stat_matched>");
    int s_labeled_stat_unmatched = grammar.symbolId("<labeled_stat_unmatched>");
    nodeCreatorRegistry[s_matched][{s_if, s_open_paren, s_exp, s_close_paren, s_matched, grammar.symbolId("else"), s_matched }] = ifElseStatement;
    nodeCreatorRegistry[s_unmatched][{s_if, s_open_paren, s_exp, s_close_paren, s_stat }] = ifStatement;
    nodeCreatorRegistry[s_unmatched][{s_if, s_open_paren, s_exp, s_close_paren, s_matched, grammar.symbolId("else"), s_unmatched }] = ifElseStatement;
    int s_switch = grammar.symbolId("switch");
    nodeCreatorRegistry[s_matched][{ s_switch, s_open_paren, s_exp, s_close_paren, s_matched }] = switchStatement;
    nodeCreatorRegistry[s_unmatched][{ s_switch, s_open_paren, s_exp, s_close_paren, s_unmatched }] = switchStatement;
    nodeCreatorRegistry[s_matched][{ s_labeled_stat_matched }] = doNothing;
    nodeCreatorRegistry[s_unmatched][{ s_labeled_stat_unmatched }] = doNothing;
    int s_case = grammar.symbolId("case");
    int s_default = grammar.symbolId("default");
    // s_identifier defined earlier with declarators / primary_exp.
    nodeCreatorRegistry[s_labeled_stat_matched][{ s_case, grammar.symbolId("<const_exp>"), s_colon, s_matched }] = caseLabel;
    nodeCreatorRegistry[s_labeled_stat_unmatched][{ s_case, grammar.symbolId("<const_exp>"), s_colon, s_unmatched }] = caseLabel;
    nodeCreatorRegistry[s_labeled_stat_matched][{ s_default, s_colon, s_matched }] = defaultLabel;
    nodeCreatorRegistry[s_labeled_stat_unmatched][{ s_default, s_colon, s_unmatched }] = defaultLabel;
    nodeCreatorRegistry[s_labeled_stat_matched][{ s_identifier, s_colon, s_matched }] = namedLabel;
    nodeCreatorRegistry[s_labeled_stat_unmatched][{ s_identifier, s_colon, s_unmatched }] = namedLabel;
    nodeCreatorRegistry[s_matched][{ s_exp_stat }] = doNothing;
    nodeCreatorRegistry[s_matched][{ s_compound_stat }] = doNothing;
    nodeCreatorRegistry[s_matched][{ s_jump_stat }] = doNothing;

    nodeCreatorRegistry[s_stat][{ s_matched }] = doNothing;
    nodeCreatorRegistry[s_stat][{ s_unmatched }] = doNothing;

    int s_stat_list = grammar.symbolId("<stat_list>");
    nodeCreatorRegistry[s_stat_list][{ s_stat }] = statementList;
    nodeCreatorRegistry[s_stat_list][{ s_stat_list, s_stat }] = addToStatementList;

    int s_return = grammar.symbolId("return");
    nodeCreatorRegistry[s_jump_stat][{ grammar.symbolId("goto"), s_identifier, s_semicolon }] = gotoStatement;
    nodeCreatorRegistry[s_jump_stat][{ grammar.symbolId("continue"), s_semicolon }] = loopJumpStatement;
    nodeCreatorRegistry[s_jump_stat][{ grammar.symbolId("break"), s_semicolon }] = loopJumpStatement;
    nodeCreatorRegistry[s_jump_stat][{ s_return, s_exp, s_semicolon }] = returnExpressionStatement;
    nodeCreatorRegistry[s_jump_stat][{ s_return, s_semicolon }] = returnVoidStatement;

    nodeCreatorRegistry[s_argument_exp_list][{ s_assignment }] = createActualArgumentsList;
    nodeCreatorRegistry[s_argument_exp_list][{ s_argument_exp_list, s_comma, s_assignment }] = addToActualArgumentsList;

    int s_block_item = grammar.symbolId("<block_item>");
    int s_block_item_list = grammar.symbolId("<block_item_list>");
    int s_stat_for_block = grammar.symbolId("<stat>");
    nodeCreatorRegistry[s_block_item][{ s_decl }] = blockItemDeclaration;
    nodeCreatorRegistry[s_block_item][{ s_stat_for_block }] = doNothing;
    nodeCreatorRegistry[s_block_item_list][{ s_block_item }] = statementList;
    nodeCreatorRegistry[s_block_item_list][{ s_block_item_list, s_block_item }] = addToStatementList;
    nodeCreatorRegistry[s_compound_stat][{ s_open_brace, s_block_item_list, s_close_brace }] = blockItemListCompound;
    nodeCreatorRegistry[s_compound_stat][{ s_open_brace, s_close_brace }] = emptyCompound;

    int s_function_definition = grammar.symbolId("<function_definition>");
    nodeCreatorRegistry[s_function_definition][{ s_decl_specs, s_declarator, s_compound_stat }] = functionDefinition;
    nodeCreatorRegistry[s_function_definition][{ s_declarator, s_compound_stat }] = defaultReturnTypeFunctionDefinition;
    // K&R definitions: `int f(a) int a; { ... }` (parameter decls between declarator and body).
    nodeCreatorRegistry[s_function_definition][{ s_decl_specs, s_declarator, s_decl_list, s_compound_stat }] =
            notImplementedYet("K&R style function definitions");
    nodeCreatorRegistry[s_function_definition][{ s_declarator, s_decl_list, s_compound_stat }] =
            notImplementedYet("K&R style function definitions");

    int s_external_decl = grammar.symbolId("<external_decl>");
    nodeCreatorRegistry[s_external_decl][{ s_function_definition }] = externalFunctionDefinition;
    nodeCreatorRegistry[s_external_decl][{ s_decl }] = externalDeclaration;

    int s_translation_unit = grammar.symbolId("<translation_unit>");
    nodeCreatorRegistry[s_translation_unit][{ s_external_decl }] = translationUnit;
    nodeCreatorRegistry[s_translation_unit][{ s_translation_unit, s_external_decl }] = addToTranslationUnit;

    int s_iteration_stat_matched = grammar.symbolId("<iteration_stat_matched>");
    int s_iteration_stat_unmatched = grammar.symbolId("<iteration_stat_unmatched>");
    nodeCreatorRegistry[s_matched][{ s_iteration_stat_matched }] = doNothing;
    nodeCreatorRegistry[s_unmatched][{ s_iteration_stat_unmatched }] = doNothing;

    int s_while = grammar.symbolId("while");
    int s_do = grammar.symbolId("do");
    int s_for = grammar.symbolId("for");
    nodeCreatorRegistry[s_iteration_stat_matched][{ s_while, s_open_paren, s_exp, s_close_paren, s_matched }] = whileLoopStatement;
    nodeCreatorRegistry[s_iteration_stat_unmatched][{ s_while, s_open_paren, s_exp, s_close_paren, s_unmatched }] = whileLoopStatement;
    nodeCreatorRegistry[s_iteration_stat_matched][{ s_do, s_matched, s_while, s_open_paren, s_exp, s_close_paren, s_semicolon }] =
            doWhileLoopStatement;
    nodeCreatorRegistry[s_iteration_stat_unmatched][{ s_do, s_unmatched, s_while, s_open_paren, s_exp, s_close_paren, s_semicolon }] =
            doWhileLoopStatement;

    // for-init: none / expression / declaration. Decl form has one fewer terminal because
    // <decl> already consumes its terminating ';'.
    enum class ForInit { None, Expression, Declaration };
    auto forLoop = [](ForInit initKind, bool hasClause, bool hasIncrement) {
        return [=](AbstractSyntaxTreeBuilderContext& context) {
            const int terminalCount = (initKind == ForInit::Declaration) ? 4 : 5;
            for (int i = 0; i < terminalCount; ++i) {
                context.popTerminal(); // for ( ; ; ) or for ( ; )
            }
            auto increment = hasIncrement ? context.popExpression() : nullptr;
            auto clause = hasClause ? context.popExpression() : nullptr;
            std::unique_ptr<AbstractSyntaxTreeNode> initialization;
            bool declarationScoped = false;
            if (initKind == ForInit::Expression) {
                initialization = context.popExpression();
            } else if (initKind == ForInit::Declaration) {
                initialization = context.popDeclaration();
                declarationScoped = true;
            }
            auto loopHeader = std::make_unique<ForLoopHeader>(
                    std::move(initialization), std::move(clause), std::move(increment), declarationScoped);
            auto body = context.popStatement();
            context.pushStatement(std::make_unique<LoopStatement>(std::move(loopHeader), std::move(body)));
        };
    };
    auto registerFor = [&](const std::vector<int>& prod, std::function<void(AbstractSyntaxTreeBuilderContext&)> creator) {
        nodeCreatorRegistry[s_iteration_stat_matched][prod] = creator;
        auto unmatchedProd = prod;
        unmatchedProd.back() = s_unmatched;
        nodeCreatorRegistry[s_iteration_stat_unmatched][unmatchedProd] = creator;
    };
    registerFor({ s_for, s_open_paren, s_exp, s_semicolon, s_exp, s_semicolon, s_exp, s_close_paren, s_matched }, forLoop(ForInit::Expression, true,  true));
    registerFor({ s_for, s_open_paren, s_exp, s_semicolon, s_exp, s_semicolon, s_close_paren, s_matched }, forLoop(ForInit::Expression, true,  false));
    registerFor({ s_for, s_open_paren, s_exp, s_semicolon, s_semicolon, s_exp, s_close_paren, s_matched }, forLoop(ForInit::Expression, false, true));
    registerFor({ s_for, s_open_paren, s_exp, s_semicolon, s_semicolon, s_close_paren, s_matched }, forLoop(ForInit::Expression, false, false));
    registerFor({ s_for, s_open_paren, s_semicolon, s_exp, s_semicolon, s_exp, s_close_paren, s_matched }, forLoop(ForInit::None, true,  true));
    registerFor({ s_for, s_open_paren, s_semicolon, s_exp, s_semicolon, s_close_paren, s_matched }, forLoop(ForInit::None, true,  false));
    registerFor({ s_for, s_open_paren, s_semicolon, s_semicolon, s_exp, s_close_paren, s_matched }, forLoop(ForInit::None, false, true));
    registerFor({ s_for, s_open_paren, s_semicolon, s_semicolon, s_close_paren, s_matched }, forLoop(ForInit::None, false, false));

    int s_decl_for = grammar.symbolId("<decl>");
    registerFor({ s_for, s_open_paren, s_decl_for, s_exp, s_semicolon, s_exp, s_close_paren, s_matched }, forLoop(ForInit::Declaration, true, true));
    registerFor({ s_for, s_open_paren, s_decl_for, s_exp, s_semicolon, s_close_paren, s_matched }, forLoop(ForInit::Declaration, true, false));
    registerFor({ s_for, s_open_paren, s_decl_for, s_semicolon, s_exp, s_close_paren, s_matched }, forLoop(ForInit::Declaration, false, true));
    registerFor({ s_for, s_open_paren, s_decl_for, s_semicolon, s_close_paren, s_matched }, forLoop(ForInit::Declaration, false, false));

    // --- enum ---
    int s_enum_spec = grammar.symbolId("<enum_spec>");
    int s_enumerator_list = grammar.symbolId("<enumerator_list>");
    int s_enumerator = grammar.symbolId("<enumerator>");
    int s_enum_kw = grammar.symbolId("enum");
    int s_id_for_enum = grammar.symbolId("id");
    int s_enum_const_exp = grammar.symbolId("<const_exp>");

    nodeCreatorRegistry[s_enumerator][{ s_id_for_enum }] = [](AbstractSyntaxTreeBuilderContext& context) {
        auto id = context.popTerminal();
        context.environment().addEnumerator(id.value);
    };
    nodeCreatorRegistry[s_enumerator][{ s_id_for_enum, grammar.symbolId("="), s_enum_const_exp }] =
            [](AbstractSyntaxTreeBuilderContext& context) {
                auto expr = context.popExpression();
                context.popTerminal(); // =
                auto id = context.popTerminal();
                long value = 0;
                if (!expr->evaluateConstant(value)) {
                    throw std::runtime_error {
                            "enumerator initializer is not a constant expression: " + id.value };
                }
                context.environment().addEnumerator(id.value, value);
            };
    nodeCreatorRegistry[s_enumerator_list][{ s_enumerator }] = doNothing;
    nodeCreatorRegistry[s_enumerator_list][{ s_enumerator_list, s_comma, s_enumerator }] =
            [](AbstractSyntaxTreeBuilderContext& context) { context.popTerminal(); };
    // C99 trailing comma after last enumerator.
    nodeCreatorRegistry[s_enumerator_list][{ s_enumerator_list, s_comma }] =
            [](AbstractSyntaxTreeBuilderContext& context) { context.popTerminal(); };

    // Enum types are product signed-int stand-ins (no first-class enum tag table).
    // Enumerator values live on LexicalSession / AST snapshot only.
    nodeCreatorRegistry[s_enum_spec][{ s_enum_kw, s_id_for_enum, s_open_brace, s_enumerator_list, s_close_brace }] =
            [](AbstractSyntaxTreeBuilderContext& context) {
                context.popTerminal(); // }
                context.popTerminal(); // {
                auto tag = context.popTerminal();
                context.popTerminal(); // enum
                context.environment().endEnumDefinition(); // values already on session
                context.pushTypeSpecifier(TypeSpecifier { type::signedInteger(), tag.value });
            };
    nodeCreatorRegistry[s_enum_spec][{ s_enum_kw, s_open_brace, s_enumerator_list, s_close_brace }] =
            [](AbstractSyntaxTreeBuilderContext& context) {
                context.popTerminal(); // }
                context.popTerminal(); // {
                context.popTerminal(); // enum
                context.environment().endEnumDefinition();
                context.pushTypeSpecifier(TypeSpecifier { type::signedInteger(), "" });
            };
    nodeCreatorRegistry[s_enum_spec][{ s_enum_kw, s_id_for_enum }] =
            [](AbstractSyntaxTreeBuilderContext& context) {
                auto tag = context.popTerminal();
                context.popTerminal(); // enum
                context.pushTypeSpecifier(TypeSpecifier { type::signedInteger(), tag.value });
            };

    // --- struct / union ---
    int s_struct_or_union = grammar.symbolId("<struct_or_union>");
    int s_struct_or_union_spec = grammar.symbolId("<struct_or_union_spec>");
    int s_struct_decl_list = grammar.symbolId("<struct_decl_list>");
    int s_struct_decl = grammar.symbolId("<struct_decl>");
    int s_struct_declarator_list = grammar.symbolId("<struct_declarator_list>");
    int s_struct_declarator = grammar.symbolId("<struct_declarator>");
    // s_open_brace / s_close_brace already defined above for compound statements.

    nodeCreatorRegistry[s_struct_or_union][{ grammar.symbolId("struct") }] = [](AbstractSyntaxTreeBuilderContext& context) {
        context.popTerminal();
        context.pushIsUnion(false);
        context.newStructMemberList();
    };
    nodeCreatorRegistry[s_struct_or_union][{ grammar.symbolId("union") }] = [](AbstractSyntaxTreeBuilderContext& context) {
        context.popTerminal();
        context.pushIsUnion(true);
        context.newStructMemberList();
    };

    nodeCreatorRegistry[s_struct_or_union_spec][{ s_struct_or_union, s_identifier, s_open_brace, s_struct_decl_list, s_close_brace }] =
            [](AbstractSyntaxTreeBuilderContext& context) {
                context.popTerminal(); // }
                context.popTerminal(); // {
                auto tag = context.popTerminal();
                auto members = context.popStructMemberList();
                bool isUnion = context.popIsUnion();
                // Shared incomplete tag so self-referential members keep one layout identity.
                type::Type tagType = context.environment().ensureStructTag(tag.value);
                if (isUnion) {
                    type::completeUnion(tagType, std::move(members));
                } else {
                    type::completeStructure(tagType, std::move(members));
                }
                // Shared body: tagType already sees completion via structureBodyIdentity().
                context.pushTypeSpecifier(TypeSpecifier { tagType, tag.value });
            };
    nodeCreatorRegistry[s_struct_or_union_spec][{ s_struct_or_union, s_open_brace, s_struct_decl_list, s_close_brace }] =
            [](AbstractSyntaxTreeBuilderContext& context) {
                context.popTerminal(); // }
                context.popTerminal(); // {
                auto members = context.popStructMemberList();
                bool isUnion = context.popIsUnion();
                type::Type completed = type::incompleteRecord();
                if (isUnion) {
                    type::completeUnion(completed, std::move(members));
                } else {
                    type::completeStructure(completed, std::move(members));
                }
                context.pushTypeSpecifier(TypeSpecifier { completed, "" });
            };
    nodeCreatorRegistry[s_struct_or_union_spec][{ s_struct_or_union, s_identifier }] =
            [](AbstractSyntaxTreeBuilderContext& context) {
                auto tag = context.popTerminal();
                context.popIsUnion(); // layout decided at definition
                context.popStructMemberList(); // no body
                context.pushTypeSpecifier(TypeSpecifier {
                        context.environment().ensureStructTag(tag.value), tag.value });
            };

    nodeCreatorRegistry[s_struct_declarator][{ s_declarator }] = [](AbstractSyntaxTreeBuilderContext& context) {
        context.addStructDeclarator(context.popDeclarator(), -1);
    };
    nodeCreatorRegistry[s_struct_declarator][{ s_declarator, grammar.symbolId(":"), grammar.symbolId("<const_exp>") }] =
            [](AbstractSyntaxTreeBuilderContext& context) {
                const int width = foldBitFieldWidth(context);
                context.addStructDeclarator(context.popDeclarator(), width);
            };
    nodeCreatorRegistry[s_struct_declarator][{ grammar.symbolId(":"), grammar.symbolId("<const_exp>") }] =
            [](AbstractSyntaxTreeBuilderContext& context) {
                const int width = foldBitFieldWidth(context);
                context.addStructDeclarator(nullptr, width);
            };
    nodeCreatorRegistry[s_struct_declarator_list][{ s_struct_declarator }] = doNothing;
    nodeCreatorRegistry[s_struct_declarator_list][{ s_struct_declarator_list, s_comma, s_struct_declarator }] =
            [](AbstractSyntaxTreeBuilderContext& context) { context.popTerminal(); };

    nodeCreatorRegistry[s_struct_decl][{ s_spec_qualifier_list, s_struct_declarator_list, s_semicolon }] =
            [](AbstractSyntaxTreeBuilderContext& context) {
                context.popTerminal(); // ;
                auto declarators = context.popStructDeclarators();
                auto baseType = popResolvedSpecQualifiers(context).getResolvedType();
                for (auto& [declarator, bitWidth] : declarators) {
                    if (!declarator) {
                        context.addStructMember("", baseType, bitWidth);
                    } else {
                        context.addStructMember(declarator->getName(),
                                declarator->getFundamentalType(baseType), bitWidth);
                    }
                }
            };
    // C11 anonymous struct/union: untagged complete record only (empty stored name).
    // Tagged type-only forms (struct T { ... };) must not become empty-name members.
    nodeCreatorRegistry[s_struct_decl][{ s_spec_qualifier_list, s_semicolon }] =
            [](AbstractSyntaxTreeBuilderContext& context) {
                context.popTerminal(); // ;
                auto specs = popResolvedSpecQualifiers(context);
                if (specs.isUntaggedCompleteRecord()) {
                    context.addStructMember("", specs.getResolvedType());
                }
            };
    nodeCreatorRegistry[s_struct_decl_list][{ s_struct_decl }] = doNothing;
    nodeCreatorRegistry[s_struct_decl_list][{ s_struct_decl_list, s_struct_decl }] = doNothing;
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

void ContextualSyntaxNodeBuilder::loopJumpStatement(AbstractSyntaxTreeBuilderContext& context) {
    context.popTerminal(); // ;
    context.pushStatement(std::make_unique<JumpStatement>(context.popTerminal())); // break | continue
}

} // namespace ast
