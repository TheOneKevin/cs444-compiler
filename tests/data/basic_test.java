public class Main {
    public Main() {}
    public static int test(int j) {
        int ax = 0;
        int bx = ax = 123;
        ax = 456; // dead assignment to ax
        return bx;
    }
}

// /u7/ll2wu/cs444/cs444-compiler/build/joosc
// /u/cs444/pub/stdlib/2.0/java/util/Arrays.java
// /u/cs444/pub/stdlib/2.0/java/lang/Byte.java
// /u/cs444/pub/stdlib/2.0/java/lang/Object.java
// /u/cs444/pub/stdlib/2.0/java/lang/Short.java
// /u/cs444/pub/stdlib/2.0/java/lang/System.java
// /u/cs444/pub/stdlib/2.0/java/lang/String.java
// /u/cs444/pub/stdlib/2.0/java/lang/Number.java
// /u/cs444/pub/stdlib/2.0/java/lang/Integer.java
// /u/cs444/pub/stdlib/2.0/java/lang/Boolean.java
// /u/cs444/pub/stdlib/2.0/java/lang/Cloneable.java
// /u/cs444/pub/stdlib/2.0/java/lang/Character.java
// /u/cs444/pub/stdlib/2.0/java/lang/Class.java
// /u/cs444/pub/stdlib/2.0/java/io/Serializable.java
/// /u/cs444/pub/stdlib/2.0/java/io/OutputStream.java
// /u/cs444/pub/stdlib/2.0/java/io/PrintStream.java