# Kaleidoscope (by Novikov Vladimir)

## Goal

Learn to develop custom programming languages - from syntactic analysis
and AST construction to runtime environments, and even compilation to
bytecode for an arbitrary virtual machine.

I am following the main [Kaleidoscope
tutorial](https://llvm.org/docs/tutorial/MyFirstLanguageFrontend/) from
LLVM, but intentionally deviate in implementation details to meet my own
objectives. Key differences include:

- The language must be embeddable into C++ projects (inspired by
  [Lua/sol2](https://github.com/ThePhD/sol2));
- Naming rules for language operators and function/data object
  identifiers are strictly defined by the grammar and can be
  unambiguously recognized during parsing. The implementation of
  operator handlers (including precedence and associativity) is not
  hardcoded into the language! The runtime environment must
  independently register the necessary handlers for built-in functions
  and operators during context initialization;
- The execution phase and runtime environment are strictly separated
  from the parsing phase. This means that during AST construction, it is
  impossible to determine the correct order of operations in a
  mathematical expression. This resolution must occur later within the
  execution context, which contains information about registered
  operator handlers, built-in functions, and data objects.

## Architecture Overview

The system follows a staged processing model:

1. **Input**: Source code in Kaleidoscope.
2. **Parsing**: The parser processes the input.
3. **Syntax Analysis**: Produces an initial, precedence-agnostic AST.
4. **Context Initialization**: The `InterpreterContext` is supplied with
   registered operators and functions.
5. **Preprocessing**: The AST is transformed into an executable form
   using context information.
6. **Evaluation**: The prepared AST is evaluated within the
   `InterpreterContext`.
7. **Result**: The final expression result is returned.

Although the AST can be executed directly, a project goal is to
implement an alternative approach: using an LLVM-based code generation
module to translate the AST into efficient bytecode for a virtual
machine.

## Learn kaleidoscope in Y minutes

```kaleidoscope
# single line comment
# global module

type i32 {.ordinal, internal.}; # declaration of embedded type

# priority less is most.

operator`+`(lhs: i32, rhs: i32): i32 {. internal, priority=20, associativity=left .}; # implementations internals
operator`-`(lhs: i32, rhs: i32): i32 {. internal, priority=20, associativity=left .}; # implementations internals
operator`*`(lhs: i32, rhs: i32): i32 {. internal, priority=10, associativity=left .}; # implementations internals
operator`/`(lhs: i32, rhs: i32): i32 {. internal, priority=10, associativity=left .}; # implementations internals

module ctypes =
    export type int = i32; # synonim for type 'i32'

# global module again

import * from ctypes;

function entry(): int =
    var num: int; # -> num: 0
    num = 42;     # -> num: 42

    let num_: int = 7;
    num_ = 42; # -> error: num_ is immutable data object

    num - num_ * 5 / 2 # returned a value of last statement

entry() # call a function
```
