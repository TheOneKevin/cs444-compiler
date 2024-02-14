package com.example;

public class HelloWorld {
  public HelloWorld() {}
  public int test = another.object().method();
  public int nullField = null;
  public static void main(String[] args) {
    a.b("hello").y = b.c();
  }

  // [ a b methodAccess c d methodAccess methodInvocation "hello" methodInvocation y memberaccess 5 = ]
}
