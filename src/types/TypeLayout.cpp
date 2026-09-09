#include "Type.h"
#include "TypeQuery.h"

#include <limits>
#include <stdexcept>
#include <string>
#include <variant>
#include <vector>

namespace type {
namespace {

long long alignUp(long long offset, int alignment) {
    if (alignment <= 1) {
        return offset;
    }
    const long long rem = offset % alignment;
    return rem == 0 ? offset : offset + (alignment - rem);
}

int memberSize(const Type& memberType) {
    const int size = memberType.getSize();
    return size < 0 ? 0 : size;
}

struct LayoutCursor {
    std::vector<Type::Member> members;
    long long bitOffset { 0 };
    int maxAlign { 1 };
    long long maxSize { 0 };
};

const char* fitsIntError(long long value, const char* error) {
    if (value > static_cast<long long>(std::numeric_limits<int>::max())) {
        return error;
    }
    return nullptr;
}

const char* uniqueMemberNameError(const LayoutCursor& cursor, const std::string& name, bool asUnion) {
    if (name.empty()) {
        return nullptr;
    }
    for (const auto& existing : cursor.members) {
        if (existing.name == name) {
            return asUnion ? "duplicate union member name" : "duplicate structure member name";
        }
    }
    return nullptr;
}

const char* completeMemberError(const Type& memberType, bool flexibleArray, bool asUnion) {
    if (isTentativeRecord(memberType)) {
        return nullptr;
    }
    if (isIncompleteMemberOrElementType(memberType) && !flexibleArray) {
        return asUnion ? "union member has incomplete type" : "structure member has incomplete type";
    }
    return nullptr;
}

// Validates bit-field width; writes the declared type width in bits (the pack unit).
const char* bitFieldUnitBits(const Type& memberType, int width, const std::string& name,
        int& typeBits) {
    if (!isIntegral(memberType)) {
        return "bit-field has non-integer type";
    }
    if (width == 0 && !name.empty()) {
        return "zero width for bit-field";
    }
    typeBits = memberType.getSize() * 8;
    if (memberType.getSize() > 8 || width > 64) {
        return "bit-field type is too wide";
    }
    if (width > typeBits) {
        return "width of bit-field exceeds its type";
    }
    return nullptr;
}

void noteUnionMemberSize(LayoutCursor& cursor, const Type& memberType) {
    const long long size = memberSize(memberType);
    if (size > cursor.maxSize) {
        cursor.maxSize = size;
    }
}

void layoutOrdinaryUnionMember(LayoutCursor& cursor, const std::string& name, const Type& memberType) {
    cursor.members.emplace_back(name, memberType, 0);
    noteUnionMemberSize(cursor, memberType);
}

const char* layoutOrdinaryStructMember(LayoutCursor& cursor, const std::string& name,
        const Type& memberType, int align) {
    long long offset = (cursor.bitOffset + 7) / 8;
    offset = alignUp(offset, align);
    if (const char* error = fitsIntError(offset, "structure size is too large")) {
        return error;
    }
    cursor.members.emplace_back(name, memberType, static_cast<int>(offset));
    offset += memberSize(memberType);
    if (const char* error = fitsIntError(offset, "structure size is too large")) {
        return error;
    }
    cursor.bitOffset = offset * 8;
    return nullptr;
}

void layoutBitFieldUnionMember(LayoutCursor& cursor, const std::string& name, const Type& memberType,
        int width) {
    if (width > 0 && !name.empty()) {
        cursor.members.emplace_back(name, memberType, 0, makeBitField(memberType, width, 0));
    }
    noteUnionMemberSize(cursor, memberType);
}

const char* layoutBitFieldStructMember(LayoutCursor& cursor, const std::string& name,
        const Type& memberType, int width, int typeBits, int align) {
    if (width == 0) {
        const long long alignBits = static_cast<long long>(align) * 8;
        if (alignBits > 0 && (cursor.bitOffset % alignBits) != 0) {
            cursor.bitOffset = alignUp(cursor.bitOffset, alignBits);
        }
        return nullptr;
    }
    const int unitBits = typeBits > 0 ? typeBits : 8;
    const int excess = static_cast<int>(cursor.bitOffset % unitBits);
    if (excess + width > unitBits) {
        cursor.bitOffset = alignUp(cursor.bitOffset, static_cast<long long>(align) * 8);
    }
    if (const char* error = fitsIntError(cursor.bitOffset, "structure size is too large")) {
        return error;
    }
    if (!name.empty()) {
        const int container = (static_cast<int>(cursor.bitOffset) / unitBits) * memberType.getSize();
        const int shift = static_cast<int>(cursor.bitOffset) % unitBits;
        cursor.members.emplace_back(name, memberType, container,
                makeBitField(memberType, width, shift));
    }
    cursor.bitOffset += width;
    return nullptr;
}

const char* finalizeLayoutSize(const LayoutCursor& cursor, bool asUnion, int& size) {
    if (asUnion) {
        const long long bytes = alignUp(cursor.maxSize, cursor.maxAlign);
        if (const char* error = fitsIntError(bytes, "union size is too large")) {
            return error;
        }
        size = static_cast<int>(bytes);
        return nullptr;
    }
    const long long bytes = alignUp((cursor.bitOffset + 7) / 8, cursor.maxAlign);
    if (const char* error = fitsIntError(bytes, "structure size is too large")) {
        return error;
    }
    size = static_cast<int>(bytes);
    return nullptr;
}

// Built into a temporary so a failed re-complete does not corrupt the live body.
const char* layoutRecordMembers(Type::StructBody& body, const std::vector<MemberSpec>& members,
        bool asUnion, bool packed) {
    LayoutCursor cursor;

    const std::size_t memberCount = members.size();
    for (std::size_t i = 0; i < memberCount; ++i) {
        const auto& spec = members[i];
        const std::string& name = spec.name;
        const Type& memberType = spec.type;
        const bool flexibleArray = !asUnion
                && i + 1 == memberCount
                && !cursor.members.empty()
                && memberType.isIncompleteArray();
        if (const char* error = completeMemberError(memberType, flexibleArray, asUnion)) {
            return error;
        }
        if (const char* error = arrayTypeError(memberType)) {
            return error;
        }
        if (const char* error = uniqueMemberNameError(cursor, name, asUnion)) {
            return error;
        }

        const int align = packed ? 1 : memberType.getAlignment();
        if (align > cursor.maxAlign) {
            cursor.maxAlign = align;
        }

        if (spec.bitWidth) {
            const int width = *spec.bitWidth;
            int typeBits = 0;
            if (const char* error = bitFieldUnitBits(memberType, width, name, typeBits)) {
                return error;
            }
            if (asUnion) {
                layoutBitFieldUnionMember(cursor, name, memberType, width);
            } else if (const char* error = layoutBitFieldStructMember(
                    cursor, name, memberType, width, typeBits, align)) {
                return error;
            }
            continue;
        }
        if (asUnion) {
            layoutOrdinaryUnionMember(cursor, name, memberType);
        } else if (const char* error = layoutOrdinaryStructMember(cursor, name, memberType, align)) {
            return error;
        }
    }

    int size = 0;
    if (const char* error = finalizeLayoutSize(cursor, asUnion, size)) {
        return error;
    }
    body.members = std::move(cursor.members);
    body.isUnion = asUnion;
    body.packed = packed;
    body.size = size;
    bool complete = true;
    for (const auto& member : body.members) {
        if (!member.type) {
            continue;
        }
        if (hasRuntimeSize(*member.type) || isTentativeRecord(*member.type)) {
            complete = false;
            break;
        }
    }
    body.complete = complete;
    return nullptr;
}

} // namespace

const char* completeRecord(Type& record, const std::vector<MemberSpec>& members, bool asUnion,
        bool packed) {
    auto* rec = std::get_if<Type::RecordPayload>(&record._payload);
    if (!rec || !rec->body) {
        throw std::domain_error {
                asUnion ? "completeUnion on non-record type" : "completeStructure on non-record type" };
    }
    Type::StructBody built;
    if (const char* error = layoutRecordMembers(built, members, asUnion, packed)) {
        return error;
    }
    *rec->body = std::move(built);
    return nullptr;
}

const char* completeStructure(Type& structType, const std::vector<MemberSpec>& members, bool packed) {
    return completeRecord(structType, members, false, packed);
}

const char* completeUnion(Type& unionTy, const std::vector<MemberSpec>& members, bool packed) {
    return completeRecord(unionTy, members, true, packed);
}

} // namespace type
