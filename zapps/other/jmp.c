#include <setjmp.h>
#include <stdio.h>

void jmpfunction(jmp_buf env_buf) {
    printf("Jump function\n");
    longjmp(env_buf, 42);
}

int main(int argc, char **argv) {
    int val;
    jmp_buf env_buffer;

    /* save calling environment for longjmp */
    val = setjmp(env_buffer);

    if( val != 0 ) {
        printf("Returned from a longjmp() with value = %d\n", val);
        return 0;
    }

    printf("Jump function call\n");
    jmpfunction(env_buffer);

    return 0;
}
