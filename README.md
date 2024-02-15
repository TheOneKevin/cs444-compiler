# JOOS1W Compiler Framework {#mainpage}

Compiler project for the University of Waterloo CS444/644 compiler construction course.
The Joos1W language is based off the Java 1.3 language.
See here for a complete feature table of Joos:

- https://student.cs.uwaterloo.ca/~cs444/joos/features/
- [Java Language Specification (2nd Edition)](https://web.archive.org/web/20111225035254/http://java.sun.com:80/docs/books/jls/second_edition/html/jTOC.doc.html)

Our project is written in C++ with Flex and Bison as our lexer and parser generators.
To build the project, simply run:

```
mkdir build
cd build
cmake .. && make
```

Make sure you are either using Clang 16 (preferred) or later or GCC 12 or later.
There is currently [a bug in older GCC compilers](https://gcc.gnu.org/bugzilla/show_bug.cgi?id=85282)
that would not compile this project. Our project directory structure is:

- `lib/`: Contains the core compiler libraries. Includes AST, parser grammar and other files.
- `tests/`: Contains the unit test drivers and data files.
- `tools/`: Contains the frontends -- these are the programs you actually can run.

## 1. Compiler component division

The below table documents the "owners" of different parts of the compiler.

<table><tr><th>Larry</th><th>Owen</th><th>Kevin</th></tr><tr>
    <!-- Larry -->
    <td><ul>
        <li>Lexer comment parsing</li>
        <li>Initial AST semantic checking rules, mainly for DeclContext AST nodes</li>
        <li>Expr AST nodes and visitor</li>
        <li>(TODO) Expr evaluation + type checking</li>
    </ul></td>
    <!-- Owen -->
    <td><ul>
        <li>joos1c compiler frontend</li>
        <li>Makefile + Marmoset submissions</li>
        <li>Most of the parser rules (Stmt + Decl)</li>
        <li>Stmt AST nodes and visitor</li>
    </ul></td>
    <!-- Kevin -->
    <td><ul>
        <li>CMake build flow + boring infrastructure</li>
        <li>Expr parser rules</li>
        <li>Remainder of AST nodes and visitor</li>
        <li>(TODO) Symbol table and name resolution API</li>
    </ul></td>
</tr></table>

A part of the compiler is broken? Go find who to blame here 😇

## 2. Lexer and parser design

The grammar files are located under `lib/grammar/*.l` and `*.y` for the Flex
and Bison files respectively. Some notes about our Flex lexer below. The accompanying
`joos1w_lexer_internal.h` file contains the remainder + definition of the lexer class
that Flex expects to be defined.

1. The Flex lexer is generated as a C++ class by the option `%option c++ yyclass="Joos1WLexer"`.
   This allows the lexer to be re-entrant and not rely on global state.
2. Line and column location must be manually kept track of by `YY_USER_ACTION` macro
3. Each lexer rule creates a parse tree leaf node via the `make_*()` functions. Leaf nodes
   that are not intended to be part of a parse tree are poisoned to catch incorrect parser rules.
4. Since we're generating a C++ class, a Bison-compatible interface must be generated manually. See `Joos1WLexer::bison_lex`.

Some notes about the Bison grammar file `joos1w_parser.y`:

1. Our grammar is written based off the JLS provided rules.
2. The Bison grammar is reentrant by declaring `%define api.pure full`
3. The grammar file contains minimal code to construct the parse tree except in 2 cases:

   - A cast expression must be checked for validity
   - A literal number must be checked for range

   These cases exist as they could not be written into the parser (or we did not know how).
   They were also not complex enough to warrant a place in the AST semantic checker.

Now we can talk about the parse tree design. Our parse tree lives under `lib/parsetree/`
and is mainly defined by the `parsetree::Node` class. Some notes about the tree:

1. The parse tree is untyped, allowing for simpler parser actions. This is desirable
   as Bison is really difficult to debug.
2. The basic structure of a parse tree is represented by the S-expression
   `(NodeType child1 ... childN)`. However, leaf nodes will contain additional
   data, and so they are subclassed from `parsetree::Node`.
3. Since the parse tree is untyped, it must be validated on-the-fly by the `parsetree::ParseTreeVisitor` class.
   This visitor class is also responsible for generating the AST.

## 3. AST design and diagnostics

🚩 WIP Section 🚩

How to define a new AST node:

1. Define the node in the correct header file under `lib/ast/`. They should
   subclass one or more of `ast::Decl`, `ast::DeclContext`, `ast::Stmt` or `ast::Type`.
2. Teach `ast::Semantic` how to construct the node, under `lib/semantic`. See existing
   `Build*` functions for examples.
3. Define how to build the AST node from the parse tree by adding a visitor to the
   `parsetree::ParseTreeVisitor` class. Make sure the visitor is reachable from the root
   visitor (visitCompilationUnit).
4. Don't forget to override the virtual functions from `AstNode` such as those for printing.
5. Run `astprinter` on a test program and check the output!

**⚠️ Expressions should not be an AST node, instead they should be part of an AST node
such as an `ast::Stmt`.**

## 4. Symbol resolution design

TODO(kevin)
