#ifndef FIELDACCESS_QUADRUPLE_H_
#define FIELDACCESS_QUADRUPLE_H_

#include <string>
#include "Quadruple.h"

namespace codegen {

class FieldLoad: public Quadruple {
public:
    FieldLoad(std::string base, int offsetBytes, std::string result, bool baseIsPointer = false);
    void generateCode(AssemblyGenerator& generator) const override;
    std::string getBase() const { return base; }
    int getOffsetBytes() const { return offsetBytes; }
    std::string getResult() const { return result; }
    bool baseIsPointer() const { return baseIsPointer_; }
private:
    void print(std::ostream& stream) const override;
    std::string base;
    int offsetBytes;
    std::string result;
    bool baseIsPointer_;
};

class FieldStore: public Quadruple {
public:
    FieldStore(std::string value, std::string base, int offsetBytes, bool baseIsPointer = false);
    void generateCode(AssemblyGenerator& generator) const override;
    std::string getValue() const { return value; }
    std::string getBase() const { return base; }
    int getOffsetBytes() const { return offsetBytes; }
    bool baseIsPointer() const { return baseIsPointer_; }
private:
    void print(std::ostream& stream) const override;
    std::string value;
    std::string base;
    int offsetBytes;
    bool baseIsPointer_;
};

class FieldAddress: public Quadruple {
public:
    FieldAddress(std::string base, int offsetBytes, std::string result, bool baseIsPointer = false);
    void generateCode(AssemblyGenerator& generator) const override;
    std::string getBase() const { return base; }
    int getOffsetBytes() const { return offsetBytes; }
    std::string getResult() const { return result; }
    bool baseIsPointer() const { return baseIsPointer_; }
private:
    void print(std::ostream& stream) const override;
    std::string base;
    int offsetBytes;
    std::string result;
    bool baseIsPointer_;
};

} // namespace codegen
#endif
