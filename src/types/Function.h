#ifndef _FUNCTION_H_
#define _FUNCTION_H_

#include <memory>
#include <string>
#include <vector>

namespace type {

class Type;

class Function {
public:
    Function(std::unique_ptr<Type> returnType, std::vector<std::unique_ptr<Type>> arguments = {},
            bool variadic = false);
    Function(const Function&) = default;
    Function(Function&&) noexcept = default;
    Function& operator=(const Function&) = default;
    Function& operator=(Function&&) noexcept = default;

    const Type& getReturnType() const;
    const std::vector<Type>& getArguments() const;
    std::size_t argumentCount() const;
    bool isVariadic() const;

    std::string to_string() const;
private:
    struct Impl;
    std::shared_ptr<Impl> impl_;
};


} // namespace type

#endif // _FUNCTION_H_
