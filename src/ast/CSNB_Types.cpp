#include "CSNB_Internal.h"

#include "DeclarationSpecifiers.h"
#include "Declarator.h"
#include "StorageSpecifier.h"
#include "TypeSpecifier.h"
#include "types/Type.h"
#include "util/ProductApprox.h"

namespace ast {
namespace csnb {

namespace {

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
void typeSpecTypeof(AbstractSyntaxTreeBuilderContext& context) {
    // type_spec -> __typeof__ ( unary_exp )
    // Approximate as unsigned long (product contract) but keep the operand so
    // offsetof(__typeof__(*p), m) can use the pointer form.
    context.popTerminal(); // )
    context.popTerminal(); // (
    auto operand = context.popExpression();
    auto typeofKw = context.popTerminal(); // __typeof__
    context.pushTypeofOperand(std::move(operand));
    context.pushTypeSpecifier(TypeSpecifier { type::unsignedLong(), "__typeof__" });
    (void)typeofKw;
}
void typeNameFromSpecifier(AbstractSyntaxTreeBuilderContext& context) {
    // type_name -> spec_qualifier_list: TypeSpecifier already on stack from type_spec.
}

// Multi-word type-name pieces (`unsigned` + `long`) must collapse to one TypeSpecifier
// so nested casts do not leave a leftover type on the builder stack (git container_of).
// Right-hand list may be only qualifiers (`int const`, `unsigned char const *`) with
// no TypeSpecifier left after the qualifier production pops them.
void combineSpecQualifierTypeSpecs(AbstractSyntaxTreeBuilderContext& context) {
    // Right-recursive: stack top is the left type_spec, below is the right-hand list.
    auto left = context.popTypeSpecifier();
    if (!context.hasTypeSpecifier()) {
        context.pushTypeSpecifier(left);
        return;
    }
    auto right = context.popTypeSpecifier();
    DeclarationSpecifiers combined { left, DeclarationSpecifiers { right } };
    type::Type resolved = combined.getResolvedType();
    context.pushTypeSpecifier(TypeSpecifier { resolved, left.getName() + " " + right.getName() });
}

void typeNamePointer(AbstractSyntaxTreeBuilderContext& context) {
    // type_name -> spec_qualifier_list abstract_declarator
    auto typeSpec = context.popTypeSpecifier();
    type::Type t = typeSpec.getType();
    if (context.hasAbstractPointers()) {
        auto pointers = context.popAbstractPointers();
        for (const auto& pointer : pointers) {
            t = type::pointer(t, pointer.getQualifiers());
        }
    }
    // Abstract array in a type-name (e.g. sizeof(int[4])) - apply array after pointers.
    if (context.hasAbstractArray()) {
        auto sizeExpr = context.popAbstractArraySize();
        while (context.hasAbstractArray()) {
            context.popAbstractArraySize();
        }
        long length = 0;
        if (!sizeExpr || !sizeExpr->evaluateConstant(length)) {
            length = 0;
        } else if (length < 0) {
            // BUILD_ASSERT_OR_ZERO negative sizeof(char[N]) path.
            // Contract: ProductContracts.buildAssertOrZeroNegativeSizeofContributesZero.
            length = product_approx::clampNegativeArrayBoundForBuildAssert();
        }
        t = type::array(t, static_cast<int>(length));
    }
    context.pushTypeSpecifier(TypeSpecifier { t, typeSpec.getName() });
}

} // namespace


void registerTypeProductions(const parser::Grammar& grammar, NodeCreatorRegistry& nodeCreatorRegistry) {
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
    nodeCreatorRegistry[s_type_specifier][{ grammar.symbolId("typedef_name") }] = typedefName;
    nodeCreatorRegistry[s_type_specifier][{ grammar.symbolId("<struct_or_union_spec>") }] = structOrUnionType;
    nodeCreatorRegistry[s_type_specifier][{ grammar.symbolId("<enum_spec>") }] = enumType;
    nodeCreatorRegistry[s_type_specifier][{
            grammar.symbolId("__typeof__"), grammar.symbolId("("),
            grammar.symbolId("<unary_exp>"), grammar.symbolId(")") }] = typeSpecTypeof;

    int s_type_qualifier = grammar.symbolId("<type_qualifier>");
    nodeCreatorRegistry[s_type_qualifier][{ grammar.symbolId("const") }] = constQualifier;
    nodeCreatorRegistry[s_type_qualifier][{ grammar.symbolId("volatile") }] = volatileQualifier;

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

    int s_type_qualifier_list = grammar.symbolId("<type_qualifier_list>");
    nodeCreatorRegistry[s_type_qualifier_list][{ s_type_qualifier }] = typeQualifierList;
    nodeCreatorRegistry[s_type_qualifier_list][{ s_type_qualifier_list, s_type_qualifier }] = addTypeQualifierToList;

    int s_spec_qualifier_list = grammar.symbolId("<spec_qualifier_list>");
    int s_type_name = grammar.symbolId("<type_name>");
    int s_abstract_declarator = grammar.symbolId("<abstract_declarator>");
    nodeCreatorRegistry[s_type_name][{ s_spec_qualifier_list }] = typeNameFromSpecifier;
    nodeCreatorRegistry[s_type_name][{ s_spec_qualifier_list, s_abstract_declarator }] = typeNamePointer;

    nodeCreatorRegistry[s_spec_qualifier_list][{ s_type_specifier }] = doNothing;
    nodeCreatorRegistry[s_spec_qualifier_list][{ s_type_specifier, s_spec_qualifier_list }] = combineSpecQualifierTypeSpecs;
    nodeCreatorRegistry[s_spec_qualifier_list][{ s_type_qualifier }] = [](AbstractSyntaxTreeBuilderContext& context) { context.popTypeQualifier(); };
    nodeCreatorRegistry[s_spec_qualifier_list][{ s_type_qualifier, s_spec_qualifier_list }] = [](AbstractSyntaxTreeBuilderContext& context) { context.popTypeQualifier(); };

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
            long value = 0;
            if (!expr->evaluateConstant(value)) {
                throw std::runtime_error { "enumerator initializer is not a constant expression: " + id.value };
            }
            context.environment().addEnumerator(id.value, value);
        };
    nodeCreatorRegistry[s_enumerator_list][{ s_enumerator }] = doNothing;
    nodeCreatorRegistry[s_enumerator_list][{ s_enumerator_list, s_comma, s_enumerator }] =
        [](AbstractSyntaxTreeBuilderContext& context) { context.popTerminal(); };
    // C99 trailing comma after last enumerator.
    nodeCreatorRegistry[s_enumerator_list][{ s_enumerator_list, s_comma }] =
        [](AbstractSyntaxTreeBuilderContext& context) { context.popTerminal(); };

