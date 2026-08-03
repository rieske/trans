#include "CSNB_Internal.h"
#include <limits>
#include <memory>

#include "DeclarationSpecifiers.h"
#include "Declarator.h"
#include "Expression.h"
#include "StorageSpecifier.h"
#include "TypeName.h"
#include "TypeSpecifier.h"
#include "types/Type.h"

namespace ast {
namespace csnb {

namespace {

int foldBitFieldWidth(AbstractSyntaxTreeBuilderContext& context) {
    auto widthExpr = context.popExpression();
    context.popTerminal();
    long width = 0;
    if (!widthExpr || !widthExpr->foldToHostLong(width) || width < 0
            || width > static_cast<long>(std::numeric_limits<int>::max())) {
        throw std::runtime_error { "bit-field width is not a constant" };
    }
    return static_cast<int>(width);
}

void completeRecordFromSpec(AbstractSyntaxTreeBuilderContext& context, type::Type& record,
        std::vector<type::MemberSpec> members, bool isUnion) {
    const bool packed = context.environment().session().recordPacked.consume();
    if (isUnion) {
        type::completeUnion(record, std::move(members), packed);
    } else {
        type::completeStructure(record, std::move(members), packed);
    }
}

void shortType(AbstractSyntaxTreeBuilderContext& context) {
    // short is a distinct 2-byte type (C ABI / ctype tables).
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
    context.pushTypeSpecifier( { type::doubleFloating(), context.popTerminal().value });
}

void complexType(AbstractSyntaxTreeBuilderContext& context) {
    context.pushTypeSpecifier( { type::complexDouble(), context.popTerminal().value });
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
}

void constQualifier(AbstractSyntaxTreeBuilderContext& context) {
    context.popTerminal(); // const
    context.pushTypeQualifier(type::Qualifier::CONST);
}

void volatileQualifier(AbstractSyntaxTreeBuilderContext& context) {
    context.popTerminal(); // volatile
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
void typeSpecTypeofTypeName(AbstractSyntaxTreeBuilderContext& context) {
    context.popTerminal(); // )
    context.popTerminal(); // (
    context.popTerminal(); // typeof
    auto typeName = context.popTypeName();
    TypeSpecifier spec { std::move(typeName) };
    spec.dropSpelling();
    context.pushTypeSpecifier(std::move(spec));
}

void typeSpecTypeofExpression(AbstractSyntaxTreeBuilderContext& context) {
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
DeclarationSpecifiers popSpecQualifiers(AbstractSyntaxTreeBuilderContext& context) {
    auto specs = context.popDeclarationSpecifiers();
    specs.resolveTypeofAtParseTime(context.environment());
    if (specs.getTypeSpecifiers().empty()) {
        throw std::runtime_error { "cannot determine type of spec-qualifier-list" };
    }
    return specs;
}

DeclarationSpecifiers popResolvedSpecQualifiers(AbstractSyntaxTreeBuilderContext& context) {
    auto specs = popSpecQualifiers(context);
    if (specs.needsSemanticResolve()) {
        throw std::runtime_error { "cannot determine type of typeof operand" };
    }
    return specs;
}

void typeNameFromSpecifier(AbstractSyntaxTreeBuilderContext& context) {
    context.pushTypeName(TypeName { popSpecQualifiers(context).toTypeSpecifier(), nullptr });
}

void typeNameWithAbstractDeclarator(AbstractSyntaxTreeBuilderContext& context) {
    auto declarator = context.popDeclarator();
    auto typeSpec = popSpecQualifiers(context).toTypeSpecifier();
    context.pushTypeName(TypeName { std::move(typeSpec), std::move(declarator) });
}

} // namespace


void registerTypeProductions(const parser::Grammar& grammar, NodeCreatorRegistry& nodeCreatorRegistry) {
    int s_type_specifier = grammar.symbolId("<type_spec>");
    nodeCreatorRegistry[s_type_specifier][{ grammar.symbolId("short") }] = shortType;
    nodeCreatorRegistry[s_type_specifier][{ grammar.symbolId("int") }] = integerType;
    nodeCreatorRegistry[s_type_specifier][{ grammar.symbolId("long") }] = longType;
    nodeCreatorRegistry[s_type_specifier][{ grammar.symbolId("char") }] = characterType;
    nodeCreatorRegistry[s_type_specifier][{ grammar.symbolId("bool") }] = boolType;
    nodeCreatorRegistry[s_type_specifier][{ grammar.symbolId("void") }] = voidType;
    nodeCreatorRegistry[s_type_specifier][{ grammar.symbolId("float") }] = floatType;
    nodeCreatorRegistry[s_type_specifier][{ grammar.symbolId("double") }] = doubleType;
    nodeCreatorRegistry[s_type_specifier][{ grammar.symbolId("_Complex") }] = complexType;
    nodeCreatorRegistry[s_type_specifier][{ grammar.symbolId("signed") }] = signedType;
    nodeCreatorRegistry[s_type_specifier][{ grammar.symbolId("unsigned") }] = unsignedType;
    nodeCreatorRegistry[s_type_specifier][{ grammar.symbolId("typedef_name") }] = typedefName;
    nodeCreatorRegistry[s_type_specifier][{ grammar.symbolId("<struct_or_union_spec>") }] = structOrUnionType;
    nodeCreatorRegistry[s_type_specifier][{ grammar.symbolId("<enum_spec>") }] = enumType;
    nodeCreatorRegistry[s_type_specifier][{
            grammar.symbolId("typeof"), grammar.symbolId("("),
            grammar.symbolId("<type_name>"), grammar.symbolId(")") }] = typeSpecTypeofTypeName;
    nodeCreatorRegistry[s_type_specifier][{
            grammar.symbolId("typeof"), grammar.symbolId("("),
            grammar.symbolId("<assignment_exp>"), grammar.symbolId(")") }] = typeSpecTypeofExpression;

    int s_type_qualifier = grammar.symbolId("<type_qualifier>");
    nodeCreatorRegistry[s_type_qualifier][{ grammar.symbolId("const") }] = constQualifier;
    nodeCreatorRegistry[s_type_qualifier][{ grammar.symbolId("volatile") }] = volatileQualifier;
    nodeCreatorRegistry[s_type_qualifier][{ grammar.symbolId("restrict") }] = restrictQualifier;

    int s_function_spec = grammar.symbolId("<function_spec>");
    nodeCreatorRegistry[s_function_spec][{ grammar.symbolId("inline") }] = functionSpecifier;
    nodeCreatorRegistry[s_function_spec][{ grammar.symbolId("noreturn") }] = functionSpecifier;

    int s_storage_class_spec = grammar.symbolId("<storage_class_spec>");
    nodeCreatorRegistry[s_storage_class_spec][{ grammar.symbolId("auto") }] = [](AbstractSyntaxTreeBuilderContext& context) {
        context.pushStorageSpecifier(StorageSpecifier::AUTO(context.popTerminal().context));
    };
    nodeCreatorRegistry[s_storage_class_spec][{ grammar.symbolId("register") }] = [](AbstractSyntaxTreeBuilderContext& context) {
        context.pushStorageSpecifier(StorageSpecifier::REGISTER(context.popTerminal().context));
    };
    nodeCreatorRegistry[s_storage_class_spec][{ grammar.symbolId("static") }] = [](AbstractSyntaxTreeBuilderContext& context) {
        context.pushStorageSpecifier(StorageSpecifier::STATIC(context.popTerminal().context));
    };
    nodeCreatorRegistry[s_storage_class_spec][{ grammar.symbolId("extern") }] = [](AbstractSyntaxTreeBuilderContext& context) {
        context.pushStorageSpecifier(StorageSpecifier::EXTERN(context.popTerminal().context));
    };
    nodeCreatorRegistry[s_storage_class_spec][{ grammar.symbolId("typedef") }] = [](AbstractSyntaxTreeBuilderContext& context) {
        context.pushStorageSpecifier(StorageSpecifier::TYPEDEF(context.popTerminal().context));
    };

    int s_decl_specs = grammar.symbolId("<decl_specs>");
    nodeCreatorRegistry[s_decl_specs][{ s_type_specifier }] = declarationTypeSpecifier;
    nodeCreatorRegistry[s_decl_specs][{ s_type_specifier, s_decl_specs }] = addDeclarationTypeSpecifier;
    nodeCreatorRegistry[s_decl_specs][{ s_storage_class_spec }] = declarationStorageClassSpecifier;
    nodeCreatorRegistry[s_decl_specs][{ s_storage_class_spec, s_decl_specs }] = addDeclarationStorageClassSpecifier;
    nodeCreatorRegistry[s_decl_specs][{ s_type_qualifier }] = declarationTypeQualifier;
    nodeCreatorRegistry[s_decl_specs][{ s_type_qualifier, s_decl_specs }] = addDeclarationTypeQualifier;
    nodeCreatorRegistry[s_decl_specs][{ s_function_spec }] = functionSpecifierOnly;
    nodeCreatorRegistry[s_decl_specs][{ s_function_spec, s_decl_specs }] = doNothing;

    int s_type_qualifier_list = grammar.symbolId("<type_qualifier_list>");
    nodeCreatorRegistry[s_type_qualifier_list][{ s_type_qualifier }] = typeQualifierList;
    nodeCreatorRegistry[s_type_qualifier_list][{ s_type_qualifier_list, s_type_qualifier }] = addTypeQualifierToList;

    int s_spec_qualifier_list = grammar.symbolId("<spec_qualifier_list>");
    int s_type_name = grammar.symbolId("<type_name>");
    int s_abstract_declarator = grammar.symbolId("<abstract_declarator>");
    nodeCreatorRegistry[s_type_name][{ s_spec_qualifier_list }] = typeNameFromSpecifier;
    nodeCreatorRegistry[s_type_name][{ s_spec_qualifier_list, s_abstract_declarator }] =
            typeNameWithAbstractDeclarator;

    // Same accumulator as decl_specs: type specs + cv (no storage class).
    nodeCreatorRegistry[s_spec_qualifier_list][{ s_type_specifier }] = declarationTypeSpecifier;
    nodeCreatorRegistry[s_spec_qualifier_list][{ s_type_specifier, s_spec_qualifier_list }] =
            addDeclarationTypeSpecifier;
    nodeCreatorRegistry[s_spec_qualifier_list][{ s_type_qualifier }] = declarationTypeQualifier;
    nodeCreatorRegistry[s_spec_qualifier_list][{ s_type_qualifier, s_spec_qualifier_list }] =
            addDeclarationTypeQualifier;

    int s_struct_or_union = grammar.symbolId("<struct_or_union>");
    int s_struct_or_union_spec = grammar.symbolId("<struct_or_union_spec>");
    int s_struct_decl_list = grammar.symbolId("<struct_decl_list>");
    int s_struct_decl = grammar.symbolId("<struct_decl>");
    int s_struct_declarator_list = grammar.symbolId("<struct_declarator_list>");
    int s_struct_declarator = grammar.symbolId("<struct_declarator>");
    int s_identifier = grammar.symbolId("id");
    int s_comma = grammar.symbolId(",");
    int s_semicolon = grammar.symbolId(";");
    int s_declarator = grammar.symbolId("<declarator>");

    int s_enum_spec = grammar.symbolId("<enum_spec>");
    int s_enumerator_list = grammar.symbolId("<enumerator_list>");
    int s_enumerator = grammar.symbolId("<enumerator>");
    int s_enum_kw = grammar.symbolId("enum");
    int s_const_exp = grammar.symbolId("<const_exp>");

    nodeCreatorRegistry[s_enumerator][{ s_identifier }] = [](AbstractSyntaxTreeBuilderContext& context) {
        auto id = context.popTerminal();
        context.environment().addEnumerator(id.value);
    };
    nodeCreatorRegistry[s_enumerator][{ s_identifier, grammar.symbolId("="), s_const_exp }] =
        [](AbstractSyntaxTreeBuilderContext& context) {
            auto expr = context.popExpression();
            context.popTerminal(); // =
            auto id = context.popTerminal();
            type::IntegerConstant ice;
            if (!expr->evaluateConstant(ice)) {
                throw std::runtime_error { "enumerator initializer is not a constant expression: " + id.value };
            }
            context.environment().addEnumerator(id.value, std::move(ice));
        };
    nodeCreatorRegistry[s_enumerator_list][{ s_enumerator }] = doNothing;
    nodeCreatorRegistry[s_enumerator_list][{ s_enumerator_list, s_comma, s_enumerator }] =
        [](AbstractSyntaxTreeBuilderContext& context) { context.popTerminal(); };
    // C99 trailing comma after last enumerator.
    nodeCreatorRegistry[s_enumerator_list][{ s_enumerator_list, s_comma }] =
        [](AbstractSyntaxTreeBuilderContext& context) { context.popTerminal(); };

    // Enum types are integer stand-ins; underlying width follows enumerator range (GCC/SysV).
    nodeCreatorRegistry[s_enum_spec][{ s_enum_kw, s_identifier, grammar.symbolId("{"), s_enumerator_list, grammar.symbolId("}") }] =
        [](AbstractSyntaxTreeBuilderContext& context) {
            context.popTerminal(); // }
            context.popTerminal(); // {
            auto tag = context.popTerminal();
            context.popTerminal(); // enum
            type::Type underlying = context.environment().endEnumDefinition(tag.value);
            context.pushTypeSpecifier(TypeSpecifier { underlying, tag.value });
        };

    nodeCreatorRegistry[s_enum_spec][{ s_enum_kw, grammar.symbolId("{"), s_enumerator_list, grammar.symbolId("}") }] =
        [](AbstractSyntaxTreeBuilderContext& context) {
            context.popTerminal(); // }
            context.popTerminal(); // {
            context.popTerminal(); // enum
            type::Type underlying = context.environment().endEnumDefinition();
            context.pushTypeSpecifier(TypeSpecifier { underlying, "" });
        };
    nodeCreatorRegistry[s_enum_spec][{ s_enum_kw, s_identifier }] =
        [](AbstractSyntaxTreeBuilderContext& context) {
            auto tag = context.popTerminal();
            context.popTerminal(); // enum
            type::Type underlying = context.environment().lookupEnumTag(tag.value)
                    .value_or(type::signedInteger());
            context.pushTypeSpecifier(TypeSpecifier { underlying, tag.value });
        };

    // Push a fresh member-list frame so nested struct/union definitions do not
    // append their fields onto the enclosing type's member list.
    nodeCreatorRegistry[s_struct_or_union][{ grammar.symbolId("struct") }] = [](AbstractSyntaxTreeBuilderContext& context) {
        context.popTerminal();
        context.environment().session().recordPacked.begin();
        context.pushIsUnion(false);
        context.newStructMemberList();
    };
    nodeCreatorRegistry[s_struct_or_union][{ grammar.symbolId("union") }] = [](AbstractSyntaxTreeBuilderContext& context) {
        context.popTerminal();
        context.environment().session().recordPacked.begin();
        context.pushIsUnion(true);
        context.newStructMemberList();
    };

    nodeCreatorRegistry[s_struct_or_union_spec][{ s_struct_or_union, s_identifier, grammar.symbolId("{"), s_struct_decl_list, grammar.symbolId("}") }] =
        [](AbstractSyntaxTreeBuilderContext& context) {
            context.popTerminal();
            context.popTerminal();
            auto tag = context.popTerminal();
            auto members = context.popStructMemberList();
            bool isUnion = context.popIsUnion();
            // Same Type instance as any earlier incomplete references (e.g. self-pointers).
            type::Type tagType = context.environment().ensureStructTag(tag.value);
            completeRecordFromSpec(context, tagType, std::move(members), isUnion);
            TypeSpecifier spec { tagType, tag.value };
            spec.markDefinesRecord();
            context.pushTypeSpecifier(std::move(spec));
        };
    nodeCreatorRegistry[s_struct_or_union_spec][{ s_struct_or_union, grammar.symbolId("{"), s_struct_decl_list, grammar.symbolId("}") }] =
        [](AbstractSyntaxTreeBuilderContext& context) {
            context.popTerminal();
            context.popTerminal();
            auto members = context.popStructMemberList();
            bool isUnion = context.popIsUnion();
            type::Type completed = type::incompleteRecord();
            completeRecordFromSpec(context, completed, std::move(members), isUnion);
            TypeSpecifier spec { completed, "" };
            spec.markDefinesRecord();
            context.pushTypeSpecifier(std::move(spec));
        };
    nodeCreatorRegistry[s_struct_or_union_spec][{ s_struct_or_union, s_identifier }] =
        [](AbstractSyntaxTreeBuilderContext& context) {
            auto tag = context.popTerminal();
            context.popIsUnion(); // tag reference; layout decided at definition
            // struct_or_union pushed a member-list frame; no body here, so discard it.
            context.popStructMemberList();
            context.environment().session().recordPacked.abandon();
            // Incomplete until a defining `struct/union Tag { ... }` completes the shared body.
            context.pushTypeSpecifier(TypeSpecifier { context.environment().ensureStructTag(tag.value), tag.value });
        };

    nodeCreatorRegistry[s_struct_declarator][{ s_declarator }] = [](AbstractSyntaxTreeBuilderContext& context) {
        context.addStructDeclarator(context.popDeclarator());
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
            context.popTerminal();
            auto declarators = context.popStructDeclarators();
            auto baseType = popResolvedSpecQualifiers(context).getResolvedType();
            for (auto& [declarator, bitWidth] : declarators) {
                if (!declarator) {
                    context.addStructMember("", baseType, bitWidth);
                    continue;
                }
                std::string name = declarator->getName();
                context.addStructMember(name, declarator->getFundamentalType(baseType), bitWidth);
            }
        };
    // C11 anonymous struct/union member: keep as one empty-name nested aggregate so
    // layout (union overlay vs sequential fields) is preserved; member lookup walks it.
    nodeCreatorRegistry[s_struct_decl][{ s_spec_qualifier_list, s_semicolon }] =
        [](AbstractSyntaxTreeBuilderContext& context) {
            context.popTerminal(); // ;
            auto specs = popResolvedSpecQualifiers(context);
            if (specs.isUntaggedRecordBody()) {
                context.addStructMember("", specs.getResolvedType());
            }
        };
    nodeCreatorRegistry[s_struct_decl_list][{ s_struct_decl }] = doNothing;
    nodeCreatorRegistry[s_struct_decl_list][{ s_struct_decl_list, s_struct_decl }] = doNothing;
}

} // namespace csnb
} // namespace ast
