#ifndef REGISTER_SUBREG_H_
#define REGISTER_SUBREG_H_

#include <string>

namespace codegen {

class Register;

// Partial register spellings without dialect prefixes (al, r8b, eax, r8d).
std::string lowByteName(const Register& reg);
std::string lowDwordName(const Register& reg);

} // namespace codegen

#endif // REGISTER_SUBREG_H_