    nodeCreatorRegistry[s_enum_spec][{ s_enum_kw, s_identifier, grammar.symbolId("{"), s_enumerator_list, grammar.symbolId("}") }] =
        [](AbstractSyntaxTreeBuilderContext& context) {
            context.popTerminal(); // }
            context.popTerminal(); // {
            // Enumerators were accumulated while reducing enumerator_list; begin was implicit on first add.
            // Ensure a definition frame exists even for empty lists (not allowed by grammar but safe).
            auto tag = context.popTerminal();
            context.popTerminal(); // enum
            auto enumerators = context.environment().endEnumDefinition();
            // If begin was never called (empty), end returns empty; restart for safety.
            context.pushTypeSpecifier(TypeSpecifier { type::signedInteger(), tag.value, std::move(enumerators) });
        };
    // beginEnumDefinition must run before enumerators are reduced. Use a production-start hook:
    // when we see enum id {, we cannot easily hook mid-parse. Instead begin on first enumerator
    // via addEnumerator. But endEnumDefinition pops the frame - if addEnumerator called begin,
    // we're fine. For empty enumerator_list grammar doesn't allow empty. Good.

    nodeCreatorRegistry[s_enum_spec][{ s_enum_kw, grammar.symbolId("{"), s_enumerator_list, grammar.symbolId("}") }] =
        [](AbstractSyntaxTreeBuilderContext& context) {
            context.popTerminal(); // }
            context.popTerminal(); // {
            context.popTerminal(); // enum
            auto enumerators = context.environment().endEnumDefinition();
            context.pushTypeSpecifier(TypeSpecifier { type::signedInteger(), "", std::move(enumerators) });
        };
    nodeCreatorRegistry[s_enum_spec][{ s_enum_kw, s_identifier }] =
        [](AbstractSyntaxTreeBuilderContext& context) {
            auto tag = context.popTerminal();
            context.popTerminal(); // enum
            context.pushTypeSpecifier(TypeSpecifier { type::signedInteger(), tag.value });
        };

    // Push a fresh member-list frame so nested struct/union definitions do not
    // append their fields onto the enclosing type's member list.
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

