#ifndef TOKENSTREAM_H_
#define TOKENSTREAM_H_

#include "scanner/Token.h"

#include <optional>
#include <functional>

namespace parser {

class TokenStream {
public:
	TokenStream(std::function<scanner::Token()> scan);

    scanner::Token getCurrentToken() const;
    scanner::Token nextToken();

	void forgeToken(scanner::Token forgedToken);
	bool currentTokenIsForged() const;
private:
    std::function<scanner::Token()> scan;

	std::optional<const scanner::Token> currentToken;
	std::optional<const scanner::Token> forgedToken;
};

} // namespace parser

#endif // TOKENSTREAM_H_
