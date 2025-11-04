# Full language reference guide

module preamble
  doc
    @brief Core language module.
  implement
    pub type bool    {.internal.}; # internal means: "Provided by runtime/compiler"

    pub type byte    {.internal.};
    pub type i8      {.internal.};
    pub type i16     {.internal.};
    pub type i32     {.internal.};
    pub type i64     {.internal.};
    pub type u8      {.internal.};
    pub type u16     {.internal.};
    pub type u32     {.internal.};
    pub type u64     {.internal.};
    pub type f32     {.internal.};
    pub type f64     {.internal.};

    pub type char    {.internal.};
    pub type string  {.internal.};
    pub type cstring {.internal.};

    concept Numeric = byte | i8 | i16 | i32 | i64 | u8 | u16 | u32 | u64 | f32 | f64;

    prefix   -[T: Numeric](self: T): T {.internal.};
    prefix   +[T: Numeric](self: T): T {.internal.};

    prefix  --[T: Numeric](mut self: ref T): ref T {.internal.};
    prefix  ++[T: Numeric](mut self: ref T): ref T {.internal.};

    postfix --[T: Numeric](mut self: ref T): T {.internal.};
    postfix ++[T: Numeric](mut self: ref T): T {.internal.};

    infix    +[T: Numeric](lhs: T, rhs: T): T {.internal, priority=20, associativity="left".}; # basic math for core numeric types
    infix    -[T: Numeric](lhs: T, rhs: T): T {.internal, priority=20, associativity="left".};
    infix    *[T: Numeric](lhs: T, rhs: T): T {.internal, priority=10, associativity="left".}; # priority lesser is higher.
    infix    /[T: Numeric](lhs: T, rhs: T): T {.internal, priority=10, associativity="left".};
end

from preamble import *; # by default for every source listing

type int = i32; # typedef
type cel {.distinct.} = i32; # distinct type

type MyStruct = struct
    property a(set_a, get_a): i32; # Property always public.
                                   # It is just field but accessed
                                   # only via setter/getter func with
                                   # assignment syntax. Field look and feel.

    pub b: i64; # public field

    c: i8; # private field
  doc
    @brief This is structural memory layout product type.

    It has public property `a`, public field `b` and public field `c`.
    It has default value semantic and RAII behavior.
  implement # only for type behavior specific
    initialize {.default.};
    clone      {.default.};
    move       {.default.};
    dispose    {.default.};

    function set_a(self, v: i32): void = self.a = v;
    function get_a(self)        : i32  = self.a;
end

type AnyNum = any_of[i64, u64, f64]; # Sum type memory layout of tagged union.

function main
  doc
    @brief Program entry point
  do
    var s: MyStruct; # stack value
    var ref_s: owned MyStruct = new MyStruct(); # heap managed nullable value
    var value = ref_s?.a ?: 0; # safe
    var roll = ref_s!.a; # maybe panic if ref_s is nil

    # RAII automatic cleanup:
    # ref_s.dispose()
    # s.dispose()
end
