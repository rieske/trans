#ifndef ASSEMBLY_H_
#define ASSEMBLY_H_

#include <ostream>

namespace codegen {

class Assembly {
public:
    Assembly(std::ostream* ostream);
    Assembly(const Assembly&) = default;
    Assembly(Assembly&&) = default;
    virtual ~Assembly() = default;

    Assembly& operator=(const Assembly&) = delete;
    Assembly& operator=(Assembly&&) = default;

    Assembly& operator<<(const std::string& instruction);

    void raw(std::string raw);
    void label(std::string label);

private:
    std::ostream* assembly;
};

}

#endif /* ASSEMBLY_H_ */
