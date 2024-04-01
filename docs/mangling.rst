Java Name Mangling
===============================================================================

Name mangling is the process of encoding the function name in a way that includes the function signature. This is done to generate unique function names to help in cases where the same function name is used to overload different function implementations (i.e., function overloading). The mangled function name includes the return type, the class name, the function name, and the function signature.

For example, the function ``String valueOf(byte)`` in the class ``java.lang.String`` is mangled as ``_JF4java4lang6String7valueOfESb``. 

Function Name Mangling Algorithm
-------------------------------------------------------------------------------

The mangled function name includes the following parts:

#. ``_JF``
#. ``C``, if the function is static
#. :ref:`the function's canonical name encoded <encode-canonical-name>`
#. :ref:`the function's return type encoded <encode-type>`
#. :ref:`the function's argument types encoded <encode-type>`, in order

.. _encode-canonical-name:

Encoding a canonical name
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

A canonical name is encoded as follows:

- It is an array of ``parts``
- The final canonical name is the array of ``parts`` joined by the char ``.``
- Each part is a length-prefixed string, that is, the length of the string followed by the string itself
- The ``parts`` array ends with the character ``E``

.. _encode-type:

Encoding a type
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

A type is encoded as follows:

.. list-table::
    :header-rows: 1

    * - Type
      - Encoding

    * - `boolean`
      - `B`

    * - `char`
      - `c`

    * - `short`
      - `s`

    * - `int`
      - `i`

    * - `byte`
      - `b`

    * - `java.lang.String`
      - `S`

    * - `java.lang.Object`
      - `O`

    * - `void`
      - `v`

    * - array
      - `A` followed by the array type

    * - reference type
      - `R` followed by the canonical name of the type

jccfilt.py
-------------------------------------------------------------------------------

When you run the :ref:`CFG printer pass <running-optimization-pass>`, the output CFG's file name contains the mangled function's name. You can run the following command to unmangle the name:

.. code-block:: bash

    echo <mangled-name> | scripts/jccfilt.py

This will output the unmangled name. For example:

.. code-block:: bash

    echo _JF4java4lang6String7valueOfESb | scripts/jccfilt.py

This should output the unmangled function name ``String java.lang.String.valueOf(byte)``.
