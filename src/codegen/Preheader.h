#ifndef CODEGEN_PREHEADER_H_
#define CODEGEN_PREHEADER_H_

#include "Cfg.h"
#include "Loops.h"

namespace codegen {

struct PreheaderStats {
    int inserted { 0 };
};

void insertOne(Cfg& cfg, const NaturalLoop& loop, IrStringTable& strings);
PreheaderStats insertPreheaders(Cfg& cfg, IrStringTable& strings);

} // namespace codegen

#endif // CODEGEN_PREHEADER_H_
