## Modifications from original TIR standard

We will keep 8 out of the 14 original TIR commands.
Our IR will be a strict subset of TIR to aid in easier code generation,
instruction selection, and to avoid the extra canonicalization step
that TIR mandates.

1. MEM -> Renamed to `load`
2. MOVE -> Subset to memory only, renamed to `store`
3. NAME -> Unused
4. CONST
5. ~~TEMP~~ -> Replaced by the Value/Use semantics
6. OP
7. CALL
8. ~~ESEQ~~ -> Removed, adopt canonical TIR by using `SEQ` only
9. ~~EXP~~ -> Replaced by the Value/Use semantics
10. SEQ -> Replaced by `BasicBlock`
11. JUMP -> Unused
12. CJUMP -> Rename to `br`
13. ~~LABEL~~ -> Removed, use `BasicBlock`
14. RETURN

Most of the changes are motivated by describing and enforcing canonical TIR form.

We should note that ESEQ is already canonicalized away into `SEQ`.
The modifications and removal of `TEMP`, `EXP`, `SEQ` and `LABEL` are motivated
by having every expression be a temporary. Every expression must have 1 side
effect is a behaviour mandated by the canonical TIR form. And this modification
enforces that.

The remainder of the changes are renaming. For instance,
the changes to `MOVE` are due to `TEMP` being removed. Hence, the only possible
`MOVE` must be memory moves.
Also, CJUMP is renamed to `br` and `MEM` to `load`. This is only sane.

## Addition of new instruction: alloca

A new instruction called `alloca` is added to allocate stack space.
This is exactly like `temp` but forcibly moves variables onto the stack
as memory references. This technically can be achieved using `malloc` but
having it in this form is easier to analyze.

For example, original TIR code:
```
SEQ(
    MOVE(TEMP(%1), CONST(i32 1))
)
```

Modified TIR code with alloca uses:
```
SEQ(
    MOVE(TEMP(%1), ALLOCA(i32)),
    MOVE(MEM(TEMP(%1)), CONST(i32 1))
)
```
Our version of canonical TIR in move-less tree form:
```
SEQ(
    STORE(
        ALLOCA(i32),
        CONST(i32 1)
    )
)
```

## Example IR translations

Original JOOS1W Code:

```
void foo() {
    if(c) {
        x = 1;
    } else {
        x = 2;
    }
    x = x + 1;
    f(x);
    return;
}
```

IR with PHI nodes:

```
bb0:
    br %c, bb1, bb2

bb1:
    %x.bb1 = i32 1
    br i32 1, bb3, bb3

bb2:
    %x.bb2 = i32 2
    br i32 1, bb3, bb3

bb3:
    %x = phi(%x.bb1 from bb1, %x.bb2 from bb2)
    %1 = i32 1
    %2 = add %x, %1
    f(%2)
```

Our TIR code, it has no PHI nodes and is in SSA form:

```
bb0:
    %x.ref = alloca i32
    br %c, bb1, bb2

bb1:
    %1 = i32 1
    %2 = store %x.ref, %1
    br i32 1, bb3, bb3

bb2:
    %3 = i32 2
    %4 = store %x.ref, %4
    br i32 1, bb3, bb3

bb3:
    %5 = load %x.ref
    %6 = i32 1
    %7 = add %5, %6
    %8 = store %x.ref
    %9 = load %x.ref
    call f, %9
    ret
```

We should note that our TIR code also has a tree form. But the tree form is
quite hard to read. Hence, we present it in the form above.
Alternatively, we can apply a set of transformations to obtain legal TIR
again from our representation.

Legal TIR translated from our version of TIR:

```
SEQ(
    LABEL(bb0),
    MOVE(TEMP(%x.ref), CALL(__alloca, i32)),
    CJUMP(TEMP(%c), LABEL(bb1), LABEL(bb2)),
    LABEL(bb1),
    MOVE(TEMP(%1), CONST(i32 1)),
    MOVE(TEMP(%2), STORE(TEMP(%x.ref), TEMP(%1))),
    CJUMP(CONST(i32 1), LABEL(bb3), LABEL(bb3)),
    LABEL(bb2),
    MOVE(TEMP(%3), CONST(i32 2)),
    MOVE(TEMP(%4), STORE(TEMP(%x.ref), TEMP(%3))),
    CJUMP(CONST(i32 1), LABEL(bb3), LABEL(bb3)),
    LABEL(bb3),
    MOVE(TEMP(%5), LOAD(TEMP(%x.ref))),
    MOVE(TEMP(%6), CONST(i32 1)),
    MOVE(TEMP(%7), OP(op_add, TEMP(%5), TEMP(%6))),
    MOVE(TEMP(%8), STORE(TEMP(%x.ref))),
    MOVE(TEMP(%9), LOAD(TEMP(%x.ref))),
    CALL(f, TEMP(%9)),
    RETURN()
)
```
