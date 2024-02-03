# Assignment 1 Manual Test Suite

Rules from A1:

1. **(DRIVER)** All characters in the input program must be in the range of 7-bit ASCII (0 to 127).
2. **(AST)** A class cannot be both abstract and final. (done)
3. **(AST)** A method has a body if and only if it is neither abstract nor native. (done)
4. **(AST)** An abstract method cannot be static or final. (done)
5. **(AST)** A static method cannot be final. (done)
6. **(AST)** A native method must be static. (done)
7. **(PARSER)** The type void may only be used as the return type of a method.
8. **(DRIVER)** A class/interface must be declared in a .java file with the same base name as the class/interface.
9. **(PARSER)** An interface cannot contain fields or constructors.
10. **(PARSER)** An interface method cannot be static, final, or native.
11. **(PARSER)** An interface method cannot have a body.
12. **(AST)** Every class must contain at least one explicit constructor. (done)
13. **(AST)** No field can be final. (done)
14. **(PARSER)** No multidimensional array types or multidimensional array creation expressions are allowed.
15. **(PARSER)** A method or constructor must not contain explicit this() or super() calls.
16. Any other tests we want to add is added here...

Each rule is labelled as DRIVER/PARSER/AST depending on which stage of the compiler is capable of rejecting such rules. They are explained below:
- DRIVER: the program run to load the file and call the frontend stages (i.e., lexing, parsing and such).
- PARSER: the Bison-generated grammar
- AST: the semantic analysis run on the AST

The structure of the test data is as follows:
```
rXX_invalid/valid_name.java
```

Where `XX` is the rule number the test case is meant to test and `name` is some name describing the test case. For example, the test case named
```
r10_invalid_intf_native_method.java
```
is meant to test rule #10, and the test case is testing interfaces that have native methods. The `invalid` indicates this test is successful if the compiler rejects the program.
