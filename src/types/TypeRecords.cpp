#include "Type.h"
#include "TypeQuery.h"

namespace type {

Type::Member::Member(std::string n, Type t, int off, std::optional<BitField> bits) :
        name { std::move(n) },
        type { std::make_shared<Type>(std::move(t)) },
        offsetBytes { off },
        bitField { std::move(bits) }
{
}

Type builtinVaListTagType() {
    return structure({
            { "gp_offset", unsignedInteger() },
            { "fp_offset", unsignedInteger() },
            { "overflow_arg_area", pointer(voidType()) },
            { "reg_save_area", pointer(voidType()) },
    });
}

Type builtinVaListType() {
    return array(builtinVaListTagType(), 1);
}

Type incompleteRecord() {
    Type result { std::vector<Qualifier> {} };
    Type::RecordPayload rec;
    rec.body = std::make_shared<Type::StructBody>();
    rec.body->complete = false;
    rec.body->size = 0;
    result._payload = std::move(rec);
    return result;
}

namespace {

std::vector<MemberSpec> specsFromPairs(const std::vector<std::pair<std::string, Type>>& members) {
    std::vector<MemberSpec> specs;
    specs.reserve(members.size());
    for (const auto& [name, memberType] : members) {
        specs.push_back(MemberSpec { name, memberType });
    }
    return specs;
}

} // namespace

Type structure(const std::vector<std::pair<std::string, Type>>& members) {
    Type result = incompleteRecord();
    completeStructure(result, specsFromPairs(members));
    return result;
}

Type unionType(const std::vector<std::pair<std::string, Type>>& members) {
    Type result = incompleteRecord();
    completeUnion(result, specsFromPairs(members));
    return result;
}

bool Type::isRecord() const {
    return recordPayload() != nullptr;
}

bool Type::isStructure() const {
    const auto* b = body();
    return b && !b->isUnion;
}

bool Type::isUnion() const {
    const auto* b = body();
    return b && b->isUnion;
}

bool Type::isTransparentUnion() const {
    const auto* b = body();
    return b && b->isUnion && b->transparentUnion;
}

void Type::markTransparentUnion() {
    auto* b = body();
    if (b && b->isUnion) {
        b->transparentUnion = true;
    }
}

bool Type::isPacked() const {
    const auto* b = body();
    return b && b->packed;
}

void Type::applyPacked() {
    auto* b = body();
    if (!b || b->packed) {
        return;
    }
    if (!b->complete) {
        b->packed = true;
        return;
    }
    relayoutFromMemberSpecs(*this, memberSpecs(*this), true);
}

std::vector<MemberSpec> memberSpecs(const Type& record) {
    std::vector<MemberSpec> specs;
    if (!record.isRecord()) {
        return specs;
    }
    const auto& members = record.getMembers();
    specs.reserve(members.size());
    for (const auto& member : members) {
        std::optional<int> width;
        if (member.bitField) {
            width = member.bitField->width;
        }
        specs.emplace_back(member.name, member.type ? *member.type : voidType(), width);
    }
    return specs;
}

const char* relayoutFromMemberSpecs(Type& record, const std::vector<MemberSpec>& specs, bool packed) {
    const bool transparent = record.isTransparentUnion();
    const char* error = record.isUnion()
            ? completeUnion(record, specs, packed)
            : completeStructure(record, specs, packed);
    if (!error && transparent) {
        record.markTransparentUnion();
    }
    return error;
}

const char* relayoutFromMemberSpecs(Type& record, const std::vector<MemberSpec>& specs) {
    return relayoutFromMemberSpecs(record, specs, record.isPacked());
}

bool Type::isAggregate() const {
    return isArray() || isRecord();
}

bool Type::isCompleteRecord() const {
    const auto* b = body();
    return b && b->complete;
}

bool Type::isIncompleteRecord() const {
    return isRecord() && !isCompleteRecord();
}

const void* Type::structureBodyIdentity() const {
    const auto* b = body();
    return b;
}

const std::vector<Type::Member>& Type::getMembers() const {
    const auto* b = body();
    if (!b) {
        static const std::vector<Member> empty;
        return empty;
    }
    return b->members;
}

std::optional<MemberPath> lookupMemberPath(const Type& record, const std::string& name) {
    const int n = record.memberCount();
    for (int i = 0; i < n; ++i) {
        auto member = memberAt(record, i);
        if (!member) {
            continue;
        }
        if (!member->name.empty() && member->name == name) {
            return MemberPath { std::move(*member), { i } };
        }
        if (member->name.empty() && member->type.isRecord()) {
            if (auto nested = lookupMemberPath(member->type, name)) {
                nested->member.offsetBytes += member->offsetBytes;
                nested->indices.insert(nested->indices.begin(), i);
                return nested;
            }
        }
    }
    return std::nullopt;
}

std::optional<FoundMember> lookupMember(const Type& record, const std::string& name) {
    if (auto path = lookupMemberPath(record, name)) {
        return std::move(path->member);
    }
    return std::nullopt;
}

std::optional<FoundMember> memberAt(const Type& record, int index) {
    if (!record.isRecord() || index < 0 || index >= record.memberCount()) {
        return std::nullopt;
    }
    const auto& member = record.getMembers()[static_cast<std::size_t>(index)];
    return FoundMember {
            member.name,
            member.type ? *member.type : voidType(),
            member.offsetBytes,
            member.bitField };
}

BitField makeBitField(const Type& declared, int width, int shift) {
    BitField bits;
    bits.width = width;
    bits.shift = shift;
    bits.isSigned = valueIsSigned(declared);
    return bits;
}

OffsetofResult resolveOffsetof(const Type& record, const std::string& name) {
    if (!record.isCompleteRecord()) {
        return { OffsetofStatus::Incomplete, 0 };
    }
    const auto found = lookupMember(record, name);
    if (!found) {
        return { OffsetofStatus::Missing, 0 };
    }
    if (found->isBitField()) {
        return { OffsetofStatus::BitField, 0 };
    }
    return { OffsetofStatus::Ok, found->offsetBytes };
}

int Type::memberCount() const {
    if (!isRecord()) {
        return 0;
    }
    return static_cast<int>(getMembers().size());
}

} // namespace type
