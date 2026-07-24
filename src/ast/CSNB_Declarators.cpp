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

void arrayDeclarator(AbstractSyntaxTreeBuilderContext& context) {
    context.popTerminal();
    context.popTerminal();
    context.pushDirectDeclarator(std::make_unique<ArrayDeclarator>(context.popDirectDeclarator(), context.popExpression()));
}

void abstractArrayDeclarator(AbstractSyntaxTreeBuilderContext& context) {
    context.popTerminal(); // ]
    context.popTerminal(); // [
    // Incomplete array declarator: T name[]
    context.pushDirectDeclarator(std::make_unique<ArrayDeclarator>(context.popDirectDeclarator(), nullptr));
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
    context.pushFormalArgument(FormalArgument { context.popDeclarationSpecifiers(), context.popDeclarator() });
}

void abstractParameterDeclaration(AbstractSyntaxTreeBuilderContext& context) {
    // Parameter with abstract declarator (e.g. int *, char[20], int [2]).
    // Only consume abstract-declarator pointers, never an outer named declarator's '*'.
    auto specs = context.popDeclarationSpecifiers();
    std::vector<Pointer> pointers;
    if (context.hasAbstractPointers()) {
        pointers = context.popAbstractPointers();
    }
    // Abstract array form: T [N] / T [] / T *[N] / T [N][M] - build anonymous
    // array declarator chain so FormalArgument::getType decays the outermost
    // array to pointer (C 6.7.6.3). Multi-dim must match named form:
    // T a[N][M] -> pointer to array[M] of T (git sha1dc: uint32_t[80][5]).
    if (context.hasAbstractArray()) {
        // Stack top is the rightmost (outermost) bound; collect outer-first.
        std::vector<std::unique_ptr<Expression>> sizesOuterFirst;
        while (context.hasAbstractArray()) {
            sizesOuterFirst.push_back(context.popAbstractArraySize());
        }
        std::unique_ptr<DirectDeclarator> dd = std::make_unique<Identifier>(
                TerminalSymbol { "id", "", translation_unit::Context { "", 0 } });
        // Apply innermost bound first so composition matches named ArrayDeclarator.
        for (auto it = sizesOuterFirst.rbegin(); it != sizesOuterFirst.rend(); ++it) {
            dd = std::make_unique<ArrayDeclarator>(std::move(dd), std::move(*it));
        }
        context.pushFormalArgument(FormalArgument {
                std::move(specs),
                std::make_unique<Declarator>(std::move(dd), std::move(pointers)) });
        return;
    }
    if (!pointers.empty()) {
        context.pushFormalArgument(FormalArgument {
                std::move(specs),
                std::make_unique<Declarator>(
                        std::make_unique<Identifier>(
                                TerminalSymbol { "id", "", translation_unit::Context { "", 0 } }),
                        std::move(pointers)) });
        return;
    }
    context.pushFormalArgument(FormalArgument { std::move(specs) });
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
void sealAbstractPointers(AbstractSyntaxTreeBuilderContext& context) {
    if (context.hasPointers()) {
        context.pushAbstractPointers(context.popPointers());
    } else {
        context.pushAbstractPointers({});
    }
}

void abstractPointerDeclarator(AbstractSyntaxTreeBuilderContext& context) {
    sealAbstractPointers(context);
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

    nodeCreatorRegistry[s_direct_declarator][{ s_identifier }] = identifierDeclarator;
    nodeCreatorRegistry[s_direct_declarator][{ s_open_paren, s_declarator, s_close_paren }] = parenthesizedDeclarator;
    nodeCreatorRegistry[s_direct_declarator][{ s_direct_declarator, s_open_bracket, grammar.symbolId("<const_exp>"), s_close_bracket }] = arrayDeclarator;
    nodeCreatorRegistry[s_direct_declarator][{ s_direct_declarator, s_open_bracket, s_close_bracket }] = abstractArrayDeclarator;
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
    nodeCreatorRegistry[s_abstract_declarator][{ s_pointer }] = abstractPointerDeclarator;
    nodeCreatorRegistry[s_abstract_declarator][{ s_pointer, s_direct_abstract_declarator }] = abstractPointerDeclarator;
    nodeCreatorRegistry[s_abstract_declarator][{ s_direct_abstract_declarator }] =
        [](AbstractSyntaxTreeBuilderContext& context) {
            // Nested form '(' abstract ')' may already have sealed pointers; keep them.
            if (!context.hasAbstractPointers()) {
                context.pushAbstractPointers({});
            }
        };
    // Minimal direct-abstract-declarator support (parens / empty array / sized array / empty param list).
    nodeCreatorRegistry[s_direct_abstract_declarator][{ s_open_paren, s_abstract_declarator, s_close_paren }] =
        [](AbstractSyntaxTreeBuilderContext& context) {
            context.popTerminal(); // )
            context.popTerminal(); // (
        };
    nodeCreatorRegistry[s_direct_abstract_declarator][{ s_open_bracket, s_close_bracket }] =
        [](AbstractSyntaxTreeBuilderContext& context) {
            context.popTerminal();
            context.popTerminal();
            context.pushAbstractArraySize(nullptr);
        };
    nodeCreatorRegistry[s_direct_abstract_declarator][{ s_open_paren, s_close_paren }] =
        [](AbstractSyntaxTreeBuilderContext& context) {
            context.popTerminal();
            context.popTerminal();
        };
    nodeCreatorRegistry[s_direct_abstract_declarator][{ s_direct_abstract_declarator, s_open_bracket, s_close_bracket }] =
        [](AbstractSyntaxTreeBuilderContext& context) {
            context.popTerminal();
            context.popTerminal();
            context.pushAbstractArraySize(nullptr);
        };
    nodeCreatorRegistry[s_direct_abstract_declarator][{ s_direct_abstract_declarator, s_open_paren, s_close_paren }] =
        [](AbstractSyntaxTreeBuilderContext& context) {
            context.popTerminal();
            context.popTerminal();
        };
    nodeCreatorRegistry[s_direct_abstract_declarator][
            { s_direct_abstract_declarator, s_open_bracket, grammar.symbolId("<const_exp>"), s_close_bracket }] =
        [](AbstractSyntaxTreeBuilderContext& context) {
            context.popTerminal(); // ]
            context.pushAbstractArraySize(context.popExpression());
            context.popTerminal(); // [
        };
    nodeCreatorRegistry[s_direct_abstract_declarator][{ s_open_bracket, grammar.symbolId("<const_exp>"), s_close_bracket }] =
        [](AbstractSyntaxTreeBuilderContext& context) {
            context.popTerminal(); // ]
            context.pushAbstractArraySize(context.popExpression());
            context.popTerminal(); // [
        };
    nodeCreatorRegistry[s_direct_abstract_declarator][
            { s_direct_abstract_declarator, s_open_paren, grammar.symbolId("<param_type_list>"), s_close_paren }] =
        [](AbstractSyntaxTreeBuilderContext& context) {
            context.popTerminal(); // )
            context.popArgumentsDeclaration();
            context.popTerminal(); // (
        };
    nodeCreatorRegistry[s_direct_abstract_declarator][{ s_open_paren, grammar.symbolId("<param_type_list>"), s_close_paren }] =
        [](AbstractSyntaxTreeBuilderContext& context) {
            context.popTerminal(); // )
            context.popArgumentsDeclaration();
            context.popTerminal(); // (
        };

    int s_type_qualifier_list = grammar.symbolId("<type_qualifier_list>");
    nodeCreatorRegistry[s_pointer][{ grammar.symbolId("*"), s_type_qualifier_list }] = qualifiedPointer;
    nodeCreatorRegistry[s_pointer][{ grammar.symbolId("*") }] = pointer;
    nodeCreatorRegistry[s_pointer][{ grammar.symbolId("*"), s_type_qualifier_list, s_pointer }] = qualifiedPointerToPointer;
    nodeCreatorRegistry[s_pointer][{ grammar.symbolId("*"), s_pointer }] = pointerToPointer;
}

} // namespace csnb
} // namespace ast
