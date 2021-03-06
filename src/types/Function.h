#ifndef _FUNCTION_H_
#define _FUNCTION_H_

#include <vector>
#include <memory>

namespace type {

class Type;

class Function {
public:
    Function(std::unique_ptr<Type> returnType, std::vector<std::unique_ptr<Type>> arguments = {});
    Function(const Function& rhs);

    Type getReturnType() const;
private:
    std::unique_ptr<Type> returnType;
    std::vector<std::unique_ptr<Type>> arguments;
};


} // namespace type

#endif // _FUNCTION_H_
