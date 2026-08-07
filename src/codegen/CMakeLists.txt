# Target is "backend" (compiler backend: IR + assembly). Not "codegen" — that
# name is reserved by CMake (CMP0171). Source tree stays src/codegen/.
add_library(backend
        Amd64Registers.cpp
        AssemblyGenerator.cpp
        ATandTInstructionSet.cpp
        CodeGeneratingVisitor.cpp
        Instruction.cpp
        InstructionPrint.cpp
        IrGenerator.cpp
        IrPasses.cpp
        IntelInstructionSet.cpp
        Register.cpp
        RegisterSubreg.cpp
        StackMachine.cpp
        StackMachine_Addressing.cpp
        StackMachine_Sse.cpp
        Value.cpp
        Assembly.cpp
        Address.h
        Amd64Registers.h
        AssemblyGenerator.h
        ATandTInstructionSet.h
        CodeGeneratingVisitor.h
        Instruction.h
        IrBuilders.h
        IrGenerator.h
        IrPasses.h
        JumpCondition.h
        InstructionSet.h
        GlobalVariable.h
        IntelInstructionSet.h
        MemoryOperand.h
        Register.h
        RegisterSubreg.h
        StackMachine.h
        SysVCallConv.h
        Value.h
        Assembly.h)

trans_target_defaults(backend)
target_link_libraries(backend PUBLIC ast symbols)
