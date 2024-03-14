public class basic_test_owen {
    public basic_test_owen() {}
    public static int test(int j) {
        if ((10 != 0) && (true || false)) { // true
            x = 0;
        } else {
            y = 0;
        }

        if ((!true || (3 <= 2)) && ((7 % 2) == 0)) { // false
            x = 0;
        } else {
            y = 0;
        }

        if (true) {
            abc = 0;
        } else {
            zyx = 0;
        }

        while(false);

        while (1 == 123) {
            a = 0;
        }

        while (1 == 1) {
            a = 0;
        }
        
        for (int i = 0; ; i = i + 1) {
            a = a + 1;
        }
        
        for (int i = 0; false; i = i + 1) {
            a = a + 1;
        }
        
        return 0;
    }
}
