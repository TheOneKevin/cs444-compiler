public class Main {
    public Main() {}
    public static int main(String[] args) {
        int x = 0;
        int y = 0;
        if (args.length > 0) {
            x = 1;
        } else {
            y = 1;
        }
        if(x > 0) {
            y = x + y;
        } else {
            x = x + y;
        }
        for(int i = 0; i < 10; i = i + 1) {
            if(x < 0) {
                x = x + y;
            }
        }
        return x + y;
    }
}
