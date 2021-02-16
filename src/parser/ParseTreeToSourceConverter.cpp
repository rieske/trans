#include "ParseTreeToSourceConverter.h"

#include "TerminalNode.h"

static const std::string DEFAULT_DELIMITER = " ";
static const std::string IDENTATION_SIZE = "    ";
static const std::string OPEN_BLOCK = "{";
static const std::string CLOSE_BLOCK = "}";

namespace parser {

ParseTreeToSourceConverter::ParseTreeToSourceConverter(std::ostream* stream) :
        outputStream { stream } {
    delimiters[";"] = "\n";
    delimiters["{"] = "\n";
    delimiters["}"] = "\n";
}

ParseTreeToSourceConverter::~ParseTreeToSourceConverter() {
}

void ParseTreeToSourceConverter::visit(const ParseTreeNode& node) const {
    for (const auto& child : node.getChildren()) {
        child->accept(*this);
    }
}

void ParseTreeToSourceConverter::visit(const TerminalNode& node) const {
    *outputStream << delimiter;
    adjustIdentation(node.getType());
    if (delimiter == "\n") {
        *outputStream << identation;
    }
    *outputStream << node.getValue();
    delimiter = getDelimiterForType(node.getType());
}

void ParseTreeToSourceConverter::adjustIdentation(std::string type) const {
    if (type == OPEN_BLOCK) {
        identation += IDENTATION_SIZE;
    } else if (type == CLOSE_BLOCK) {
        identation.resize(identation.length() - IDENTATION_SIZE.length());
    }
}

std::string ParseTreeToSourceConverter::getDelimiterForType(std::string type) const {
    if (delimiters.find(type) == delimiters.end()) {
        return DEFAULT_DELIMITER;
    }
    return delimiters.at(type);
}

} // namespace parser