    nodeCreatorRegistry[s_struct_or_union_spec][{ s_struct_or_union, s_identifier, grammar.symbolId("{"), s_struct_decl_list, grammar.symbolId("}") }] =
        [](AbstractSyntaxTreeBuilderContext& context) {
            context.popTerminal();
            context.popTerminal();
            auto tag = context.popTerminal();
            auto members = context.popStructMemberList();
            auto pending = context.popPendingMemberDeclarators();
            bool isUnion = context.popIsUnion();
            // Same Type instance as any earlier incomplete references (e.g. self-pointers).
            type::Type tagType = context.environment().ensureStructTag(tag.value);
            if (isUnion) {
                type::completeUnion(tagType, std::move(members));
            } else {
                type::completeStructure(tagType, std::move(members));
            }
            // Pending ARRAY_SIZE/sizeof bounds re-folded once after file-scope symbols exist.
            for (auto& p : pending) {
                if (!p.declarator) {
                    continue;
                }
                context.environment().pendingArrayMembers().add(
                        tagType,
                        PendingArrayMember {
                                std::move(p.name), std::move(p.baseType),
                                std::shared_ptr<Declarator>(p.declarator.release()) });
            }
            context.pushTypeSpecifier(TypeSpecifier { tagType, tag.value });
        };
    nodeCreatorRegistry[s_struct_or_union_spec][{ s_struct_or_union, grammar.symbolId("{"), s_struct_decl_list, grammar.symbolId("}") }] =
        [](AbstractSyntaxTreeBuilderContext& context) {
            context.popTerminal();
            context.popTerminal();
            auto members = context.popStructMemberList();
            auto pending = context.popPendingMemberDeclarators();
            bool isUnion = context.popIsUnion();
            type::Type completed = isUnion
                    ? type::unionType(std::move(members))
                    : type::structure(std::move(members));
            for (auto& p : pending) {
                if (!p.declarator) {
                    continue;
                }
                context.environment().pendingArrayMembers().add(
                        completed,
                        PendingArrayMember {
                                std::move(p.name), std::move(p.baseType),
                                std::shared_ptr<Declarator>(p.declarator.release()) });
            }
            context.pushTypeSpecifier(TypeSpecifier { completed, "" });
        };
    nodeCreatorRegistry[s_struct_or_union_spec][{ s_struct_or_union, s_identifier }] =
        [](AbstractSyntaxTreeBuilderContext& context) {
            auto tag = context.popTerminal();
            context.popIsUnion(); // tag reference; layout decided at definition
            // struct_or_union pushed a member-list frame; no body here, so discard it.
            context.popStructMemberList();
            context.popPendingMemberDeclarators();
            // Incomplete until a defining `struct/union Tag { ... }` completes the shared body.
            context.pushTypeSpecifier(TypeSpecifier { context.environment().ensureStructTag(tag.value), tag.value });
        };

    nodeCreatorRegistry[s_struct_declarator][{ s_declarator }] = [](AbstractSyntaxTreeBuilderContext& context) {
        context.addStructDeclarator(context.popDeclarator());
    };
    // Bit-fields: accept syntax and treat named fields as ordinary members (width ignored).
    nodeCreatorRegistry[s_struct_declarator][{ s_declarator, grammar.symbolId(":"), grammar.symbolId("<const_exp>") }] =
        [](AbstractSyntaxTreeBuilderContext& context) {
            context.popExpression(); // bit width
            context.popTerminal(); // :
            context.addStructDeclarator(context.popDeclarator());
        };
    // Anonymous padding bit-field (`int : 32;`) - no member.
    nodeCreatorRegistry[s_struct_declarator][{ grammar.symbolId(":"), grammar.symbolId("<const_exp>") }] =
        [](AbstractSyntaxTreeBuilderContext& context) {
            context.popExpression();
            context.popTerminal(); // :
        };
    nodeCreatorRegistry[s_struct_declarator_list][{ s_struct_declarator }] = doNothing;
    nodeCreatorRegistry[s_struct_declarator_list][{ s_struct_declarator_list, s_comma, s_struct_declarator }] =
        [](AbstractSyntaxTreeBuilderContext& context) { context.popTerminal(); };

    nodeCreatorRegistry[s_struct_decl][{ s_spec_qualifier_list, s_struct_declarator_list, s_semicolon }] =
        [](AbstractSyntaxTreeBuilderContext& context) {
            context.popTerminal();
            auto declarators = context.popStructDeclarators();
            auto typeSpec = context.popTypeSpecifier();
            auto baseType = typeSpec.getType();
            for (auto& declarator : declarators) {
                std::string name = declarator->getName();
                context.addStructMember(name, declarator->getFundamentalType(baseType));
                // Keep declarator for semantic re-fold of sizeof/ARRAY_SIZE bounds.
                context.addPendingMemberDeclarator(std::move(name), baseType, std::move(declarator));
            }
        };
    // C11 anonymous struct/union member: keep as one empty-name nested aggregate so
    // layout (union overlay vs sequential fields) is preserved; member lookup walks it.
    nodeCreatorRegistry[s_struct_decl][{ s_spec_qualifier_list, s_semicolon }] =
        [](AbstractSyntaxTreeBuilderContext& context) {
            context.popTerminal(); // ;
            auto typeSpec = context.popTypeSpecifier();
            auto nested = typeSpec.getType();
            if (nested.isRecord() && nested.isCompleteRecord()) {
                context.addStructMember("", nested);
            }
        };
    nodeCreatorRegistry[s_struct_decl_list][{ s_struct_decl }] = doNothing;
    nodeCreatorRegistry[s_struct_decl_list][{ s_struct_decl_list, s_struct_decl }] = doNothing;
}

} // namespace csnb
} // namespace ast
