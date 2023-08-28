#include <stdio.h>

void print_memory_contents(void *addr, int size) {
    for (int i = 0; i < size / 16; i++) {
        printf("%08x: ", (unsigned int)addr + i * 16);
        for (int j = 0; j < 16; j++) {
            if (i * 16 + j < size)
                printf("%02x ", *((unsigned char *)addr + i * 16 + j));
            else
                printf("   ");
            if (j % 4 == 3)
                printf(" ");
        }
        for (int j = 0; j < 16; j++) {
            unsigned char c = *((unsigned char *)addr + i * 16 + j);
            if (i * 16 + j >= size)
                break;
            if (c >= 32 && c <= 126) {
                printf("%c", c);
            } else {
                printf(".");
            }
        }
        printf("\n");
    }
}

int str_to_int(char *str) {
    int ret = 0;
    if (str[0] != '0' || str[1] != 'x') {
        for (int i = 0; str[i]; i++) {
            if (str[i] >= '0' && str[i] <= '9')
                ret = ret * 10 + str[i] - '0';
            else if (str[i] != ' ')
                return -1;
        }
        return ret;
    }
    for (int i = 2; str[i]; i++) {
        if (str[i] >= '0' && str[i] <= '9')
            ret = ret * 16 + str[i] - '0';
        else if (str[i] >= 'a' && str[i] <= 'f')
            ret = ret * 16 + str[i] - 'a' + 10;
        else if (str[i] >= 'A' && str[i] <= 'F')
            ret = ret * 16 + str[i] - 'A' + 10;
        else if (str[i] != ' ')
            return -1;
    }
    return ret;
}

int main(int argc, char *argv[]) {
    uint32_t addr;
    int size;

    if (argc < 3 || argc > 4) {
        printf("$BUsage: $3mem <address> <size>$$\n");
        return 1;
    }
    addr = (uint32_t) str_to_int(argv[2]);
    if (addr == (uint32_t) -1) {
        printf("$3%s$B is not a valid address$$\n", argv[2]);
        return 1;
    }
    if (argc == 3)
        size = 512;
    else
        size = str_to_int(argv[3]);
    if (size < 1) {
        printf("$3%s$B is not a valid size$$\n", argv[3]);
        return 1;
    }
    print_memory_contents((void *)addr, size);
    return 0;
}
