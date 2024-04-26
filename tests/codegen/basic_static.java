public class Main {
    public static short a;
    public static int[] b;

    public Main() {}
    public static int main(String[] args) {
        a = (short) 10;
        b = new int[10];
        b[0] = 10;
        return b[0] + a;
    }
}
