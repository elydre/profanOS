#include <stdio.h>
#include <syscall.h>

int main(int argc, char **argv) {
    // read memory at 0x12345678
    uint32_t *ptr = (uint32_t *) 0x12345000;

    printf("0x12345000: %x\n", *ptr);

    c_serial_print(SERIAL_PORT_A, "TEST\n");
    return 0;
}
