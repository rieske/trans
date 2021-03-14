#include "TokenStream.h"

namespace parser {

TokenStream::TokenStream(std::function<scanner::Token()> scan) :
    scan { scan },
    currentToken { this->scan() }
{
}

scanner::Token TokenStream::getCurrentToken() const {
	return (forgedToken ? *forgedToken : *currentToken);
}

scanner::Token TokenStream::nextToken() {
	if (forgedToken) {
        forgedToken.reset();
	} else {
		currentToken.emplace(scan());
	}
	return *currentToken;
}

void TokenStream::forgeToken(scanner::Token forgedToken) {
	this->forgedToken.emplace(forgedToken);
}

bool TokenStream::currentTokenIsForged() const {
	return (forgedToken ? true : false);
}

} // namespace parser

