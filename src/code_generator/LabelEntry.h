#ifndef LABELENTRY_H_
#define LABELENTRY_H_

#include <string>

namespace code_generator {

class LabelEntry {
public:
    LabelEntry(std::string name);
    virtual ~LabelEntry();

    void print() const;

    std::string getName() const;

private:
    std::string name;
};

} /* namespace code_generator */

#endif /* LABELENTRY_H_ */
