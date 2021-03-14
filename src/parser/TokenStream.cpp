#include "TokenStream.h"

#include "scanner/Scanner.h"
#include "scanner/Token.h"

namespace parser {

TokenStream::TokenStream(std::function<scanner::Token()> scanner) :
    scanner { scanner },
    currentToken { new scanner::Token { this->scanner() } }
{
}

scanner::Token TokenStream::getCurrentToken() const {
	return (forgedToken ? *forgedToken : *currentToken);
}

scanner::Token TokenStream::nextToken() {
	if (forgedToken) {
        forgedToken.reset();
	} else {
		currentToken.reset(new scanner::Token { scanner() });
	}
	return *currentToken;
}

void TokenStream::forgeToken(scanner::Token forgedToken) {
	this->forgedToken.reset(new scanner::Token { forgedToken });
}

bool TokenStream::currentTokenIsForged() const {
	return (forgedToken ? true : false);
}

} // namespace parser

