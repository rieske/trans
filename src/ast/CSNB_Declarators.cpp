#include "CSNB_Internal.h"

#include "ArrayDeclarator.h"
#include "Declarator.h"
#include "FormalArgument.h"
#include "FunctionDeclarator.h"
#include "Identifier.h"
#include "NestedDeclarator.h"
#include "Pointer.h"
#include "types/Type.h"

namespace ast {
namespace csnb {

namespace {

void parenthesizedDeclarator(AbstractSyntaxTreeBuilderContext& context) {
    context.popTerminal(); // )
    context.popTerminal(); // (
    context.pushDirectDeclarator(std::make_unique<NestedDeclarator>(context.popDeclarator()));
}
void identifierDeclarator(AbstractSyntaxTreeBuilderContext& context) {
    context.pushDirectDeclarator(std::make_unique<Identifier>(context.popTerminal()));
}

std::unique_ptr<Identifier> anonymousIdentifier() {
    return std::make_unique<Identifier>(
            TerminalSymbol { "id", "", translation_unit::Context { "", 0 } });
}

void wrapArray(AbstractSyntaxTreeBuilderContext& context, std::unique_ptr<Expression> size,
        bool existingDirect) {
    std::unique_ptr<DirectDeclarator> base = existingDirect
            ? context.popDirectDeclarator()
            : anonymousIdentifier();
    context.pushDirectDeclarator(std::make_unique<ArrayDeclarator>(std::move(base), std::move(size)));
}

void arrayDeclarator(AbstractSyntaxTreeBuilderContext& context) {
    context.popTerminal();
    auto size = context.popExpression();
    context.popTerminal();
    wrapArray(context, std::move(size), true);
}

void incompleteArrayDeclarator(AbstractSyntaxTreeBuilderContext& context) {
    context.popTerminal();
    context.popTerminal();
    wrapArray(context, nullptr, true);
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
    auto declarator = context.popDeclarator();
    auto specs = context.popDeclarationSpecifiers();
    context.pushFormalArgument(FormalArgument { std::move(specs), std::move(declarator) });
}

void abstractDeclaratorPointer(AbstractSyntaxTreeBuilderContext& context) {
    context.pushDeclarator(std::make_unique<Declarator>(anonymousIdentifier(), context.popPointers()));
}

void abstractDeclaratorPointerAndDirect(AbstractSyntaxTreeBuilderContext& context) {
    context.pushDeclarator(std::make_unique<Declarator>(
            context.popDirectDeclarator(), context.popPointers()));
}

void abstractDeclaratorDirect(AbstractSyntaxTreeBuilderContext& context) {
    context.pushDeclarator(std::make_unique<Declarator>(context.popDirectDeclarator()));
}

void abstractParenDeclarator(AbstractSyntaxTreeBuilderContext& context) {
    context.popTerminal();
    context.popTerminal();
    context.pushDirectDeclarator(std::make_unique<NestedDeclarator>(context.popDeclarator()));
}

void abstractArrayOnly(AbstractSyntaxTreeBuilderContext& context) {
    context.popTerminal();
    context.popTerminal();
    wrapArray(context, nullptr, false);
}

void abstractArrayOnlySized(AbstractSyntaxTreeBuilderContext& context) {
    context.popTerminal();
    auto size = context.popExpression();
    context.popTerminal();
    wrapArray(context, std::move(size), false);
}

void abstractArraySuffix(AbstractSyntaxTreeBuilderContext& context) {
    context.popTerminal();
    context.popTerminal();
    wrapArray(context, nullptr, true);
}

void abstractArraySuffixSized(AbstractSyntaxTreeBuilderContext& context) {
    context.popTerminal();
    auto size = context.popExpression();
    context.popTerminal();
    wrapArray(context, std::move(size), true);
}

void abstractFuncOnly(AbstractSyntaxTreeBuilderContext& context) {
    context.popTerminal();
    context.popTerminal();
    context.pushDirectDeclarator(std::make_unique<FunctionDeclarator>(anonymousIdentifier()));
}

void abstractFuncOnlyWithParams(AbstractSyntaxTreeBuilderContext& context) {
    context.popTerminal();
    auto argumentsDeclaration = context.popArgumentsDeclaration();
    context.popTerminal();
    auto arguments = std::move(argumentsDeclaration.first);
    const bool variadic = argumentsDeclaration.second;
    if (arguments.size() == 1 && arguments.front().isVoid()) {
        arguments.clear();
    }
    context.pushDirectDeclarator(std::make_unique<FunctionDeclarator>(
            anonymousIdentifier(), std::move(arguments), variadic));
}

void abstractFuncSuffix(AbstractSyntaxTreeBuilderContext& context) {
    context.popTerminal();
    context.popTerminal();
    context.pushDirectDeclarator(std::make_unique<FunctionDeclarator>(context.popDirectDeclarator()));
}

void abstractFuncSuffixWithParams(AbstractSyntaxTreeBuilderContext& context) {
    context.popTerminal();
    auto argumentsDeclaration = context.popArgumentsDeclaration();
    context.popTerminal();
    auto arguments = std::move(argumentsDeclaration.first);
    const bool variadic = argumentsDeclaration.second;
    if (arguments.size() == 1 && arguments.front().isVoid()) {
        arguments.clear();
    }
    context.pushDirectDeclarator(std::make_unique<FunctionDeclarator>(
            context.popDirectDeclarator(), std::move(arguments), variadic));
}

// C99 [static]/qualifier array forms. Tokens are popped in reverse of the RHS.
enum class ArrayQualForm {
    QualSize,       // [ qual size ]
    QualOnly,       // [ qual ]
    StaticSize,     // [ static size ]
    StaticQualSize, // [ static qual size ]
    QualStaticSize, // [ qual static size ]
};

void arrayDeclaratorForm(AbstractSyntaxTreeBuilderContext& context, bool existingDirect,
        ArrayQualForm form) {
    context.popTerminal(); // ]
    std::unique_ptr<Expression> size;
    const bool hasSize = form != ArrayQualForm::QualOnly;
    if (hasSize) {
        size = context.popExpression();
    }
    if (form == ArrayQualForm::QualStaticSize) {
        context.popTerminal(); // static
    }
    if (form != ArrayQualForm::StaticSize) {
        context.popTypeQualifierList();
    }
    if (form == ArrayQualForm::StaticSize || form == ArrayQualForm::StaticQualSize) {
        context.popTerminal(); // static
    }
    context.popTerminal(); // [
    wrapArray(context, std::move(size), existingDirect);
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
    context.popTerminal(); // ...
    context.popTerminal(); // ,
    // Accept varargs in prototypes; call sites may pass extra args.
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

} // namespace


void registerDeclaratorProductions(const parser::Grammar& grammar, NodeCreatorRegistry& nodeCreatorRegistry) {
    int s_direct_declarator = grammar.symbolId("<direct_declarator>");
    int s_declarator = grammar.symbolId("<declarator>");
    int s_param_type_list = grammar.symbolId("<param_type_list>");
    int s_identifier = grammar.symbolId("id");
    int s_open_paren = grammar.symbolId("(");
    int s_close_paren = grammar.symbolId(")");
    int s_open_bracket = grammar.symbolId("[");
    int s_close_bracket = grammar.symbolId("]");
    int s_decl_specs = grammar.symbolId("<decl_specs>");

    int s_type_qualifier_list = grammar.symbolId("<type_qualifier_list>");
    int s_static = grammar.symbolId("static");
    int s_const_exp = grammar.symbolId("<const_exp>");

    nodeCreatorRegistry[s_direct_declarator][{ s_identifier }] = identifierDeclarator;
    nodeCreatorRegistry[s_direct_declarator][{ s_open_paren, s_declarator, s_close_paren }] = parenthesizedDeclarator;
    nodeCreatorRegistry[s_direct_declarator][{ s_direct_declarator, s_open_bracket, s_const_exp, s_close_bracket }] = arrayDeclarator;
    nodeCreatorRegistry[s_direct_declarator][{ s_direct_declarator, s_open_bracket, s_close_bracket }] = incompleteArrayDeclarator;
    auto bindArrayForm = [&](const std::vector<int>& seq, bool existing, ArrayQualForm form) {
        nodeCreatorRegistry[existing ? s_direct_declarator : grammar.symbolId("<direct_abstract_declarator>")][seq] =
                [existing, form](AbstractSyntaxTreeBuilderContext& c) {
                    arrayDeclaratorForm(c, existing, form);
                };
    };
    bindArrayForm({ s_direct_declarator, s_open_bracket, s_type_qualifier_list, s_const_exp, s_close_bracket },
            true, ArrayQualForm::QualSize);
    bindArrayForm({ s_direct_declarator, s_open_bracket, s_type_qualifier_list, s_close_bracket },
            true, ArrayQualForm::QualOnly);
    bindArrayForm({ s_direct_declarator, s_open_bracket, s_static, s_const_exp, s_close_bracket },
            true, ArrayQualForm::StaticSize);
    bindArrayForm({ s_direct_declarator, s_open_bracket, s_static, s_type_qualifier_list, s_const_exp, s_close_bracket },
            true, ArrayQualForm::StaticQualSize);
    bindArrayForm({ s_direct_declarator, s_open_bracket, s_type_qualifier_list, s_static, s_const_exp, s_close_bracket },
            true, ArrayQualForm::QualStaticSize);
    nodeCreatorRegistry[s_direct_declarator][{ s_direct_declarator, s_open_paren, s_param_type_list, s_close_paren }] = functionDeclarator;
    nodeCreatorRegistry[s_direct_declarator][{ s_direct_declarator, s_open_paren, s_close_paren }] = noargFunctionDeclarator;

    int s_pointer = grammar.symbolId("<pointer>" );
    nodeCreatorRegistry[s_declarator][{ s_pointer, s_direct_declarator }] = pointerToDeclarator;
    nodeCreatorRegistry[s_declarator][{ s_direct_declarator }] = declarator;

    int s_param_decl = grammar.symbolId("<param_decl>");
    nodeCreatorRegistry[s_param_decl][{ s_decl_specs, s_declarator }] = parameterDeclaration;
    nodeCreatorRegistry[s_param_decl][{ s_decl_specs, grammar.symbolId("<abstract_declarator>") }] = abstractParameterDeclaration;
    nodeCreatorRegistry[s_param_decl][{ s_decl_specs }] = parameterBaseTypeDeclaration;

    int s_param_list = grammar.symbolId("<param_list>");
    int s_comma = grammar.symbolId(",");
    nodeCreatorRegistry[s_param_list][{ s_param_decl }] = formalArguments;
    nodeCreatorRegistry[s_param_list][{ s_param_list, s_comma, s_param_decl }] = addFormalArgument;

    // K&R identifier parameter list: `f(a, b)` — not the modern `f(int a, int b)`.
    int s_id_list = grammar.symbolId("<id_list>");
    nodeCreatorRegistry[s_id_list][{ s_identifier }] = notImplementedYet("K&R identifier parameter lists");
    nodeCreatorRegistry[s_id_list][{ s_id_list, s_comma, s_identifier }] =
            notImplementedYet("K&R identifier parameter lists");
    nodeCreatorRegistry[s_direct_declarator][{ s_direct_declarator, s_open_paren, s_id_list, s_close_paren }] =
            notImplementedYet("K&R identifier parameter lists");

    nodeCreatorRegistry[s_param_type_list][{ s_param_list }] = formalArgumentsDeclaration;
    nodeCreatorRegistry[s_param_type_list][{ s_param_list, s_comma, grammar.symbolId("...") }] = formalArgumentsWithVararg;

    int s_abstract_declarator = grammar.symbolId("<abstract_declarator>");
    int s_direct_abstract_declarator = grammar.symbolId("<direct_abstract_declarator>");
    nodeCreatorRegistry[s_abstract_declarator][{ s_pointer }] = abstractDeclaratorPointer;
    nodeCreatorRegistry[s_abstract_declarator][{ s_pointer, s_direct_abstract_declarator }] =
            abstractDeclaratorPointerAndDirect;
    nodeCreatorRegistry[s_abstract_declarator][{ s_direct_abstract_declarator }] = abstractDeclaratorDirect;

    nodeCreatorRegistry[s_direct_abstract_declarator][{ s_open_paren, s_abstract_declarator, s_close_paren }] =
            abstractParenDeclarator;
    nodeCreatorRegistry[s_direct_abstract_declarator][{ s_open_bracket, s_close_bracket }] = abstractArrayOnly;
    nodeCreatorRegistry[s_direct_abstract_declarator][{ s_open_paren, s_close_paren }] = abstractFuncOnly;
    nodeCreatorRegistry[s_direct_abstract_declarator][{ s_direct_abstract_declarator, s_open_bracket, s_close_bracket }] =
            abstractArraySuffix;
    nodeCreatorRegistry[s_direct_abstract_declarator][{ s_direct_abstract_declarator, s_open_paren, s_close_paren }] =
            abstractFuncSuffix;
    nodeCreatorRegistry[s_direct_abstract_declarator][
            { s_direct_abstract_declarator, s_open_bracket, s_const_exp, s_close_bracket }] = abstractArraySuffixSized;
    nodeCreatorRegistry[s_direct_abstract_declarator][{ s_open_bracket, s_const_exp, s_close_bracket }] =
            abstractArrayOnlySized;
    nodeCreatorRegistry[s_direct_abstract_declarator][
            { s_direct_abstract_declarator, s_open_paren, s_param_type_list, s_close_paren }] =
            abstractFuncSuffixWithParams;
    nodeCreatorRegistry[s_direct_abstract_declarator][{ s_open_paren, s_param_type_list, s_close_paren }] =
            abstractFuncOnlyWithParams;

    auto bindAbs = [&](const std::vector<int>& seq, bool existing, ArrayQualForm form) {
        nodeCreatorRegistry[s_direct_abstract_declarator][seq] =
                [existing, form](AbstractSyntaxTreeBuilderContext& c) {
                    arrayDeclaratorForm(c, existing, form);
                };
    };
    bindAbs({ s_direct_abstract_declarator, s_open_bracket, s_type_qualifier_list, s_const_exp, s_close_bracket },
            true, ArrayQualForm::QualSize);
    bindAbs({ s_open_bracket, s_type_qualifier_list, s_const_exp, s_close_bracket },
            false, ArrayQualForm::QualSize);
    bindAbs({ s_direct_abstract_declarator, s_open_bracket, s_type_qualifier_list, s_close_bracket },
            true, ArrayQualForm::QualOnly);
    bindAbs({ s_open_bracket, s_type_qualifier_list, s_close_bracket }, false, ArrayQualForm::QualOnly);
    bindAbs({ s_direct_abstract_declarator, s_open_bracket, s_static, s_const_exp, s_close_bracket },
            true, ArrayQualForm::StaticSize);
    bindAbs({ s_open_bracket, s_static, s_const_exp, s_close_bracket }, false, ArrayQualForm::StaticSize);
    bindAbs({ s_direct_abstract_declarator, s_open_bracket, s_static, s_type_qualifier_list, s_const_exp,
                    s_close_bracket },
            true, ArrayQualForm::StaticQualSize);
    bindAbs({ s_open_bracket, s_static, s_type_qualifier_list, s_const_exp, s_close_bracket },
            false, ArrayQualForm::StaticQualSize);
    bindAbs({ s_direct_abstract_declarator, s_open_bracket, s_type_qualifier_list, s_static, s_const_exp,
                    s_close_bracket },
            true, ArrayQualForm::QualStaticSize);
    bindAbs({ s_open_bracket, s_type_qualifier_list, s_static, s_const_exp, s_close_bracket },
            false, ArrayQualForm::QualStaticSize);

    nodeCreatorRegistry[s_pointer][{ grammar.symbolId("*"), s_type_qualifier_list }] = qualifiedPointer;
    nodeCreatorRegistry[s_pointer][{ grammar.symbolId("*") }] = pointer;
    nodeCreatorRegistry[s_pointer][{ grammar.symbolId("*"), s_type_qualifier_list, s_pointer }] = qualifiedPointerToPointer;
    nodeCreatorRegistry[s_pointer][{ grammar.symbolId("*"), s_pointer }] = pointerToPointer;
}

} // namespace csnb
} // namespace ast
