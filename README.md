<div align="center">
<img src="docs/assets/logo.svg" width="100px">
<h1>jCF - j/Compiler Framework</h1>
</div>

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
that would not compile this project. In addition, here is a summary of the dependencies:

1. `CMake` >= 3.21
2. `Clang` >= 17 for C++23 support
3. `libstdc++-12-dev` or later (must have C++23 support)
4. `flex` >= 2.6.4 and `bison` >= 3.8.2

In addition, if you wish to build the debug version, you will need:
1. `xsltproc` (for debug builds)
2. One of `libdw-dev`, `binutils-dev` or `libdwarf-dev` (for backward.cpp)

Our project directory structure is:

- `lib/`: Contains the core compiler libraries. Includes AST, parser grammar and other files.
- `tests/`: Contains the unit test drivers and data files.
- `tools/`: Contains the frontends -- these are the programs you actually can run.
