
# Environment Building

The environment building stage creates environments (containing classes, interfaces, fields, methods, local variables, and formal parameters) for each scope. Given a name of one of these entities, the environment should be able to locate the correct declaration of the entity.

After constructing all of the environments, the following requirements of the Joos 1W language must be checked:

- ✅ No two fields declared in the same class may have the same name.
- ✅ No two local variables with overlapping scope have the same name.
- ✅ No two classes or interfaces have the same canonical name.

# Type Linking
The type linking stage connects each use of a named type (class or interface) to the declaration of the type. At this stage, only names that can be syntactically (according to JLS 6.5.1) determined to be names of types need to be linked. Some names are syntactically ambiguous, in the sense that type checking must be done before it can be determined whether they are names of types or of other entities (see JLS 6.5). These ambiguous names will be linked in a later assignment.

When linking type names, the following requirements of the Joos 1W language must be checked:

- ✅ No single-type-import declaration clashes with the class or interface declared in the same file.
- ✅ No two single-type-import declarations clash with each other.
- ❌ All type names must resolve to some class or interface declared in some file listed on the Joos command line.
- ❌ All simple type names must resolve to a unique class or interface.
- ❌ When a fully qualified name resolves to a type, no strict prefix of the fully qualified name can resolve to a type in the same environment.
- ❌ No package names—including their prefixes—of declared packages, single-type-import declarations or import-on-demand declarations that are used may resolve to types, except for types in the default, unnamed package.
- ❌ Every import-on-demand declaration must refer to a package declared in some file listed on the Joos command line. That is, the import-on-demand declaration must refer to a package whose name appears as the package declaration in some source file, or whose name is a prefix of the name appearing in some package declaration.

# Hierarchy Checking
The fourth stage computes the inheritance relationships for classes, interfaces, methods, and fields, and checks that they conform to the various rules given in Chapters 8 and 9 of the Java Language Specification. Specifically, this stage should check that:

- 🚧 A class must not extend an interface. (JLS 8.1.3)
- 🚧 A class must not implement a class. (JLS 8.1.4)
- 🚧 An interface must not be repeated in an implements clause or in an extends clause of an interface. (JLS 8.1.4)
- 🚧 A class must not extend a final class. (JLS 8.1.1.2, 8.1.3)
- 🚧 An interface must not extend a class. (JLS 9.1.2)
- 🚧 The hierarchy must be acyclic. (JLS 8.1.3, 9.1.2)
- 🚧 A class or interface must not declare two methods with the same signature (name and parameter types). (JLS 8.4, 9.4)
- 🚧 A class must not declare two constructors with the same parameter types (JLS 8.8.2)
- 🚧 A class or interface must not contain (declare or inherit) two methods with the same signature but different return types (JLS 8.1.1.1, 8.4, 8.4.2, 8.4.6.3, 8.4.6.4, 9.2, 9.4.1)
- 🚧 A class that contains (declares or inherits) any abstract methods must be abstract. (JLS 8.1.1.1)
- ❌ A nonstatic method must not replace a static method. (JLS 8.4.6.1)
- ❌ A static method must not replace a nonstatic method. (JLS 8.4.6.2)
- ❌ A method must not replace a method with a different return type. (JLS 8.1.1.1, 8.4, 8.4.2, 8.4.6.3, 8.4.6.4, 9.2, 9.4.1)
- ❌ A protected method must not replace a public method. (JLS 8.4.6.3)
- ❌ A method must not replace a final method. (JLS 8.4.3.3)

Note: 🚧 means untested.
