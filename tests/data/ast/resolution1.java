package org.joosc.test;

public class A extends B {
    public A() {
        System.out.println("A");
    }
}

---

package org.joosc.test;

public class B extends A {
    public B() {
        System.out.println("B");
    }
}
