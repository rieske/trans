#ifndef TYPE_H_
#define TYPE_H_

namespace ast {

enum class BuiltInType {
    CHAR, SHORT, INTEGER, LONG, FLOAT, DOUBLE
};

class FundamentalType {
public:
    virtual ~FundamentalType() = default;

protected:
    FundamentalType() = default;
    FundamentalType(const FundamentalType& rhs) = default;
    FundamentalType(FundamentalType&& rhs) = default;
    FundamentalType& operator=(const FundamentalType& rhs) = default;
    FundamentalType& operator=(FundamentalType&& rhs) = default;
};

class StoredType: public FundamentalType {
public:
    virtual ~StoredType() = default;

protected:
    StoredType() = default;
    StoredType(const StoredType& rhs) = default;
    StoredType(StoredType&& rhs) = default;
    StoredType& operator=(const StoredType& rhs) = default;
    StoredType& operator=(StoredType&& rhs) = default;
};

class Pointer: public StoredType {
public:
    Pointer(std::unique_ptr<FundamentalType> to, bool _const, bool _volatile);
    Pointer(const Pointer& rhs);
    Pointer(Pointer&& rhs);
    Pointer& operator=(const Pointer& rhs);
    Pointer& operator=(Pointer&& rhs);
    ~Pointer() = default;

private:
    std::unique_ptr<FundamentalType> pointsTo;

    bool _const;
    bool _volatile;
};

class IntegralType: public StoredType {
public:

private:
    Storage storage;
    bool _const;
    bool _volatile;
};

class Function: public FundamentalType {
public:
    Function(std::vector<std::unique_ptr<StoredType>> argumentTypes);
    Function(const Function& rhs);
    Function(Function&& rhs);
    Function& operator=(const Function& rhs);
    Function& operator=(Function&& rhs);
    ~Function() = default;

private:
    std::vector<std::unique_ptr<StoredType>> argumentTypes;
};

class Array: public StoredType {
public:
    Array(std::unique_ptr<StoredType> elementType, std::size_t size);
    Array(const Array& rhs);
    Array(Array&& rhs);
    Array& operator=(const Array& rhs);
    Array& operator=(Array&& rhs);
    ~Array() = default;

private:
    std::unique_ptr<StoredType> elementType;
    std::size_t size;
};

}

#endif /* TYPE_H_ */
