#ifndef PARSETREETOSOURCECONVERTER_H_
#define PARSETREETOSOURCECONVERTER_H_

#include <ostream>
#include <unordered_map>
#include <string>

#include "ParseTreeNodeVisitor.h"

namespace parser {

class ParseTreeToSourceConverter: public parser::ParseTreeNodeVisitor {
public:
    ParseTreeToSourceConverter(std::ostream* stream);
    virtual ~ParseTreeToSourceConverter();

    void visit(const ParseTreeNode& node) const;
    void visit(const TerminalNode& node) const;

private:
    void adjustIdentation(std::string type) const;
    std::string getDelimiterForType(std::string type) const;

    std::ostream* outputStream;

    std::unordered_map<std::string, std::string> delimiters;

    mutable std::string identation { "" };
    mutable std::string delimiter { "" };
};

} // namespace parser

#endif // PARSETREETOSOURCECONVERTER_H_
