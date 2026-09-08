# Target is "backend" (compiler backend: IR + assembly). Not "codegen" — that
# name is reserved by CMake (CMP0171). Source tree stays src/codegen/.
add_library(backend
        Amd64Registers.cpp
        AssemblyGenerator.cpp
        ATandTInstructionSet.cpp
        Cfg.cpp
        CodeGeneratingVisitor.cpp
        CodeGeneratingVisitor_Addressing.cpp
        CodeGeneratingVisitor_Complex.cpp
        CodeGeneratingVisitor_BitField.cpp
        CodeGeneratingVisitor_Control.cpp
        CodeGeneratingVisitor_Lvalue.cpp
        FrameLayout.cpp
        Instruction.cpp
        InstructionPrint.cpp
        InstructionSet.cpp
        IrGenerator.cpp
        IrPasses.cpp
        IrStringTable.cpp
        SymbolRefs.cpp
        IntelInstructionSet.cpp
        Register.cpp
        RegisterSubreg.cpp
        StackMachine.cpp
        StackMachine_Addressing.cpp
        StackMachine_Arith.cpp
        StackMachine_Call.cpp
        StackMachine_Complex.cpp
        StackMachine_Sse.cpp
        StackMachine_Va.cpp
        StackMachine_Wide.cpp
        StackMachine_X87.cpp
        Value.cpp
        Assembly.cpp)

trans_target_defaults(backend)
target_link_libraries(backend PUBLIC ast symbols)
