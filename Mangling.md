This document is for mangling Java names.

A function name is encoded as follows:

1. `_JF` is emitted
2. If the function is static, then `C` is emitted
3. The function's canonical name is encoded. See canonical name encoding below.
4. The return type is emitted. See type encoding below.
5. Each of the parameter types are emitted in-order.

A canonical name is encoded as follows:

- It is an array of `parts`
- The final canonical name is the array of `parts` joined by the char `.`
- Each part is a length-prefixed string i.e., a number followed by that
  many characters of string.
- The `parts` array ends with the character `E`

A type is encoded as follows:

- `B` if the type is a `boolean`
- `c` if the type is a `char`
- `s` if the type is a `short`
- `i` if the type is a `int`
- `b` if the type is a `byte`
- `S` if the type is a `java.lang.String`
- `O` if the type is a `java.lang.Object`
- `v` if the type is a `void`
- If the type is an array, `A` is emitted followed by the array type.
- If the type is a reference type, `R` is emitted followed by the canonical
  name of the type
