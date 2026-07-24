# Architecture

trans is a C-to-NASM compiler (x86-64, System V AMD64 ABI). Host `gcc` is used
only as a preprocessor and linker; the real frontend and backend live in this
tree.

For build instructions see [README.md](README.md).

## Pipeline

```
  .c source
      |
      v
  host gcc -E  (dialect + -I/-D/-U + trailing defines)
      |
      v
  Scanner  (finite automaton; scanner.lex)
      |
      v
  LALR parser  (grammar.bnf + parsing_table) -> AST
      |
      v
  SemanticAnalysisVisitor*  (types, symbols, constant / multi-word global init)
      |
      v
  CodeGeneratingVisitor*  (AST -> quadruples)
      |
      v
  StackMachine + IntelInstructionSet  (quadruples -> NASM)
      |
      v
  nasm -f elf64
      |
      v
  host gcc -m64 -no-pie ... + va_tls.o + libc
```

Driver entry points:

| Stage | Main code |
| --- | --- |
| CLI / flags | `src/driver/ConfigurationParser.cpp` (table-driven options + special `-l`/`-g`), `Driver.cpp` |
| Preprocess | `Compiler::preprocess`, `HostToolchain.cpp` |
| Host paths / link | `HostToolchain.cpp` |
| Scan / parse / AST | `src/scanner`, `src/parser`, `src/ast` |
| Semantics | `src/semantic_analyzer` (`DeclarationAnalyzer`, `InitializerLowering`, `ConstantAddress`, `ArrayDecay`, `BuiltinAnalysis`) |
| IR + assembly | `src/codegen` (`FrameLayout`, multi-TU `StackMachine*`, multi-TU `CodeGeneratingVisitor*`) |
| Types / layout | `src/types` (`ObjectAbi.h` as `type::object_abi`, `TypeQuery.h`); frame bridge `codegen/FrameSymbol.h` |
| Symbols | `src/symbols` (`ValueEntry`, `LabelEntry`, `FunctionEntry`) — leaf of ast + SA; **no** ast→semantic_analyzer link |
| Product contracts | `src/util/ProductApprox.h` + `ProductContractsTest` |

Per translation unit, typedef/enum registries are cleared before compile
(`Compiler::compile`). Pending struct `ARRAY_SIZE` bounds live on the
`AbstractSyntaxTree` product (`PendingArrayMemberStore`, keyed by
`structureBodyIdentity()`), filled during AST build. Semantic analysis is
two-phase: (1) file-scope symbols, (2) one late re-fold of all pending member
array bounds, (3) function bodies — not process-global and not inside `types/`.

## Host preprocess + real frontend

There is no post-`gcc -E` string-rewrite layer. Host preprocess expands system
headers; the scanner, grammar, and visitors handle the resulting C (including
GNU spellings that matter for real headers).

### Host preprocess

`gcc -E -P -std=c99` expands system headers. Dialect and trailing defines are
set in `HostToolchain` (see Host integration below). Already-preprocessed `.i`
inputs skip this step.

### Real compiler (scan through NASM)

- **Scanner** - lexemes from `resources/configuration/scanner.lex`; typedef and
  enum-constant registries feed the automaton. Lexical conveniences include
  ignoring attributes/asm noise, string concat, wide prefixes, `_Bool`,
  `__int128` / `_FloatN` stand-ins (`long long` / `float`/`double`), and
  `__func__`-style idents.
- **Parser** - LALR table from `grammar.bnf`; `ContextualSyntaxNodeBuilder`
  (domain TUs: `CSNB_{Types,Declarators,Expressions,Initializers,Statements,Builtins}.cpp`)
  reduces productions into AST nodes (including `_Generic`, compound literals,
  `typeof`, type-arg builtins, and first-class `va_*` / `__builtin_va_list`).
- **Semantic analysis** - multi-TU `SemanticAnalysisVisitor*` orchestrates analysis;
  `DeclarationAnalyzer` owns linkage/symbol insert and init dispatch;
  `InitializerLowering` expands brace/designated/string init into field stores
  or multi-word `.data`; `ConstantAddress` folds address constants;
  `ArrayDecay` rewrites array results in place (decay source on `Expression`);
  `BuiltinAnalysis` table-drives `__builtin_*` / `va_*` kinds;
  `PendingArrayMemberStore` on the AST holds struct member declarators whose
  bounds need a single late re-fold after file-scope symbols exist.
