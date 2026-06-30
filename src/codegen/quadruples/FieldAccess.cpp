#include "FieldAccess.h"
#include "codegen/AssemblyGenerator.h"
#include <iostream>

namespace codegen {

FieldLoad::FieldLoad(std::string base, int offsetBytes, std::string result, bool baseIsPointer) :
        base { std::move(base) }, offsetBytes { offsetBytes }, result { std::move(result) }, baseIsPointer_ { baseIsPointer } {}
void FieldLoad::generateCode(AssemblyGenerator& generator) const { generator.generateCodeFor(*this); }
void FieldLoad::print(std::ostream& stream) const {
    stream << "\t" << result << " := *(" << base << (baseIsPointer_ ? "->" : ".") << offsetBytes << ")\n";
}

FieldStore::FieldStore(std::string value, std::string base, int offsetBytes, bool baseIsPointer) :
        value { std::move(value) }, base { std::move(base) }, offsetBytes { offsetBytes }, baseIsPointer_ { baseIsPointer } {}
void FieldStore::generateCode(AssemblyGenerator& generator) const { generator.generateCodeFor(*this); }
void FieldStore::print(std::ostream& stream) const {
    stream << "\t*(" << base << (baseIsPointer_ ? "->" : ".") << offsetBytes << ") := " << value << "\n";
}

FieldAddress::FieldAddress(std::string base, int offsetBytes, std::string result, bool baseIsPointer) :
        base { std::move(base) }, offsetBytes { offsetBytes }, result { std::move(result) }, baseIsPointer_ { baseIsPointer } {}
void FieldAddress::generateCode(AssemblyGenerator& generator) const { generator.generateCodeFor(*this); }
void FieldAddress::print(std::ostream& stream) const {
    stream << "\t" << result << " := &(" << base << (baseIsPointer_ ? "->" : ".") << offsetBytes << ")\n";
}

} // namespace codegen
