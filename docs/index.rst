jcc1 Compiler Framework
================================================================================

.. toctree::
   :maxdepth: 2
   :hidden:

   driver
   frontend-internals
   pass-architecture
   mangling


.. |nbsp| unicode:: 0x2007 0x2007
   :trim:

Design Overview
--------------------------------------------------------------------------------

This section provides an overview of the directory structure under ``lib/``, which contains the core compiler functionality:

.. list-table::
   :widths: 30 70
   :header-rows: 0

   * - :fa:`folder-open-o` ``ast/``
     - AST node definitions
  
   * - :fa:`folder-open-o` ``grammar/``
     - Flex and Bison grammar files, and support classes
   
   * - :fa:`folder-open-o` ``parsetree/``
     - Parse tree node definitions and visitor pattern

   * - :fa:`folder-open-o` ``passes/``
     - Compiler pass definitions. The important ones are:

   * - |nbsp|:fa:`folder-open-o` ``transform/``
     - All IR transformation passes

   * - |nbsp|:fa:`file-o` ``CompilerPasses.cc``
     - Front-end parsing and AST builder passes

   * - |nbsp|:fa:`file-o` ``IRContextPass.cc``
     - Pass to hold the IR context lifetime

   * - |nbsp|:fa:`file-o` ``SemanticPasses.cc``
     - Semantic analysis passes

   * - :fa:`folder-open-o` ``semantic/``
     - Semantic analysis passes, checks the semantics of JOOSC code.
     
   * - |nbsp|:fa:`file-o` ``AstValidator.cc``
     - Misc checks on AST (i.e., if condition must be bool, etc.)
   
   * - |nbsp|:fa:`file-o` ``CFGBuilder.h``
     - Builds the control flow graph (CFG) for dataflow analysis
     
   * - |nbsp|:fa:`file-o` ``DataflowAnalysis.cc``
     - Dataflow analysis
  
   * - |nbsp|:fa:`file-o` ``ExprStaticChecker.h``
     - Checks expressions for illegal static/non-static uses
     
   * - |nbsp|:fa:`file-o` ``ExprResolver.cc``
     - Huge file, resolves names in expressions + misc. checks
  
   * - |nbsp|:fa:`file-o` ``ExprTypeResolver.cc``
     - Context-free type checks on a single expression
  
   * - |nbsp|:fa:`file-o` ``HierarchyChecker.cc``
     - Builds + validates class inheritance hierarchy
  
   * - |nbsp|:fa:`file-o` ``NameResolver.cc``
     - Builds import + package resolution trees for each CU
    
   * - |nbsp|:fa:`file-o` ``Semantic.cc``
     - AST node builder + basic semantic checking

   * - :fa:`folder-open-o` ``third-party/``
     - Contains CLI11 command arguments parser and backtrace libraries
  
   * - :fa:`folder-open-o` ``utils/``
     - Support code, like exceptions and macros.

The joosc compiler as well as a jcc1 compiler are under ``tools/``. ``jcc1`` is used to test locally as it has more verbose error reporting and a rich command line feature set. ``joosc`` is only for submission.

