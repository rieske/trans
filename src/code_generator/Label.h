#ifndef LABEL_H_
#define LABEL_H_

#include <string>

namespace code_generator {

class Label {
public:
    Label(std::string name);

    std::string getName() const;

private:
    std::string name;
};

} /* namespace code_generator */

#endif /* LABEL_H_ */
