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
  CodeGeneratingVisitor*  (AST -> IntermediateRepresentation)
      |
      v
  StackMachine + IntelInstructionSet  (IR -> NASM)
      |
      v
  nasm -f elf64
      |
      v
  host gcc -m64 -pie ... + libc
```

Driver entry points:

| Stage | Main code |
| --- | --- |
| CLI / flags | `src/driver/ConfigurationParser.cpp` (table-driven options + special `-l`/`-g`), `Driver.cpp` |
| Preprocess | `Compiler::preprocess`, `HostToolchain.cpp` |
| Host paths / link | `HostToolchain.cpp` |
| Scan / parse / AST | `src/scanner`, `src/parser`, `src/ast` |
| Semantics | `src/semantic_analyzer` (`DeclarationAnalyzer`, `InitializerLowering`, `ConstantAddress`, `ArrayDecay`); builtins in `src/builtins` (`BuiltinRegistry`) |
| IR + assembly | `src/codegen` (`FrameLayout`, multi-TU `StackMachine*`, multi-TU `CodeGeneratingVisitor*`) |
| Types / layout | `src/types` (`ObjectAbi.h` as `type::object_abi`, `TypeQuery.h`); frame bridge `codegen/FrameSymbol.h` |
| Symbols | `src/symbols` (`ValueEntry`, `LabelEntry`, `FunctionEntry`) — leaf of ast + SA; **no** ast→semantic_analyzer link |
| Product contracts | `src/util/ProductApprox.h` + `ProductContractsTest` |

Per translation unit, `Compiler::compileTranslationUnit` owns a
`scanner::LexicalSession` (instance `TypedefRegistry` + `EnumConstantRegistry`)
shared by the FA lexer, `TokenStream` id-role context, and `ParseEnvironment`.
Pending struct `ARRAY_SIZE` bounds live on the `AbstractSyntaxTree` product
(`PendingArrayMemberStore`, keyed by `structureBodyIdentity()`), filled during
AST build. Enumerators: sole parse authority is `session.enums`; the AST holds
a snapshot for SA import (not lists on `TypeSpecifier`). Semantic analysis is
three explicit passes: (1) file-scope symbols + function registration only,
(2) late re-fold of pending member array bounds, (3) function bodies via
`analyzeFunctionBody` — not process-global and not inside `types/`.

### Typedef name / id role (`TokenStream::LexIdContext`)

C allows the same spelling as a typedef and as an object/member/tag. The FA marks
known typedef spellings as `typedef_name`; `TokenStream` reclassifies to `id` when
`LexIdContext` is `AsIdentifier` (or when the name is an identifier shadow).

`LexIdContext` is a **previous-token role machine**, not a full parser-driven
type-position / expression-position state:

- `AsType` (default after `;` `{` `}` `)` `]` `(` `,`): keep `typedef_name`
- `AsIdentifier` after tags (struct/union/enum), member ops (`.` `->`), declarator
  cues (`*` / `typedef_name`), and expression cues (keywords, unary/binary ops,
  primaries)

Brace scopes push/pop typedef object shadows on the session registry. This is an
approximate C scope (brace-matched), not semantic block scope. Do not extend it
by bolting more one-off previous tokens without updating the role model; a
parser-fed context would be the larger redesign if this limit is hit again.

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
  `builtins::BuiltinRegistry` table-drives `__builtin_*` / `va_*` into closed
  `CallPlan` variants;
  `PendingArrayMemberStore` on the AST holds struct member declarators whose
  bounds need a single late re-fold after file-scope symbols exist.
  SA writes `AddressPlan` / `PointerArithPlan` into the annotation store so
  codegen does not re-derive index/field/pointer-scale metadata from AST fields.
- **Codegen IR** - procedure-scoped data IR (`Instruction`, `ir::` builders, `IrPasses`) under `src/codegen/`
  (assign, call, field address, `BuiltinOp`, `VaOp`, etc.); frame packing in
  `FrameLayout`; multi-TU `CodeGeneratingVisitor*` with shared helpers in
  `CodeGeneratingVisitorInternal.h`.
- **FrameLayout** - linear-scan spill slots; `:= &obj` keeps `obj` live through the
  pointer's last use (incl. PARAM→CALL) so multi-word arrays/CLs are not reused early
- **StackMachine** - multi-TU (`StackMachine{Frame,Call,Memory,Va,Arith}.cpp`)
  maps IR instructions to NASM using homes and SysV conventions; multi-word and sret
  policy is centralized in `types/ObjectAbi.h`. Variadic frames use `VaOp` plus a
  compile-time `VariadicFrame` (regSave / overflow Addresses written into the
  `va_list` tag; no text rewrite of `va_start`).

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

Product assign: **`type::productAssignFrom(dest, source)`** only (in
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

Two related notions after semantic analysis (**do not collapse**):

- **`expressionType()`** — C type of the expression (sizeof, `isArray`).
  Array members / multi-dim rows keep the array type here. Throws if unset.
- **`valueType(store)`** — type of the Result-slot symbol after SA (decay temps).
  Prefer for arithmetic, signedness, and codegen value paths. **Requires** a
  Result annotation (fail-closed; no soft fallback to `expressionType()`).

Write protocol:

- **`setTypeAndResult(store, symbol)`** — same type for both (most expressions)
- **`setType(T)` then `setResult(store, symbol)`** — dual ownership: keep array
  / function expression type while Result is a pointer temp (C 6.3.2.1 decay)

Helper: **`isArrayObjectType()`** — `expressionType` is array; CG must keep the
address (no scalar load) for multi-dim rows and member arrays.
**`hasDecayedArrayValue(store)`** — array expression type *and* pointer value
type (true dual ownership after SA).

Index / field / pointer-scale SA→CG facts live in the annotation store as
**closed variants**: `AddressPlan`, `CallPlan`, and `PointerArithPlan`
(`PointerScalePlan` / `PointerDifferencePlan`). Plan children and store keys
use **`NodeRef`**. Field/Index use SA-owned **`AddressBaseMode`**
(`LeaObject` / `PointerValue`; SA fills `baseName` on Field/Index plans). Address temps use sole
**`ValueSlot::Lvalue`**. Registry seeds `BuiltinPlan` or synthetic Direct call (`alloca`→`malloc`).
Field/Index emission is shared; call/`&` dispatch is a single `std::visit`.
Expression dual-type uses **`ValueForm`** (`Scalar` / `AggregateAddress` / `FunctionDesignator`); CG keeps addresses via `holdsAggregateAddress()`.
**`CallPlan`** is shape-only (`Direct`|`Indirect`); callee name from `FunctionEntry` or operand Result. Product builtins use **`BuiltinPlan`**.
Product assign is permanent policy via **`type::productAssignFrom`** (see `TypeQuery.h`).

Product asm is **Intel/NASM only**. `__builtin_va_arg`'s type_name lives on the
`FunctionCall` node (`builtinTypeArgument()`), not a PE/AST side map. CG is
fail-closed when SA plans or required args are missing.

## Host integration

| Concern | Behavior |
| --- | --- |
| Preprocess dialect | `-E -P -std=c99 -x c` |
| Trailing defines | `-D__STDC__=0 -DCURL_DISABLE_TYPECHECK -w` (after user `-D`/`-U`) |
| Assemble | Intel: `nasm -O1 -f elf64`; AT&T: `as --64` (`-a intel|att`) |
| Link | `gcc -m64 -pie` plus objects and user link args |

PIE is the product default: `default rel` (Intel), PLT calls for externals,
GOT loads for external function addresses, and `-pie` at link.

### Variadic frames (`VariadicFrame`)

SysV `va_list` is a 24-byte tag: `gp_offset`, `fp_offset`, `overflow_arg_area`,
`reg_save_area`. A variadic prologue dumps incoming GP and XMM argument
registers into a frame-local save area. `va_start` stores the named-arg gp/fp
offsets and LEAs `regSave` and overflow (`rbp+16` plus named stack words) into
that tag. Nested and concurrent calls do not share
process-wide pointers: each `va_list` holds rbp-relative addresses for its own
frame.

`__builtin_va_list` is a first-class type (24-byte SysV tag array-of-1).
`va_start` / `va_arg` / `va_copy` / `va_end` are frontend builtins that emit
`VaStart`/`VaArg`/`VaCopy`/`VaEnd`. No separate runtime object is linked.

## Source layout

```
src/
  driver/           CLI, configuration, preprocess, host toolchain
  scanner/          FA lexer + registries
  parser/           LALR table, actions
  ast/              nodes + ContextualSyntaxNodeBuilder; ParseEnvironment
                    (tags/typedefs/enums) vs reduction stacks on builder context
  semantic_analyzer/
  codegen/          Instruction IR, StackMachine, Intel NASM, ObjectAbi
  types/            Type, Primitive, Function, struct/union layout
  translation_unit/ Context / source positions
  util/             logging
resources/configuration/
  scanner.lex, grammar.bnf, parsing_table
test/src/
  functionalTest/   end-to-end compile-and-run (themed by area)
  *Test/            unit tests per library
```

## Testing model

- **Functional tests** (`Compiler.*`) compile small C programs with `trans`,
  assemble, link, and check stdout/exit. Grouped by theme
  (e.g. `Global*`, `Structs*`, `Variadic*`, `Preprocessor*`, `DriverModes*`).
- **Driver unit tests** cover host path search, configuration flags, and
  link argv.
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
| SysV `va_list` / nested / concurrent / float packing | `VariadicTest`, `StackMachineTests` |
| Unsupported: multi-word `va_arg` (syntax+link pin; runtime wrong) | `VariadicTest` (`vaArgOfStructIsAcceptedByFrontend`) |
| Assignment type compatibility (`productAssignFrom`) | `TypeTest`, `SemanticErrorsTest` (`assignStructToScalarRejected`) |
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
  Analysis annotations live only in `symbols::AnnotationStore` (one
  `NodeAnnotations` record per node address), owned by `AbstractSyntaxTree`.
  SA is constructed with `AnnotationStore&` and `PendingArrayMemberStore&` (no
  bind/null phase). Codegen takes `AnnotationStore&`. Result **reads** go through
  `Expression::{hasResult,result,valueType}`; other slots use the store. Writes
  use `setTypeAndResult` / `setResult` (dual ownership for sizeof vs value).
  Polymorphic `lvalueAnnotation` remains for address forms. Unary `&` gets an SA-produced `AddressPlan` so codegen emits Field/Index/Lvalue/AddressOf without a `dynamic_cast` forest. Builtin kinds and
  lowering live in leaf package `builtins` (no `ast` include).

- The intermediate form is intentionally thin; ABI detail concentrates in
  `StackMachine` + `ObjectAbi`, not in every IR op type.
- Dual assembly dialects: Intel/NASM (default) and full AT&T/GAS (`-a att`);
  functional tests run both. Older note: AT&T stubs existed; product path is
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
   IR ops if needed, then StackMachine emission; functional test first.
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
