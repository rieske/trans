#include "CSNB_Internal.h"

#include <limits>
#include <stdexcept>
#include <vector>

namespace ast {
namespace {

int foldBitFieldWidth(AbstractSyntaxTreeBuilderContext& context) {
    auto widthExpr = context.popExpression();
    context.popTerminal();
    long width = 0;
    if (!widthExpr || !widthExpr->foldToHostLong(width) || width < 0
            || width > static_cast<long>(std::numeric_limits<int>::max())) {
        const translation_unit::Context where = widthExpr
                ? widthExpr->getContext() : translation_unit::Context { "", 0 };
        context.error(where, "bit-field width is not a constant");
        return 0;
    }
    return static_cast<int>(width);
}

void completeRecordFromSpec(AbstractSyntaxTreeBuilderContext& context, type::Type& record,
        std::vector<type::MemberSpec> members, bool isUnion,
        const translation_unit::Context& where) {
    const bool packed = context.environment().session().recordPacked.consume();
    try {
        if (isUnion) {
            type::completeUnion(record, std::move(members), packed);
        } else {
            type::completeStructure(record, std::move(members), packed);
        }
    } catch (const std::invalid_argument& error) {
        context.error(where, error.what());
    }
}

} // namespace

ContextualSyntaxNodeBuilder::ContextualSyntaxNodeBuilder(const parser::Grammar& grammar) {
    this->grammar = &grammar;
    creators_.assign(grammar.ruleCount(), {});

    int s_type_specifier = grammar.symbolId("<type_spec>");
    bind(s_type_specifier, { grammar.symbolId("short") }, shortType);
    bind(s_type_specifier, { grammar.symbolId("int") }, integerType);
    bind(s_type_specifier, { grammar.symbolId("long") }, longType);
    bind(s_type_specifier, { grammar.symbolId("char") }, characterType);
    bind(s_type_specifier, { grammar.symbolId("void") }, voidType);
    bind(s_type_specifier, { grammar.symbolId("float") }, floatType);
    bind(s_type_specifier, { grammar.symbolId("double") }, doubleType);
    bind(s_type_specifier, { grammar.symbolId("signed") }, signedType);
    bind(s_type_specifier, { grammar.symbolId("unsigned") }, unsignedType);
    bind(s_type_specifier, { grammar.symbolId("bool") }, boolType);
    bind(s_type_specifier, { grammar.symbolId("_Complex") }, complexType);
    bind(s_type_specifier, { grammar.symbolId("typedef_name") }, typedefName);
    bind(s_type_specifier, { grammar.symbolId("<struct_or_union_spec>") }, structOrUnionType);
    bind(s_type_specifier, { grammar.symbolId("<enum_spec>") }, enumType);

    int s_type_qualifier = grammar.symbolId("<type_qualifier>");
    bind(s_type_qualifier, { grammar.symbolId("const") }, constQualifier);
    bind(s_type_qualifier, { grammar.symbolId("volatile") }, volatileQualifier);
    bind(s_type_qualifier, { grammar.symbolId("restrict") }, restrictQualifier);

    int s_type_qualifier_list = grammar.symbolId("<type_qualifier_list>");
    bind(s_type_qualifier_list, { s_type_qualifier }, typeQualifierList);
    bind(s_type_qualifier_list, { s_type_qualifier_list, s_type_qualifier }, addTypeQualifierToList);

    int s_storage_class_spec = grammar.symbolId("<storage_class_spec>");
    bind(s_storage_class_spec, { grammar.symbolId("auto") }, autoStorageClass);
    bind(s_storage_class_spec, { grammar.symbolId("register") }, registerStorageClass);
    bind(s_storage_class_spec, { grammar.symbolId("static") }, staticStorageClass);
    bind(s_storage_class_spec, { grammar.symbolId("extern") }, externStorageClass);
    bind(s_storage_class_spec, { grammar.symbolId("typedef") }, typedefStorageClass);

    int s_function_spec = grammar.symbolId("<function_spec>");
    bind(s_function_spec, { grammar.symbolId("inline") }, functionSpecifier);
    bind(s_function_spec, { grammar.symbolId("noreturn") }, functionSpecifier);

    int s_decl_specs = grammar.symbolId("<decl_specs>");
    bind(s_decl_specs, { s_type_specifier }, declarationTypeSpecifier);
    bind(s_decl_specs, { s_type_specifier, s_decl_specs }, addDeclarationTypeSpecifier);
    bind(s_decl_specs, { s_storage_class_spec }, declarationStorageClassSpecifier);
    bind(s_decl_specs, { s_storage_class_spec, s_decl_specs }, addDeclarationStorageClassSpecifier);
    bind(s_decl_specs, { s_type_qualifier }, declarationTypeQualifier);
    bind(s_decl_specs, { s_type_qualifier, s_decl_specs }, addDeclarationTypeQualifier);
    bind(s_decl_specs, { s_function_spec }, functionSpecifierOnly);
    bind(s_decl_specs, { s_function_spec, s_decl_specs }, doNothing);

    int s_direct_declarator = grammar.symbolId("<direct_declarator>");
    int s_declarator = grammar.symbolId("<declarator>");
    int s_param_type_list = grammar.symbolId("<param_type_list>");
    int s_identifier = grammar.symbolId("id");
    int s_open_paren = grammar.symbolId("(");
    int s_close_paren = grammar.symbolId(")");
    int s_open_bracket = grammar.symbolId("[");
    int s_close_bracket = grammar.symbolId("]");

    bind(s_direct_declarator, { s_identifier }, identifierDeclarator);
    bind(s_direct_declarator, { s_open_paren, s_declarator, s_close_paren }, parenthesizedDeclarator);
    bind(s_direct_declarator, { s_direct_declarator, s_open_bracket, grammar.symbolId("<const_exp>"), s_close_bracket }, arrayDeclarator);
    bind(s_direct_declarator, { s_direct_declarator, s_open_bracket, s_close_bracket }, abstractArrayDeclarator);
    bind(s_direct_declarator, { s_direct_declarator, s_open_bracket, s_type_qualifier_list, s_close_bracket }, abstractArrayDeclaratorQualified);
    bind(s_direct_declarator, { s_direct_declarator, s_open_bracket, s_type_qualifier_list, grammar.symbolId("<const_exp>"), s_close_bracket }, arrayDeclaratorQualified);
    bind(s_direct_declarator, { s_direct_declarator, s_open_paren, s_param_type_list, s_close_paren }, functionDeclarator);
    bind(s_direct_declarator, { s_direct_declarator, s_open_paren, s_close_paren }, noargFunctionDeclarator);

    int s_pointer = grammar.symbolId("<pointer>" );
    bind(s_pointer, { grammar.symbolId("*"), s_type_qualifier_list }, qualifiedPointer);
    bind(s_pointer, { grammar.symbolId("*") }, pointer);
    bind(s_pointer, { grammar.symbolId("*"), s_type_qualifier_list, s_pointer }, qualifiedPointerToPointer);
    bind(s_pointer, { grammar.symbolId("*"), s_pointer }, pointerToPointer);
    bind(s_declarator, { s_pointer, s_direct_declarator }, pointerToDeclarator);
    bind(s_declarator, { s_direct_declarator }, declarator);

    int s_param_decl = grammar.symbolId("<param_decl>");
    int s_abstract_declarator = grammar.symbolId("<abstract_declarator>");
    bind(s_param_decl, { s_decl_specs, s_declarator }, parameterDeclaration);
    bind(s_param_decl, { s_decl_specs, s_abstract_declarator }, abstractParameterDeclaration);
    bind(s_param_decl, { s_decl_specs }, parameterBaseTypeDeclaration);

    // Abstract declarators share named-declarator creators once a DirectDeclarator exists.
    // Bare `[N]` / `[]` / `(params)` / `()` inject an anonymous Identifier first.
    bind(s_abstract_declarator, { s_pointer }, abstractPointerDeclarator);
    int s_direct_abstract_declarator = grammar.symbolId("<direct_abstract_declarator>");
    bind(s_abstract_declarator, { s_direct_abstract_declarator }, declarator);
    bind(s_abstract_declarator, { s_pointer, s_direct_abstract_declarator }, pointerToDeclarator);

    bind(s_direct_abstract_declarator, { s_open_paren, s_abstract_declarator, s_close_paren }, parenthesizedDeclarator);
    bind(s_direct_abstract_declarator, { s_direct_abstract_declarator, s_open_bracket, grammar.symbolId("<const_exp>"), s_close_bracket }, arrayDeclarator);
    bind(s_direct_abstract_declarator, { s_open_bracket, grammar.symbolId("<const_exp>"), s_close_bracket }, abstractArrayOnlySized);
    bind(s_direct_abstract_declarator, { s_direct_abstract_declarator, s_open_bracket, s_close_bracket }, abstractArrayDeclarator);
    bind(s_direct_abstract_declarator, { s_open_bracket, s_close_bracket }, abstractArrayOnlyUnsized);
    bind(s_direct_abstract_declarator, { s_direct_abstract_declarator, s_open_bracket, s_type_qualifier_list, s_close_bracket }, abstractArrayDeclaratorQualified);
    bind(s_direct_abstract_declarator, { s_open_bracket, s_type_qualifier_list, s_close_bracket }, abstractArrayOnlyQualifiedUnsized);
    bind(s_direct_abstract_declarator, { s_direct_abstract_declarator, s_open_bracket, s_type_qualifier_list, grammar.symbolId("<const_exp>"), s_close_bracket }, arrayDeclaratorQualified);
    bind(s_direct_abstract_declarator, { s_open_bracket, s_type_qualifier_list, grammar.symbolId("<const_exp>"), s_close_bracket }, abstractArrayOnlyQualifiedSized);
    bind(s_direct_abstract_declarator, { s_direct_abstract_declarator, s_open_paren, s_param_type_list, s_close_paren }, functionDeclarator);
    bind(s_direct_abstract_declarator, { s_open_paren, s_param_type_list, s_close_paren }, abstractFuncOnly);
    bind(s_direct_abstract_declarator, { s_direct_abstract_declarator, s_open_paren, s_close_paren }, noargFunctionDeclarator);
    bind(s_direct_abstract_declarator, { s_open_paren, s_close_paren }, abstractNoargOnly);

    int s_param_list = grammar.symbolId("<param_list>");
    int s_comma = grammar.symbolId(",");
    bind(s_param_list, { s_param_decl }, formalArguments);
    bind(s_param_list, { s_param_list, s_comma, s_param_decl }, addFormalArgument);

    bind(s_param_type_list, { s_param_list }, formalArgumentsDeclaration);
    bind(s_param_type_list, { s_param_list, s_comma, grammar.symbolId("...") }, formalArgumentsWithVararg);

    // K&R identifier parameter list: `f(a, b)` — not the modern `f(int a, int b)`.
    int s_id_list = grammar.symbolId("<id_list>");
    bind(s_id_list, { s_identifier }, notImplementedYet("K&R identifier parameter lists"));
    bind(s_id_list, { s_id_list, s_comma, s_identifier }, notImplementedYet("K&R identifier parameter lists"));
    bind(s_direct_declarator, { s_direct_declarator, s_open_paren, s_id_list, s_close_paren }, notImplementedYet("K&R identifier parameter lists"));

    int s_constant = grammar.symbolId("<const>");
    bind(s_constant, { grammar.symbolId("int_const") }, integerConstant);
    bind(s_constant, { grammar.symbolId("char_const") }, characterConstant);
    bind(s_constant, { grammar.symbolId("float_const") }, floatConstant);
    bind(s_constant, { grammar.symbolId("enumeration_const") }, enumerationConstant);

    int s_exp = grammar.symbolId("<exp>");
    int s_primary_exp = grammar.symbolId("<primary_exp>");
    bind(s_primary_exp, { s_identifier }, identifierExpression);
    bind(s_primary_exp, { s_constant }, constantExpression);
    bind(s_primary_exp, { grammar.symbolId("string") }, stringLiteralExpression);
    bind(s_primary_exp, { s_open_paren, s_exp, s_close_paren }, parenthesizedExpression);
    bind(s_primary_exp, { grammar.symbolId("nullptr") }, nullptrExpression);
    bind(s_primary_exp, { grammar.symbolId("true") }, trueExpression);
    bind(s_primary_exp, { grammar.symbolId("false") }, falseExpression);
    int s_generic_assoc_list = grammar.symbolId("<generic_assoc_list>");
    int s_generic_association = grammar.symbolId("<generic_association>");
    int s_assignment_exp = grammar.symbolId("<assignment_exp>");
    bind(s_primary_exp, { grammar.symbolId("_Generic"), s_open_paren, s_assignment_exp, s_comma, s_generic_assoc_list, s_close_paren }, genericSelection);
    bind(s_generic_assoc_list, { s_generic_association }, genericAssocListFirst);
    bind(s_generic_assoc_list, { s_generic_assoc_list, s_comma, s_generic_association }, genericAssocListAppend);
    bind(s_generic_association, { grammar.symbolId("<type_name>"), grammar.symbolId(":"), s_assignment_exp }, genericAssociationTyped);
    bind(s_generic_association, { grammar.symbolId("default"), grammar.symbolId(":"), s_assignment_exp }, genericAssociationDefault);

    int s_argument_exp_list = grammar.symbolId("<argument_exp_list>");
    int s_postfix_exp = grammar.symbolId("<postfix_exp>");
    bind(s_postfix_exp, { s_primary_exp }, doNothing);
    bind(s_postfix_exp, { s_postfix_exp, s_open_bracket, s_exp, s_close_bracket }, arrayAccess);
    bind(s_postfix_exp, { s_postfix_exp, s_open_paren, s_argument_exp_list, s_close_paren }, functionCall);
    bind(s_postfix_exp, { s_postfix_exp, s_open_paren, s_close_paren }, noargFunctionCall);
    bind(s_postfix_exp, { s_postfix_exp, grammar.symbolId("."), s_identifier }, directMemberAccess);
    bind(s_postfix_exp, { s_postfix_exp, grammar.symbolId("->"), s_identifier }, pointeeMemberAccess);
    bind(s_postfix_exp, { s_postfix_exp, grammar.symbolId("++") }, postfixIncrementDecrement);
    bind(s_postfix_exp, { s_postfix_exp, grammar.symbolId("--") }, postfixIncrementDecrement);

    int s_cast_exp = grammar.symbolId("<cast_exp>");
    int s_unary_exp = grammar.symbolId("<unary_exp>");
    int s_unary_operator = grammar.symbolId("<unary_operator>");
    bind(s_unary_exp, { s_postfix_exp }, doNothing);
    bind(s_unary_exp, { grammar.symbolId("++"), s_unary_exp }, prefixIncrementDecrement);
    bind(s_unary_exp, { grammar.symbolId("--"), s_unary_exp }, prefixIncrementDecrement);
    bind(s_unary_exp, { s_unary_operator, s_cast_exp }, unaryExpression);
    bind(s_unary_exp, { grammar.symbolId("sizeof"), s_unary_exp }, sizeofExpression);
    bind(s_unary_exp, { grammar.symbolId("sizeof"), s_open_paren, grammar.symbolId("<type_name>"), s_close_paren }, sizeofTypeExpression);

    bind(s_cast_exp, { s_unary_exp }, doNothing);
    bind(s_cast_exp, { s_open_paren, grammar.symbolId("<type_name>"), s_close_paren, s_cast_exp }, typeCast);

    // type_name / spec_qualifier_list for sizeof(type) and casts.
    int s_spec_qualifier_list = grammar.symbolId("<spec_qualifier_list>");
    int s_type_name = grammar.symbolId("<type_name>");
    int s_abstract_declarator_sym = grammar.symbolId("<abstract_declarator>");
    // Same accumulator as decl_specs: type specs + cv (no storage class).
    bind(s_spec_qualifier_list, { s_type_specifier }, declarationTypeSpecifier);
    bind(s_spec_qualifier_list, { s_type_specifier, s_spec_qualifier_list }, addDeclarationTypeSpecifier);
    bind(s_spec_qualifier_list, { s_type_qualifier }, declarationTypeQualifier);
    bind(s_spec_qualifier_list, { s_type_qualifier, s_spec_qualifier_list }, addDeclarationTypeQualifier);
    bind(s_type_name, { s_spec_qualifier_list }, specQualifierListTypeName);
    bind(s_type_name, { s_spec_qualifier_list, s_abstract_declarator_sym }, typeNameWithAbstractDeclarator);

    int s_mult_exp = grammar.symbolId("<mult_exp>");
    bind(s_mult_exp, { s_cast_exp }, doNothing);
    bind(s_mult_exp, { s_mult_exp, grammar.symbolId("*"), s_cast_exp }, arithmeticExpression);
    bind(s_mult_exp, { s_mult_exp, grammar.symbolId("/"), s_cast_exp }, arithmeticExpression);
    bind(s_mult_exp, { s_mult_exp, grammar.symbolId("%"), s_cast_exp }, arithmeticExpression);

    int s_additive_exp = grammar.symbolId("<additive_exp>");
    bind(s_additive_exp, { s_mult_exp }, doNothing);
    bind(s_additive_exp, { s_additive_exp, grammar.symbolId("+"), s_mult_exp }, arithmeticExpression);
    bind(s_additive_exp, { s_additive_exp, grammar.symbolId("-"), s_mult_exp }, arithmeticExpression);

    int s_shift_exp = grammar.symbolId("<shift_expression>");
    bind(s_shift_exp, { s_additive_exp }, doNothing);
    bind(s_shift_exp, { s_shift_exp, grammar.symbolId("<<"), s_additive_exp }, shiftExpression);
    bind(s_shift_exp, { s_shift_exp, grammar.symbolId(">>"), s_additive_exp }, shiftExpression);

    int s_relational_exp = grammar.symbolId("<relational_exp>");
    bind(s_relational_exp, { s_shift_exp }, doNothing);
    bind(s_relational_exp, { s_relational_exp, grammar.symbolId("<"), s_shift_exp }, relationalExpression);
    bind(s_relational_exp, { s_relational_exp, grammar.symbolId(">"), s_shift_exp }, relationalExpression);
    bind(s_relational_exp, { s_relational_exp, grammar.symbolId("<="), s_shift_exp }, relationalExpression);
    bind(s_relational_exp, { s_relational_exp, grammar.symbolId(">="), s_shift_exp }, relationalExpression);

    int s_equality_exp = grammar.symbolId("<equality_exp>");
    bind(s_equality_exp, { s_relational_exp }, doNothing);
    bind(s_equality_exp, { s_equality_exp, grammar.symbolId("=="), s_relational_exp }, relationalExpression);
    bind(s_equality_exp, { s_equality_exp, grammar.symbolId("!="), s_relational_exp }, relationalExpression);

    int s_and_exp = grammar.symbolId("<and_exp>");
    bind(s_and_exp, { s_equality_exp }, doNothing);
    bind(s_and_exp, { s_and_exp, grammar.symbolId("&"), s_equality_exp }, bitwiseExpression);

    int s_exclusive_or_exp = grammar.symbolId("<exclusive_or_exp>");
    bind(s_exclusive_or_exp, { s_and_exp }, doNothing);
    bind(s_exclusive_or_exp, { s_exclusive_or_exp, grammar.symbolId("^"), s_and_exp }, bitwiseExpression);

    int s_inclusive_or_exp = grammar.symbolId("<inclusive_or_exp>");
    bind(s_inclusive_or_exp, { s_exclusive_or_exp }, doNothing);
    bind(s_inclusive_or_exp, { s_inclusive_or_exp, grammar.symbolId("|"), s_exclusive_or_exp }, bitwiseExpression);

    int s_logical_and_exp = grammar.symbolId("<logical_and_exp>");
    bind(s_logical_and_exp, { s_inclusive_or_exp }, doNothing);
    bind(s_logical_and_exp, { s_logical_and_exp, grammar.symbolId("&&"), s_inclusive_or_exp }, logicalAndExpression);

    int s_logical_or_exp = grammar.symbolId("<logical_or_exp>");
    bind(s_logical_or_exp, { s_logical_and_exp }, doNothing);
    bind(s_logical_or_exp, { s_logical_or_exp, grammar.symbolId("||"), s_logical_and_exp }, logicalOrExpression);

    int s_conditional_exp = grammar.symbolId("<conditional_exp>");
    bind(s_conditional_exp, { s_logical_or_exp }, doNothing);
    bind(s_conditional_exp, { s_logical_or_exp, grammar.symbolId("?"), s_exp, grammar.symbolId(":"), s_conditional_exp }, conditionalExpression);

    // Identity: const_exp is a conditional_exp (array bounds, enum values, case labels, bit-fields).
    int s_const_exp = grammar.symbolId("<const_exp>");
    bind(s_const_exp, { s_conditional_exp }, doNothing);

    int s_assignment = grammar.symbolId("<assignment_exp>");
    int s_assignment_operator = grammar.symbolId("<assignment_operator>");
    bind(s_assignment, { s_conditional_exp }, doNothing);
    bind(s_assignment, { s_unary_exp, s_assignment_operator, s_assignment }, assignmentExpression);

    bind(s_type_specifier, { grammar.symbolId("typeof"), s_open_paren, grammar.symbolId("<type_name>"), s_close_paren }, typeofTypeName);
    bind(s_type_specifier, { grammar.symbolId("typeof"), s_open_paren, s_assignment, s_close_paren }, typeofExpression);

    int s_initializer = grammar.symbolId("<initializer>");
    int s_open_brace = grammar.symbolId("{");
    int s_close_brace = grammar.symbolId("}");
    int s_initializer_list = grammar.symbolId("<initializer_list>");
    bind(s_initializer, { s_assignment }, doNothing);
    bind(s_initializer, { s_open_brace, s_initializer_list, s_close_brace }, braceInitializer);
    bind(s_initializer, { s_open_brace, s_initializer_list, s_comma, s_close_brace }, braceInitializerTrailingComma);
    bind(s_postfix_exp, { s_open_paren, grammar.symbolId("<type_name>"), s_close_paren, s_open_brace, s_initializer_list, s_close_brace }, compoundLiteral);
    bind(s_postfix_exp, { s_open_paren, grammar.symbolId("<type_name>"), s_close_paren, s_open_brace, s_initializer_list, s_comma, s_close_brace }, compoundLiteralTrailingComma);

    int s_designator = grammar.symbolId("<designator>");
    int s_designator_list = grammar.symbolId("<designator_list>");
    int s_designation = grammar.symbolId("<designation>");
    bind(s_designator, { grammar.symbolId("."), grammar.symbolId("id") }, memberDesignator);
    bind(s_designator, { grammar.symbolId("["), s_const_exp, grammar.symbolId("]") }, arrayDesignator);
    bind(s_designator_list, { s_designator }, designatorListSingle);
    bind(s_designator_list, { s_designator_list, s_designator }, designatorListAppend);
    bind(s_designation, { s_designator_list, grammar.symbolId("=") }, designation);

    bind(s_initializer_list, { s_initializer }, initializerListFirst);
    bind(s_initializer_list, { s_designation, s_initializer }, designatedInitializerListFirst);
    bind(s_initializer_list, { s_initializer_list, s_comma, s_initializer }, initializerListAppend);
    bind(s_initializer_list, { s_initializer_list, s_comma, s_designation, s_initializer }, designatedInitializerListAppend);

    int s_init_declarator = grammar.symbolId("<init_declarator>");
    bind(s_init_declarator, { s_declarator }, initializedDeclarator);
    bind(s_init_declarator, { s_declarator, grammar.symbolId("="), s_initializer }, initializedDeclaratorWithInitializer);

    int s_init_declarator_list = grammar.symbolId("<init_declarator_list>");
    bind(s_init_declarator_list, { s_init_declarator }, initializedDeclaratorList);
    bind(s_init_declarator_list, { s_init_declarator_list, s_comma, s_init_declarator }, addToInitializedDeclaratorList);

    int s_decl = grammar.symbolId("<decl>");
    int s_semicolon = grammar.symbolId(";");
    bind(s_decl, { s_decl_specs, s_init_declarator_list, s_semicolon }, initializedDeclaration);
    bind(s_decl, { s_decl_specs, s_semicolon }, declaration);

    int s_decl_list = grammar.symbolId("<decl_list>");
    bind(s_decl_list, { s_decl }, declarationList);
    bind(s_decl_list, { s_decl_list, s_decl }, addDeclarationToList);

    bind(s_exp, { s_assignment }, doNothing);
    bind(s_exp, { s_exp, s_comma, s_assignment }, expressionList);

    bind(s_unary_operator, { grammar.symbolId("&") }, doNothing);
    bind(s_unary_operator, { grammar.symbolId("*") }, doNothing);
    bind(s_unary_operator, { grammar.symbolId("+") }, doNothing);
    bind(s_unary_operator, { grammar.symbolId("-") }, doNothing);
    bind(s_unary_operator, { grammar.symbolId("~") }, doNothing);
    bind(s_unary_operator, { grammar.symbolId("!") }, doNothing);

    bind(s_assignment_operator, { grammar.symbolId("=") }, doNothing);
    bind(s_assignment_operator, { grammar.symbolId("*=") }, doNothing);
    bind(s_assignment_operator, { grammar.symbolId("/=") }, doNothing);
    bind(s_assignment_operator, { grammar.symbolId("%=") }, doNothing);
    bind(s_assignment_operator, { grammar.symbolId("+=") }, doNothing);
    bind(s_assignment_operator, { grammar.symbolId("-=") }, doNothing);
    bind(s_assignment_operator, { grammar.symbolId("<<=") }, doNothing);
    bind(s_assignment_operator, { grammar.symbolId(">>=") }, doNothing);
    bind(s_assignment_operator, { grammar.symbolId("&=") }, doNothing);
    bind(s_assignment_operator, { grammar.symbolId("^=") }, doNothing);
    bind(s_assignment_operator, { grammar.symbolId("|=") }, doNothing);

    int s_exp_stat = grammar.symbolId("<exp_stat>");
    bind(s_exp_stat, { s_exp, s_semicolon }, expressionStatement);
    bind(s_exp_stat, { s_semicolon }, emptyStatement);

    int s_matched = grammar.symbolId("<matched>");
    int s_unmatched = grammar.symbolId("<unmatched>");
    int s_stat = grammar.symbolId("<stat>");
    int s_compound_stat = grammar.symbolId("<compound_stat>");
    int s_jump_stat = grammar.symbolId("<jump_stat>");
    int s_if = grammar.symbolId("if");
    int s_colon = grammar.symbolId(":");
    int s_labeled_stat_matched = grammar.symbolId("<labeled_stat_matched>");
    int s_labeled_stat_unmatched = grammar.symbolId("<labeled_stat_unmatched>");
    bind(s_matched, { s_if, s_open_paren, s_exp, s_close_paren, s_matched, grammar.symbolId("else"), s_matched }, ifElseStatement);
    bind(s_unmatched, { s_if, s_open_paren, s_exp, s_close_paren, s_stat }, ifStatement);
    bind(s_unmatched, { s_if, s_open_paren, s_exp, s_close_paren, s_matched, grammar.symbolId("else"), s_unmatched }, ifElseStatement);
    int s_switch = grammar.symbolId("switch");
    bind(s_matched, { s_switch, s_open_paren, s_exp, s_close_paren, s_matched }, switchStatement);
    bind(s_unmatched, { s_switch, s_open_paren, s_exp, s_close_paren, s_unmatched }, switchStatement);
    bind(s_matched, { s_labeled_stat_matched }, doNothing);
    bind(s_unmatched, { s_labeled_stat_unmatched }, doNothing);
    int s_case = grammar.symbolId("case");
    int s_default = grammar.symbolId("default");
    // s_identifier defined earlier with declarators / primary_exp.
    bind(s_labeled_stat_matched, { s_case, grammar.symbolId("<const_exp>"), s_colon, s_matched }, caseLabel);
    bind(s_labeled_stat_unmatched, { s_case, grammar.symbolId("<const_exp>"), s_colon, s_unmatched }, caseLabel);
    bind(s_labeled_stat_matched, { s_default, s_colon, s_matched }, defaultLabel);
    bind(s_labeled_stat_unmatched, { s_default, s_colon, s_unmatched }, defaultLabel);
    bind(s_labeled_stat_matched, { s_identifier, s_colon, s_matched }, namedLabel);
    bind(s_labeled_stat_unmatched, { s_identifier, s_colon, s_unmatched }, namedLabel);
    bind(s_matched, { s_exp_stat }, doNothing);
    bind(s_matched, { s_compound_stat }, doNothing);
    bind(s_matched, { s_jump_stat }, doNothing);

    bind(s_stat, { s_matched }, doNothing);
    bind(s_stat, { s_unmatched }, doNothing);

    int s_stat_list = grammar.symbolId("<stat_list>");
    bind(s_stat_list, { s_stat }, statementList);
    bind(s_stat_list, { s_stat_list, s_stat }, addToStatementList);

    int s_return = grammar.symbolId("return");
    bind(s_jump_stat, { grammar.symbolId("goto"), s_identifier, s_semicolon }, gotoStatement);
    bind(s_jump_stat, { grammar.symbolId("continue"), s_semicolon }, loopJumpStatement);
    bind(s_jump_stat, { grammar.symbolId("break"), s_semicolon }, loopJumpStatement);
    bind(s_jump_stat, { s_return, s_exp, s_semicolon }, returnExpressionStatement);
    bind(s_jump_stat, { s_return, s_semicolon }, returnVoidStatement);

    bind(s_argument_exp_list, { s_assignment }, createActualArgumentsList);
    bind(s_argument_exp_list, { s_argument_exp_list, s_comma, s_assignment }, addToActualArgumentsList);

    int s_block_item = grammar.symbolId("<block_item>");
    int s_block_item_list = grammar.symbolId("<block_item_list>");
    int s_stat_for_block = grammar.symbolId("<stat>");
    bind(s_block_item, { s_decl }, blockItemDeclaration);
    bind(s_block_item, { s_stat_for_block }, doNothing);
    bind(s_block_item_list, { s_block_item }, statementList);
    bind(s_block_item_list, { s_block_item_list, s_block_item }, addToStatementList);
    bind(s_compound_stat, { s_open_brace, s_block_item_list, s_close_brace }, blockItemListCompound);
    bind(s_compound_stat, { s_open_brace, s_close_brace }, emptyCompound);

    int s_function_definition = grammar.symbolId("<function_definition>");
    bind(s_function_definition, { s_decl_specs, s_declarator, s_compound_stat }, functionDefinition);
    bind(s_function_definition, { s_declarator, s_compound_stat }, defaultReturnTypeFunctionDefinition);
    // K&R definitions: `int f(a) int a; { ... }` (parameter decls between declarator and body).
    bind(s_function_definition, { s_decl_specs, s_declarator, s_decl_list, s_compound_stat }, notImplementedYet("K&R style function definitions"));
    bind(s_function_definition, { s_declarator, s_decl_list, s_compound_stat }, notImplementedYet("K&R style function definitions"));

    int s_external_decl = grammar.symbolId("<external_decl>");
    bind(s_external_decl, { s_function_definition }, externalFunctionDefinition);
    bind(s_external_decl, { s_decl }, externalDeclaration);

    int s_translation_unit = grammar.symbolId("<translation_unit>");
    bind(s_translation_unit, { s_external_decl }, translationUnit);
    bind(s_translation_unit, { s_translation_unit, s_external_decl }, addToTranslationUnit);

    int s_iteration_stat_matched = grammar.symbolId("<iteration_stat_matched>");
    int s_iteration_stat_unmatched = grammar.symbolId("<iteration_stat_unmatched>");
    bind(s_matched, { s_iteration_stat_matched }, doNothing);
    bind(s_unmatched, { s_iteration_stat_unmatched }, doNothing);

    int s_while = grammar.symbolId("while");
    int s_do = grammar.symbolId("do");
    int s_for = grammar.symbolId("for");
    bind(s_iteration_stat_matched, { s_while, s_open_paren, s_exp, s_close_paren, s_matched }, whileLoopStatement);
    bind(s_iteration_stat_unmatched, { s_while, s_open_paren, s_exp, s_close_paren, s_unmatched }, whileLoopStatement);
    bind(s_iteration_stat_matched, { s_do, s_matched, s_while, s_open_paren, s_exp, s_close_paren, s_semicolon }, doWhileLoopStatement);
    bind(s_iteration_stat_unmatched, { s_do, s_unmatched, s_while, s_open_paren, s_exp, s_close_paren, s_semicolon }, doWhileLoopStatement);

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
    auto registerFor = [&](const std::vector<int>& prod, Creator creator) {
        bind(s_iteration_stat_matched, prod, creator);
        auto unmatchedProd = prod;
        unmatchedProd.back() = s_unmatched;
        bind(s_iteration_stat_unmatched, unmatchedProd, creator);
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

    bind(s_enumerator, { s_id_for_enum }, [](AbstractSyntaxTreeBuilderContext& context) {
        auto id = context.popTerminal();
        if (context.environment().session().enums.containsInCurrentScope(id.value)) {
            context.error(id.context, "redefinition of enumerator `" + id.value + "`");
            return;
        }
        context.environment().addEnumerator(id.value);
    });
    bind(s_enumerator, { s_id_for_enum, grammar.symbolId("="), s_enum_const_exp }, [](AbstractSyntaxTreeBuilderContext& context) {
                auto expr = context.popExpression();
                context.popTerminal(); // =
                auto id = context.popTerminal();
                type::IntegerConstant ice;
                if (!expr->evaluateConstant(ice)) {
                    context.error(id.context,
                            "enumerator initializer is not a constant expression: " + id.value);
                    return;
                }
                if (context.environment().session().enums.containsInCurrentScope(id.value)) {
                    context.error(id.context, "redefinition of enumerator `" + id.value + "`");
                    return;
                }
                context.environment().addEnumerator(id.value, std::move(ice));
            });
    bind(s_enumerator_list, { s_enumerator }, doNothing);
    bind(s_enumerator_list, { s_enumerator_list, s_comma, s_enumerator }, [](AbstractSyntaxTreeBuilderContext& context) { context.popTerminal(); });
    // C99 trailing comma after last enumerator.
    bind(s_enumerator_list, { s_enumerator_list, s_comma }, [](AbstractSyntaxTreeBuilderContext& context) { context.popTerminal(); });

    // Enum types are integer stand-ins; underlying width follows enumerator range (GCC/SysV).
    // Enumerator values live on LexicalSession; named tags keep the chosen type.
    bind(s_enum_spec, { s_enum_kw, s_id_for_enum, s_open_brace, s_enumerator_list, s_close_brace }, [](AbstractSyntaxTreeBuilderContext& context) {
                context.popTerminal(); // }
                context.popTerminal(); // {
                auto tag = context.popTerminal();
                context.popTerminal(); // enum
                type::Type underlying = context.environment().endEnumDefinition(tag.value);
                context.pushTypeSpecifier(TypeSpecifier { underlying, tag.value });
            });
    bind(s_enum_spec, { s_enum_kw, s_open_brace, s_enumerator_list, s_close_brace }, [](AbstractSyntaxTreeBuilderContext& context) {
                context.popTerminal(); // }
                context.popTerminal(); // {
                context.popTerminal(); // enum
                type::Type underlying = context.environment().endEnumDefinition();
                context.pushTypeSpecifier(TypeSpecifier { underlying, "" });
            });
    bind(s_enum_spec, { s_enum_kw, s_id_for_enum }, [](AbstractSyntaxTreeBuilderContext& context) {
                auto tag = context.popTerminal();
                context.popTerminal(); // enum
                type::Type underlying = context.environment().lookupEnumTag(tag.value)
                        .value_or(type::signedInteger());
                context.pushTypeSpecifier(TypeSpecifier { underlying, tag.value });
            });

    // --- struct / union ---
    int s_struct_or_union = grammar.symbolId("<struct_or_union>");
    int s_struct_or_union_spec = grammar.symbolId("<struct_or_union_spec>");
    int s_struct_decl_list = grammar.symbolId("<struct_decl_list>");
    int s_struct_decl = grammar.symbolId("<struct_decl>");
    int s_struct_declarator_list = grammar.symbolId("<struct_declarator_list>");
    int s_struct_declarator = grammar.symbolId("<struct_declarator>");
    // s_open_brace / s_close_brace already defined above for compound statements.

    bind(s_struct_or_union, { grammar.symbolId("struct") }, [](AbstractSyntaxTreeBuilderContext& context) {
        context.popTerminal();
        context.environment().session().recordPacked.begin();
        context.pushIsUnion(false);
        context.newStructMemberList();
    });
    bind(s_struct_or_union, { grammar.symbolId("union") }, [](AbstractSyntaxTreeBuilderContext& context) {
        context.popTerminal();
        context.environment().session().recordPacked.begin();
        context.pushIsUnion(true);
        context.newStructMemberList();
    });

    bind(s_struct_or_union_spec, { s_struct_or_union, s_identifier, s_open_brace, s_struct_decl_list, s_close_brace }, [](AbstractSyntaxTreeBuilderContext& context) {
                const auto close = context.popTerminal(); // }
                context.popTerminal(); // {
                auto tag = context.popTerminal();
                auto members = context.popStructMemberList();
                bool isUnion = context.popIsUnion();
                // Shared incomplete tag so self-referential members keep one layout identity.
                type::Type tagType = context.environment().ensureStructTag(tag.value);
                completeRecordFromSpec(context, tagType, std::move(members), isUnion, close.context);
                if (context.failed()) {
                    return;
                }
                // Shared body: tagType already sees completion via structureBodyIdentity().
                TypeSpecifier spec { tagType, tag.value };
                spec.markDefinesRecord();
                context.pushTypeSpecifier(std::move(spec));
            });
    bind(s_struct_or_union_spec, { s_struct_or_union, s_open_brace, s_struct_decl_list, s_close_brace }, [](AbstractSyntaxTreeBuilderContext& context) {
                const auto close = context.popTerminal(); // }
                context.popTerminal(); // {
                auto members = context.popStructMemberList();
                bool isUnion = context.popIsUnion();
                type::Type completed = type::incompleteRecord();
                completeRecordFromSpec(context, completed, std::move(members), isUnion, close.context);
                if (context.failed()) {
                    return;
                }
                TypeSpecifier spec { completed, "" };
                spec.markDefinesRecord();
                context.pushTypeSpecifier(std::move(spec));
            });
    bind(s_struct_or_union_spec, { s_struct_or_union, s_identifier }, [](AbstractSyntaxTreeBuilderContext& context) {
                auto tag = context.popTerminal();
                context.popIsUnion(); // layout decided at definition
                context.popStructMemberList(); // no body
                context.environment().session().recordPacked.abandon();
                context.pushTypeSpecifier(TypeSpecifier {
                        context.environment().ensureStructTag(tag.value), tag.value });
            });

    bind(s_struct_declarator, { s_declarator }, [](AbstractSyntaxTreeBuilderContext& context) {
        context.addStructDeclarator(context.popDeclarator());
    });
    bind(s_struct_declarator, { s_declarator, grammar.symbolId(":"), grammar.symbolId("<const_exp>") }, [](AbstractSyntaxTreeBuilderContext& context) {
                const int width = foldBitFieldWidth(context);
                if (context.failed()) {
                    return;
                }
                context.addStructDeclarator(context.popDeclarator(), width);
            });
    bind(s_struct_declarator, { grammar.symbolId(":"), grammar.symbolId("<const_exp>") }, [](AbstractSyntaxTreeBuilderContext& context) {
                const int width = foldBitFieldWidth(context);
                if (context.failed()) {
                    return;
                }
                context.addStructDeclarator(nullptr, width);
            });
    bind(s_struct_declarator_list, { s_struct_declarator }, doNothing);
    bind(s_struct_declarator_list, { s_struct_declarator_list, s_comma, s_struct_declarator }, [](AbstractSyntaxTreeBuilderContext& context) { context.popTerminal(); });

    bind(s_struct_decl, { s_spec_qualifier_list, s_struct_declarator_list, s_semicolon }, [](AbstractSyntaxTreeBuilderContext& context) {
                context.popTerminal(); // ;
                auto declarators = context.popStructDeclarators();
                auto specs = popResolvedSpecQualifiers(context);
                if (context.failed()) {
                    return;
                }
                auto baseType = specs.getResolvedType();
                for (auto& [declarator, bitWidth] : declarators) {
                    if (!declarator) {
                        context.addStructMember("", baseType, bitWidth);
                    } else {
                        context.addStructMember(declarator->getName(),
                                declarator->getFundamentalType(baseType), bitWidth);
                    }
                }
            });
    // C11 anonymous struct/union: untagged record body (empty stored name).
    // Tagged type-only forms (struct T { ... };) must not become empty-name members.
    bind(s_struct_decl, { s_spec_qualifier_list, s_semicolon }, [](AbstractSyntaxTreeBuilderContext& context) {
                context.popTerminal(); // ;
                auto specs = popResolvedSpecQualifiers(context);
                if (context.failed()) {
                    return;
                }
                if (specs.isUntaggedRecordBody()) {
                    context.addStructMember("", specs.getResolvedType());
                }
            });
    bind(s_struct_decl_list, { s_struct_decl }, doNothing);
    bind(s_struct_decl_list, { s_struct_decl_list, s_struct_decl }, doNothing);
}

ContextualSyntaxNodeBuilder::~ContextualSyntaxNodeBuilder() = default;

void ContextualSyntaxNodeBuilder::bind(int lhs, std::vector<int> rhs, Creator creator) {
    for (const parser::Production& production : grammar->getProductionsOfSymbol(lhs)) {
        if (production.producedSequence() != rhs) {
            continue;
        }
        const int id = production.getId();
        if (id < 0 || static_cast<std::size_t>(id) >= creators_.size()) {
            throw std::logic_error { "production id out of range" };
        }
        if (creators_[id]) {
            throw std::logic_error { "duplicate creator for production" };
        }
        creators_[id] = std::move(creator);
        return;
    }
    throw std::logic_error { "no production for creator" };
}

void ContextualSyntaxNodeBuilder::updateContext(const parser::Production& production, AbstractSyntaxTreeBuilderContext& context) const {
    const int id = production.getId();
    if (id < 0 || static_cast<std::size_t>(id) >= creators_.size() || !creators_[id]) {
        noCreatorDefined(production, context);
        return;
    }
    creators_[id](context);
}

void ContextualSyntaxNodeBuilder::noCreatorDefined(const parser::Production& production,
        AbstractSyntaxTreeBuilderContext& context) const {
    context.error({ "", 0 },
            "language construct not implemented yet (production `" + grammar->str(production) + "`)");
}

void ContextualSyntaxNodeBuilder::loopJumpStatement(AbstractSyntaxTreeBuilderContext& context) {
    context.popTerminal(); // ;
    context.pushStatement(std::make_unique<JumpStatement>(context.popTerminal())); // break | continue
}

} // namespace ast
