import org.joosc.test.A;

public class Main {
    public Main() {}
    public static void main(String[] args) {
        A a = new A();
        org.joosc.test.B b = new org.joosc.test.B();
    }
}

---

package org.joosc.test;

public class A extends B {
    public A() {
        System.out.println("A");
    }
}

---

package org.joosc.test;

public class B {
    public B() {
        System.out.println("B");
    }
}
