#include <stdlib.h>
#include <i_iolib.h>

void print_state(int is_fine, char* name);

int main(int argc, char **argv) {
    int is_fine;
    
    fsprint("Testing stdlib.h\n");

    // test of calloc
    is_fine = 1;
    int *x = calloc(1, sizeof(int));
    *x = 10;
    if (*x != 10) is_fine = 0;
    free(x);
    print_state(is_fine, "calloc");

    // test of malloc
    is_fine = 1;
    int *y = malloc(sizeof(int));
    *y = 10;
    if (*y != 10) is_fine = 0;
    print_state(is_fine, "malloc");

    // test of realloc
    is_fine = 1;
    y = realloc(y, 2*sizeof(int));
    *(y+1) = 20;
    if (*(y+1) != 20) is_fine = 0;
    if (*y != 10) is_fine = 0;
    print_state(is_fine, "realloc");
    free(y);

    // test of a64l
    is_fine = 1;
    if (a64l("./") != 64) {is_fine = 0; fsprint("1 is not okay !\n");}
    if (a64l("") != 0) {is_fine = 0; fsprint("2 is not okay !\n");}
    if (a64l("/") != 1) {is_fine = 0; fsprint("3 is not okay !\n");}
    if (a64l("FT") != 2001) {is_fine = 0; fsprint("4 is not okay !\n");}
    if (a64l("zzzzz1") != (long int) 0xffffffff) {is_fine = 0; fsprint("5 is not okay !\n");}
    if (a64l("zzzz1") != 0x3ffffff) {is_fine = 0; fsprint("6 is not okay !\n");}
    if (a64l("zzz1") != 0xfffff) {is_fine = 0; fsprint("7 is not okay !\n");}
    if (a64l("zz1") != 0x3fff) {is_fine = 0; fsprint("8 is not okay !\n");}
    if (a64l("z1") != 0xff) {is_fine = 0; fsprint("9 is not okay !\n");}
    if (a64l("1") != 0x3) {is_fine = 0; fsprint("10 is not okay !\n");}
    print_state(is_fine, "a64l");

    return 0;
}

void print_state(int is_fine, char* name) {
    if (is_fine) {
        fsprint("$1%s$7: $1OK$7\n", name);
    } else {
        fsprint("$3%s$7: $3NOT OK$7\n", name);
    }
}