package com.example;

public class LocalScope {
  public LocalScope() {
    int i;
    {
      String j;
      j = "Hello";
    }
    {
      int j;
      j = 10;
      i = j;
    }
    {
      int k = i;
      {
        char j = 'a';
        k = j;
      }
    }
  }
}
