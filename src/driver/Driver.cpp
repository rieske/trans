#include "Driver.h"

#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

#include "Compiler.h"
#include "CompilerComponentsFactory.h"
#include "Configuration.h"

using std::string;
using std::vector;
using std::unique_ptr;

Driver::Driver(const Configuration* configuration) :
		configuration(configuration) {
}

Driver::~Driver() {
}

void Driver::run() const {
	Compiler compiler { new CompilerComponentsFactory { *configuration } };

	vector<string> sourceFileNames = configuration->getSourceFileNames();
	for (string fileName : sourceFileNames) {
		try {
			compiler.compile(fileName);
		} catch (std::runtime_error& exception) {
			std::cerr << exception.what() << std::endl;
		}
	}
}