- **Codegen IR** - thin quadruple classes under `src/codegen/quadruples/`
  (assign, call, field address, `BuiltinOp`, `VaOp`, etc.); frame packing in
  `FrameLayout`; multi-TU `CodeGeneratingVisitor*` with shared helpers in
  `CodeGeneratingVisitorInternal.h`.
- **FrameLayout** - linear-scan spill slots; `:= &obj` keeps `obj` live through the
  pointer's last use (incl. PARAM→CALL) so multi-word arrays/CLs are not reused early
- **StackMachine** - multi-TU (`StackMachine{Frame,Call,Memory,Va,Arith}.cpp`)
  maps quadruples to NASM using homes and SysV conventions; multi-word and sret
  policy is centralized in `types/ObjectAbi.h`. Variadic frames use `VaOp` plus
  `va_tls` helpers (no text rewrite of `va_start`).

Documented approximations (not full C) — named helpers in `util/ProductApprox.h`,
contracts in `ProductContractsTest`:

- `product_approx::foldConstantP` / `foldTypesCompatibleP` (always 0)
- `product_approx::clampNegativeArrayBoundForBuildAssert` (`BUILD_ASSERT_OR_ZERO`)
- `__STDC__` forced via trailing `-D` (and `-w` to silence redefinition noise)
- attributes / asm / pragmas ignored in the scanner when they would break the
  grammar rather than fully typed

Treat a failing functional case as the bar for frontend changes; do not polish
without a regression test.

## Types (`types/Type.h`)

`TypeKind` classifies void / primitive / pointer / function / array / struct /
union. Predicates: `isRecord()` (struct|union), `isStructure()` (struct only),
`isUnion()`, `isAggregate()` (array|record). Unions are not structures.

Pointers and arrays are recursive: `pointer(T)` / `array(T,n)` hold a nested
`Type` (shared_ptr). The node payload is a **closed `std::variant`** (void /
primitive / pointer / function / array / record) — not a bag of optionals.
`isPrimitive()` / `isFunction()` / `isRecord()` are kind of this node only.
Prefer `type::isFloating` / `isIntegral` / `valueIsSigned` / `isProductScalar`
from `TypeQuery.h`.

Struct and union share `RecordPayload` + `StructBody`. Incomplete tags use
`incompleteStructure()`; `completeStructure` / `completeUnion` mutate the shared
body (including `isUnion`). Types that share `structureBodyIdentity()` see the
same completion — required for self-referential tags. Do not split struct/union
into separate variant arms without preserving that identity.

Product assign: **`type::productCanAssignFrom(dest, source)`** only (in
`TypeQuery.cpp`, with other type-policy helpers) is intentionally looser than
ISO C. SA enforces it via `requireProductAssignable`. Structural layout and
equivalence stay in `Type.cpp`.

Codegen register classification uses **`codegen::ValueKind`** (`INTEGRAL` /
`FLOATING`) and `Value::getValueKind()` — not `type::Type`.

## Operators (`ast/Operator.h`)

`OperatorKind` is set at construction from the lexeme. Unary forms use
`Operator::makeUnary` (`*` → `Deref`, `&` → `AddressOf`). Binary/assignment use
`Operator(string)`. Codegen shares `emitBinaryOp` for arithmetic, bitwise, shift,
and compound assignment (`compoundAssignBase`).

## Object ABI (`types/ObjectAbi.h`)

Shared size and return policy (namespace `codegen::object_abi`):

- live Values / stack slots (`valueWords`, min 1 word)
- multi-word `.data` flattening in semantics (`dataWords`, 0 if empty)
- aggregate memory return / sret: **`typeNeedsMemoryReturn(const Type&)`**
  (aggregate ∧ size > 16); hidden pointer local `__sret`. Size-only helper
  `needsMemoryReturn(size)` is the threshold piece, not a second policy.

Call sites:

- `CodeGeneratingVisitor` - `typeNeedsMemoryReturn` for Call sret dest and
  `StartProcedure` memoryReturn; sets `Retrieve(name, memoryReturn)` with the
  same decision
- `StackMachine` - word counts; callee return trusts `sretSymbolName`; caller
  retrieve trusts `Retrieve::isMemoryReturn` (no size re-derivation)
