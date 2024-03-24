#include <stdlib.h>
#include <stdio.h>

int main(int argc, char **argv, char **envp) {
    for (int i = 0; envp[i]; i++) {
        printf("%s\n", envp[i]);
    }

    return 0;
}
