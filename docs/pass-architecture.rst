Pass Architecture
=================

The compiler front-end is built off of individual passes chained together. A **pass** is some code that has dependencies on other passes. The benefit of this system is that we can test components of the compiler individually. This is because, passes can be selectively installed and run. This also lets us detect regressions in previous assignments' public tests.

For instance, **HierarchyChecker** relies on **NameResolver**, since the type names must first be resolved to do hierarchy checking. However, the user working on **HierarchyChecker** can still work on and compile their code without **NameResolver** being complete. Moreover, the user working on **NameResolver** can test their code without **HierarchyChecker** being complete!

Another motivation for having passes is that the compiler front-end allocates objects using a bump allocator (much like Clang). The pass manager is then able to track pass allocation lifetimes and re-use allocators when possible. Bump allocation is a key aspect of our compiler performance. We have observed a consistent 2x improvement in performance over the default C++ allocator.