- `InitializerLowering` - packing brace/designated init into qword words

Machine word size is 8; stack alignment at call sites is 16.

## Expression types (`ast/Expression`)

Two related notions after semantic analysis:

- **`expressionType()`** — C type of the expression (sizeof, `isArray`).
  Array members / multi-dim rows keep the array type here.
- **`valueType()`** — type of the result symbol (after array-to-pointer decay
  temps). Prefer for arithmetic, signedness, and codegen value paths.

Ownership: `setResultSymbol` sets only the value annotation; `setType` sets only
expression type. `expressionType()` falls back to the result symbol type when
expression type is unset. Member arrays and multi-dim rows call `setType(T[N])`
then `setResultSymbol(pointer)` so the two stay distinct.

## Host integration

| Concern | Behavior |
| --- | --- |
| Preprocess dialect | `-E -P -std=c99 -x c` |
| Trailing defines | `-D__STDC__=0 -DCURL_DISABLE_TYPECHECK -w` (after user `-D`/`-U`) |
| Assemble | `nasm -O1 -f elf64` |
| Link | `gcc -m64 -no-pie` plus objects, `va_tls.o`, and user link args |
| `va_tls.o` | Built from `runtime/va_tls.c`; searched via `TRANS_VA_TLS`, resources path, exe/cwd walks |

`-no-pie` is required: NASM objects use absolute/PC32 relocs that fail default
PIE links against libc.

### Variadic runtime (`runtime/va_tls.c`)

SysV `va_list` needs a reg_save_area and overflow_arg_area. Prologue code
registers those into a **thread-local stack** (`__trans_va_set_areas` /
`__trans_va_pop_areas`) so:

- concurrent threads do not share one save area
- nested variadic calls restore the outer frame on return

`__builtin_va_list` is a first-class type (24-byte SysV tag array-of-1).
`va_start` / `va_arg` / `va_copy` / `va_end` are real frontend builtins that
emit `VaOp` quadruples; the runtime supplies only the TLS area helpers.

## Source layout

```
src/
  driver/           CLI, configuration, preprocess, host toolchain
  scanner/          FA lexer + registries
  parser/           LALR table, actions
  ast/              nodes + ContextualSyntaxNodeBuilder; ParseEnvironment
                    (tags/typedefs/enums) vs reduction stacks on builder context
  semantic_analyzer/
  codegen/          quadruples, StackMachine, Intel NASM, ObjectAbi
  types/            Type, Primitive, Function, struct/union layout
  translation_unit/ Context / source positions
  util/             logging
runtime/            va_tls.c (linked into user programs)
resources/configuration/
  scanner.lex, grammar.bnf, parsing_table
test/src/
  functionalTest/   end-to-end compile-and-run (themed by area)
  *Test/            unit tests per library
```

## Testing model

- **Functional tests** (`Compiler.*`) compile small C programs with `trans`,
  assemble, link with `va_tls.o`, and check stdout/exit. Grouped by theme
  (e.g. `Global*`, `Structs*`, `Variadic*`, `Preprocessor*`, `DriverModes*`).
- **Driver unit tests** cover host path search, configuration flags, and
  `va_tls` helpers.
- **Codegen unit tests** cover instruction emission, stack machine behavior, and
  `ObjectAbi` formulas.

Intentional product changes should land with a failing test first (functional
when user-visible; unit when isolating a pure helper).

### Feature → test map

Use this as the first place to add coverage when extending a surface. Product
approximations are frozen in `ProductContracts*` so “fixing” them is a
deliberate contract change.

