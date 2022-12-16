#include <syscall.h>

int main(int argc, char** argv) {
    uint8_t *addr = c_malloc(0x100000);
    uint8_t *addr2 = c_malloc(0x10);
    c_free(addr);
    return 0; 
    // for (int i = 0; i < 1000; i++) {
    //     c_malloc(10);
    // }
    // return 0;
}
