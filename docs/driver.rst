jcc1 Driver Program
--------------------------------------------------------------------------------

The driver program is the entry point for the compiler. It is responsible for parsing command line arguments, reading input files, and running the compiler passes. The program is located in ``tools/jcc1/main.cpp``.

Options
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

The table below is just a summary. For a complete list of options, run ``jcc1 -h``.

.. option:: -h,--help

  Print a help message and exit

.. option:: -v verbosity_level

Set the verbosity level, an integer in [0,3]

.. option:: -c

  Compile-only, running all the front-end passes

.. option:: -s

  Run the front-end passes, then generate IR code. Implies ``-c``

.. option:: -o output_file

  Output the generated code to this file

.. option:: --stdlib stdlib_dir

  The path to the standard library to use for compilation

.. option:: --print-dot

  If a printing pass is run, print any trees in DOT format

.. option:: --print-output output_file

  If a printing pass is run, will write the output to this file or directory

.. option:: -p,--pipeline pipeline_string

  The pipeline string to run.

Pipeline String
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

The driver program can run a sequence of passes. You can specify a comma separated list of passes to run. Optimization passes are run in-order and may be specified more than once. Frontend passes are always run first, and run only once.

Learn more about the pass architecture in :doc:`pass-architecture`.

Front-end Passes
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

A front-end pass takes in an AST and stores the its analysis result, which can be queried by other front-end passes.

.. list-table::

  * - ``dfa``
    - Dataflow analysis

  * - ``sema-expr``
    - Expression Resolution

  * - ``sema-hier``
    - Hierarchy Checking

  * - ``sema-name``
    - Name Resolution

  * - ``print-ast``
    - Print the AST

Optimization Passes
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

An optimization pass takes in an IR and returns a new IR with the optimization applied.

🚧 WIP 🚧

Example Usage
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

**Running a Front-end Pass**

To run just one front-end pass, you can use the ``--pipeline`` option. For instance, to run the name resolution pass, you can use:

.. code-block:: console

  $ jcc1 test.java --pipeline sema-name

This will run just the name resolution pass. 

You can also run just the optimization passes by using the ``-p`` option. For instance, to run the CFG printer pass, you can use:

.. code-block:: console

  $ jcc1 test.java -p printcfg

This will run just the CFG printer pass.

.. _running-optimization-pass:

**Running an Optimization Pass**

You can also run multiple passes by separating them with a comma. For instance, to run the CFG printer pass, followed by the simplifycfg optimization pass, and then again the CFG printer pass, you can use:

.. code-block:: console

  $ jcc1 test.java -p printcfg,simplifycfg,printcfg

This will run the CFG printer pass, followed by the simplifycfg optimization pass, and then again the CFG printer pass. The CFG printer pass will output the CFG of the function in DOT format.

CFG prints are output in a file, with the name of the form ``<i>.<mangled function name>.dot``,  for the ``i``-th time the pass is invoked. To learn more, see :doc:`mangling`.

**Running from stdin vs from file**

By default, the driver program reads from a file. However, you can also read from stdin by using the ``-`` option. For instance, to read from stdin, you can use:

.. code-block:: console

  $ cat test.java | jcc1 -p sema-name

This will run the name resolution pass on the input read from stdin.

**Printing the AST**

To print the AST, simply run the ``print-ast`` pass. By default, the AST will be dumped in a text-based format into standard output. Using ``--print-dot`` will force it to print in DOT format. Using ``--print-output out.dot`` will write the output (whether DOT or text-based) to the output file. For example,

.. code-block:: console

  $ jcc1 test.java -p print-ast --print-dot --print-output out.dot

This will print the AST of the input file in DOT format to ``out.dot``.

Java Standard Library
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

By default, the driver program uses the standard library version 6.1 on the student environment. However, you can specify your own Java standard library to use. You must compile with a standard library.
