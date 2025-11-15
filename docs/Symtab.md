# kaleidoscope module symbol table

Just imagination.

```txt
$ ks-objdump example.ksm

.symtab

ID NAME  KIND    REF
-- ----  ------- -------------------------
 0 i32   Type    TypeInfo[0]
 1 f64   Type    TypeInfo[1]
 2 _1_t  Type    TypeInfo[2]
 3 +     Infix   OverloadSet["infix +"][0]
 4 -     Infix   OverloadSet["infix +"][1]
 5 pi    DataObj Data["pi"]

.overloads

["infix +"]

<0> function +(i32, i32): i32;
<1> function -(i32, i32): i32;

.types

["::"]

<0> type i32    {.ordinal, internal.};
<1> type f64    {.ordinal, internal.};
<2> type _1_t = callable[(i32, i32): i32];

.text

.data

<0> 3.14'f64

.bss

```
