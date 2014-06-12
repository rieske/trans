#ifndef SCANNER_H_
#define SCANNER_H_

class Token;

class Scanner {
public:
	virtual ~Scanner() {
	}

	virtual Token nextToken() = 0;
};

#endif /* SCANNER_H_ */
