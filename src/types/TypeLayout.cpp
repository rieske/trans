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

void requireFitsInt(long long value, const char* error) {
    if (value > static_cast<long long>(std::numeric_limits<int>::max())) {
        throw std::invalid_argument { error };
    }
}

void requireUniqueMemberName(const LayoutCursor& cursor, const std::string& name, bool asUnion) {
    if (name.empty()) {
        return;
    }
    for (const auto& existing : cursor.members) {
        if (existing.name == name) {
            throw std::invalid_argument { asUnion
                    ? "duplicate union member name"
                    : "duplicate structure member name" };
        }
    }
}

void requireCompleteMember(const Type& memberType, bool flexibleArray, bool asUnion) {
    if (isTentativeRecord(memberType)) {
        return;
    }
    if (isIncompleteMemberOrElementType(memberType) && !flexibleArray) {
        throw std::invalid_argument { asUnion
                ? "union member has incomplete type"
                : "structure member has incomplete type" };
    }
}

// Validates bit-field width; returns the declared type width in bits (the pack unit).
int bitFieldUnitBits(const Type& memberType, int width, const std::string& name) {
    if (!isIntegral(memberType)) {
        throw std::invalid_argument { "bit-field has non-integer type" };
    }
    if (width == 0 && !name.empty()) {
        throw std::invalid_argument { "zero width for bit-field" };
    }
    const int typeBits = memberType.getSize() * 8;
    if (memberType.getSize() > 8 || width > 64) {
        throw std::invalid_argument { "bit-field type is too wide" };
    }
    if (width > typeBits) {
        throw std::invalid_argument { "width of bit-field exceeds its type" };
    }
    return typeBits;
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

void layoutOrdinaryStructMember(LayoutCursor& cursor, const std::string& name, const Type& memberType,
        int align) {
    long long offset = (cursor.bitOffset + 7) / 8;
    offset = alignUp(offset, align);
    requireFitsInt(offset, "structure size is too large");
    cursor.members.emplace_back(name, memberType, static_cast<int>(offset));
    offset += memberSize(memberType);
    requireFitsInt(offset, "structure size is too large");
    cursor.bitOffset = offset * 8;
}

void layoutBitFieldUnionMember(LayoutCursor& cursor, const std::string& name, const Type& memberType,
        int width) {
    if (width > 0 && !name.empty()) {
        cursor.members.emplace_back(name, memberType, 0, makeBitField(memberType, width, 0));
    }
    noteUnionMemberSize(cursor, memberType);
}

void layoutBitFieldStructMember(LayoutCursor& cursor, const std::string& name, const Type& memberType,
        int width, int typeBits, int align) {
    if (width == 0) {
        const long long alignBits = static_cast<long long>(align) * 8;
        if (alignBits > 0 && (cursor.bitOffset % alignBits) != 0) {
            cursor.bitOffset = alignUp(cursor.bitOffset, alignBits);
        }
        return;
    }
    const int unitBits = typeBits > 0 ? typeBits : 8;
    const int excess = static_cast<int>(cursor.bitOffset % unitBits);
    if (excess + width > unitBits) {
        cursor.bitOffset = alignUp(cursor.bitOffset, static_cast<long long>(align) * 8);
    }
    requireFitsInt(cursor.bitOffset, "structure size is too large");
    if (!name.empty()) {
        const int container = (static_cast<int>(cursor.bitOffset) / unitBits) * memberType.getSize();
        const int shift = static_cast<int>(cursor.bitOffset) % unitBits;
        cursor.members.emplace_back(name, memberType, container,
                makeBitField(memberType, width, shift));
    }
    cursor.bitOffset += width;
}

int finalizeLayoutSize(const LayoutCursor& cursor, bool asUnion) {
    if (asUnion) {
        const long long size = alignUp(cursor.maxSize, cursor.maxAlign);
        requireFitsInt(size, "union size is too large");
        return static_cast<int>(size);
    }
    const long long size = alignUp((cursor.bitOffset + 7) / 8, cursor.maxAlign);
    requireFitsInt(size, "structure size is too large");
    return static_cast<int>(size);
}

// Built into temporaries so a failed re-complete does not corrupt the live body.
void layoutRecordMembers(Type::StructBody& body, const std::vector<MemberSpec>& members, bool asUnion,
        bool packed) {
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
        requireCompleteMember(memberType, flexibleArray, asUnion);
        requireUniqueMemberName(cursor, name, asUnion);

        const int align = packed ? 1 : memberType.getAlignment();
        if (align > cursor.maxAlign) {
            cursor.maxAlign = align;
        }

        if (spec.bitWidth) {
            const int width = *spec.bitWidth;
            const int unitBits = bitFieldUnitBits(memberType, width, name);
            if (asUnion) {
                layoutBitFieldUnionMember(cursor, name, memberType, width);
            } else {
                layoutBitFieldStructMember(cursor, name, memberType, width, unitBits, align);
            }
            continue;
        }
        if (asUnion) {
            layoutOrdinaryUnionMember(cursor, name, memberType);
        } else {
            layoutOrdinaryStructMember(cursor, name, memberType, align);
        }
    }

    body.members = std::move(cursor.members);
    body.isUnion = asUnion;
    body.packed = packed;
    body.size = finalizeLayoutSize(cursor, asUnion);
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
}

} // namespace

void completeStructure(Type& structType, const std::vector<MemberSpec>& members, bool packed) {
    auto* rec = std::get_if<Type::RecordPayload>(&structType._payload);
    if (!rec || !rec->body) {
        throw std::domain_error { "completeStructure on non-record type" };
    }
    layoutRecordMembers(*rec->body, members, false, packed);
}

void completeUnion(Type& unionTy, const std::vector<MemberSpec>& members, bool packed) {
    auto* rec = std::get_if<Type::RecordPayload>(&unionTy._payload);
    if (!rec || !rec->body) {
        throw std::domain_error { "completeUnion on non-record type" };
    }
    layoutRecordMembers(*rec->body, members, true, packed);
}

} // namespace type
