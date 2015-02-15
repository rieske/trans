#ifndef FUNCTION_H_
#define FUNCTION_H_

#include <cstddef>
#include <string>

namespace code_generator {

class Function {
public:
    Function(std::string name, std::size_t argumentCount);

    std::string getName() const;
    std::size_t getArgumentCount() const;

private:
    std::string name;
    std::size_t argumentCount;
};

} /* namespace code_generator */

#endif /* FUNCTION_H_ */
