#include <unistd.h>

int main(void) {
    char c;
    while (read(0, &c, 1) > 0) {
        if (c >= 'a' && c <= 'z') {
            c -= 32;
        }
        write(1, &c, 1);
    }
    return 0;
}
