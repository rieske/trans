#include "TokenStream.h"

#include "scanner/Scanner.h"
#include "scanner/Token.h"

namespace parser {

TokenStream::TokenStream(scanner::Scanner* scanner) :
    scanner { scanner },
    currentToken { new scanner::Token { this->scanner->nextToken() } }
{
}

TokenStream::~TokenStream() {
}

scanner::Token TokenStream::getCurrentToken() const {
	return (forgedToken ? *forgedToken : *currentToken);
}

scanner::Token TokenStream::nextToken() {
	if (forgedToken) {
        forgedToken.reset();
	} else {
		currentToken.reset(new scanner::Token { scanner->nextToken() });
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