| Feature / concern | Primary tests |
| --- | --- |
| Host preprocess contracts (`__STDC__=0`, curl, builtins→0, attributes, `_Generic` default, `__int128` size, BUILD_ASSERT, `__func__`, stmt-expr reject, bitfield widths ignored) | `ProductContractsTest`, also `PreprocessorTest` |
| Driver modes (`-c/-S/-E/-o`, depfiles, link-only, multi-file argv) | `DriverModesTest`, `ConfigurationParserTest`, `HostToolchainTest` |
| Multi-TU define/call, static isolation, extern data, duplicate-def link fail | `MultiTuTest` |
| Structs, `.` / `->`, pointer members, multi-word homes | `StructsTest`, `StructsMultiWordTest` |
| Struct / designated / string init | `StructsInitTest` |
| Compound literals | `StructsCompoundLiteralTest` |
| Struct return (incl. sret), nested sret, pass+return (≤16B) | `StructsReturnTest` |
| Unsupported: large pass-by-value + sret return (compile-only pin) | `StructsReturnTest` (`largeStructPassAndSretReturnCompiles`) |
| sret policy for variadic callees | `StackMachineTests` (`variadicMemoryReturnSkipsSret`), `StructsReturnTest` |
| Unions, anonymous flatten | `UnionsTest` |
| SysV `va_list` / nested / concurrent / float packing | `VariadicTest`, `VaTlsRuntimeTest`, `StackMachineTests` |
| Unsupported: multi-word `va_arg` (syntax+link pin; runtime wrong) | `VariadicTest` (`vaArgOfStructIsAcceptedByFrontend`) |
| Assignment type compatibility (`productCanAssignFrom`) | `TypeTest`, `SemanticErrorsTest` (`assignStructToScalarRejected`) |
| Bare expression statements (`a + b;`) | `IntegrationTest` (`expressionStatementOnly`) |
| Float/double SysV (xmm, compare promote, implicit convert) | `TypesTest`, `VariadicTest` |
| Layout vs libc (`struct stat`, natural align) | `TypeTest`, `StructsMultiWordTest` (`fstat…`) |
| Enums, typedef, typedef shadow | `EnumsTest`, `TypedefTest` |
| Switch / goto / for-decl / do-while | `SwitchTest`, `GotoTest`, `ControlFlowTest`, `DoWhileTest` |
| Function pointers / indirect calls | `FunctionPointersTest` |
| Globals, static locals, linkage, NASM-reserved names | `GlobalVariablesTest`, `GlobalStaticsAndLinkageTest` |
| Sizeof, string escapes, incomplete arrays | `SizeofTest`, `ArraysTest` |
| Semantic / parse rejections | `SemanticErrorsTest`, `CompilerInitTest` |
| Object ABI formulas | `ObjectAbiTests` |
| Stack machine call/sret/va/float emission | `StackMachineTests` |

Unsupported or approximated surfaces should keep an explicit contract test (expected
output or expected compile failure) rather than relying only on git’s suite.

## Design notes / non-goals

- **Layering:** `ast` links `symbols` + `types` only — never `semantic_analyzer`.
  Analysis annotations (result symbols, labels, frames) live in
  `symbols::AnnotationStore` (side table keyed by node address), bound for SA+codegen
  of one TU. AST objects keep C structure and expression types only.
  Builtin lowering tables live in `ast::BuiltinRegistry` (shared by SA and codegen;
  backend does not link `semantic_analyzer`).

- The intermediate form is intentionally thin; ABI detail concentrates in
  `StackMachine` + `ObjectAbi`, not in every quadruple type.
- AT&T / GAS instruction set stubs exist but the production path is
  Intel-syntax NASM only.
- Full C (or full GCC C) is not the goal. The product target is enough C and
  SysV ABI fidelity to compile substantial real code (including git-shaped
  patterns) after host preprocess, with language features in the real
  frontend rather than a string rewrite layer.
- Large visitors are multi-TU (`SemanticAnalysisVisitor_*`, `StackMachine*`,
  `CSNB_*`). Put policy in `TypeQuery`, `type::object_abi`, or `ProductApprox`
  first; visitors only call those helpers.

## Extending the compiler

1. **Language feature the grammar already parses** - semantic visitor, then
   quadruples if needed, then StackMachine emission; functional test first.
2. **GNU / header construct after gcc -E** - prefer scanner, grammar, or
   visitor support with a functional case; do not reintroduce a whole-file
   string rewrite pipeline.
3. **ABI / layout rule** - put the formula in `ObjectAbi` (or `types` layout)
   and call it from both semantic init and codegen so they cannot drift.
4. **Driver flag** - `ConfigurationParser` + `DriverModesTest` (or related).
5. **Product specials (git-shaped rules, soft assign, layout exceptions)** - put
   the *decision* in `TypeQuery`, `ObjectAbi`, or `ProductApprox` first, with a
   unit/contract test; visitors only call the helper. Do not bolt one-off
   branches into busy SA/codegen paths as the default.

When in doubt, mirror an existing path that already has functional coverage
rather than inventing a parallel mechanism.
