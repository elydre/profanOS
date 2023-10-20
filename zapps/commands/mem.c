#include <profan.h>
#include <stdio.h>

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

    if (argc < 2 || argc > 3) {
        printf("$BUsage: $3mem <address> <size>$$\n");
        return 1;
    }
    addr = (uint32_t) str_to_int(argv[1]);
    if (addr == (uint32_t) -1) {
        printf("$3%s$B is not a valid address$$\n", argv[1]);
        return 1;
    }
    if (argc == 2)
        size = 512;
    else
        size = str_to_int(argv[2]);
    if (size < 1) {
        printf("$3%s$B is not a valid size$$\n", argv[2]);
        return 1;
    }
    profan_print_memory((void *)addr, size);
    return 0;
}
