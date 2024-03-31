public class Main {
    public Main() {}
    public static int main(String[] args) {
        short a = (short) -1;
        byte b = (byte) -1;
        short c = (short) 1;
        byte d = (byte) 1;
        char e = (char) (a + b + c + d); // 0
        return e;
    }
}
