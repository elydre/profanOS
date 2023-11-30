#include <stdlib.h>
#include <stdio.h>

int main(int argc, char **argv) {
    char **envp = getfullenv();

    for (int i = 0; envp[i]; i++) {
        printf("%s\n", envp[i]);
    }

    return 0;
}
