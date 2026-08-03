#ifndef REGISTER_SUBREG_H_
#define REGISTER_SUBREG_H_

#include <string>

namespace codegen {

class Register;

// Partial register spellings without dialect prefixes (al, ax, eax, r8b, ...).
// Shared by Intel and AT&T instruction sets - one map, no per-dialect copies.
std::string lowByteName(const Register& reg);
std::string lowWordName(const Register& reg);
std::string lowDwordName(const Register& reg);
std::string gprName(const Register& reg, int widthBytes);

} // namespace codegen

#endif // REGISTER_SUBREG_H_
