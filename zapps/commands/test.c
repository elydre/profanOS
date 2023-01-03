#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

#define TEST_ABORT 0
#define TEST_ATOF 0
#define TEST_ATEXIT 0

void print_state(int is_fine, char* name);

int main(int argc, char **argv) {
    int is_fine;
    
    printf("Testing stdlib.h\n");

    // test of calloc
    is_fine = 1;
    int *x = calloc(1, sizeof(int));
    if (*x != 0) is_fine = 0;
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
    if (a64l("./") != 64) {is_fine = 0; printf("1 is not okay !\n");}
    if (a64l("") != 0) {is_fine = 0; printf("2 is not okay !\n");}
    if (a64l("/") != 1) {is_fine = 0; printf("3 is not okay !\n");}
    if (a64l("FT") != 2001) {is_fine = 0; printf("4 is not okay !\n");}
    if (a64l("zzzzz1") != (long int) 0xffffffff) {is_fine = 0; printf("5 is not okay !\n");}
    if (a64l("zzzz1") != 0x3ffffff) {is_fine = 0; printf("6 is not okay !\n");}
    if (a64l("zzz1") != 0xfffff) {is_fine = 0; printf("7 is not okay !\n");}
    if (a64l("zz1") != 0x3fff) {is_fine = 0; printf("8 is not okay !\n");}
    if (a64l("z1") != 0xff) {is_fine = 0; printf("9 is not okay !\n");}
    if (a64l("1") != 0x3) {is_fine = 0; printf("10 is not okay !\n");}
    print_state(is_fine, "a64l");

    // test of abort
    #if TEST_ABORT
    abort();
    printf("This line should not be printed or else abort isnt working !\n");
    #endif
    printf("$1abort$7: $1NOT OK (cant be tested)\n");

    // test of abs
    is_fine = 1;
    if (abs(10) != 10) is_fine = 0;
    if (abs(-10) != 10) is_fine = 0;
    print_state(is_fine, "abs");

    // test of atexit
    #if TEST_ATEXIT
    is_fine = 1;
    if (atexit(0) != 0) is_fine = 0;
    print_state(is_fine, "atexit");
    #else
    printf("$1atexit$7: $1NOT OK (not implemented)\n");
    #endif

    // test of atof
    #if TEST_ATOF
    is_fine = 1;
    if (atof("10.0") != 10.0) is_fine = 0;
    if (atof("-10.0") != -10.0) is_fine = 0;
    printf("atof: %f\n", atof("10.0"));
    printf("atof: %f\n", atof("-10.0"));
    print_state(is_fine, "atof");
    #else
    printf("$1atof$7: $1NOT OK (not implemented)\n");
    #endif

    // test of atoi
    is_fine = 1;
    if (atoi("10") != 10) {is_fine = 0; printf("%d\n", atoi("10"));}
    if (atoi("-10") != -10) {is_fine = 0; printf("%d\n", atoi("-10"));}
    print_state(is_fine, "atoi");

    // test of itoa
    is_fine = 1;
    char *itoa_test = malloc(10);
    itoa(10, itoa_test, 10);
    if (strcmp(itoa_test, "10") != 0) is_fine = 0;
    itoa(-10, itoa_test, 10);
    if (strcmp(itoa_test, "-10") != 0) is_fine = 0;
    free(itoa_test);
    print_state(is_fine, "itoa");

    printf("Testing stdlib.h: $1OK$7\n");
    printf("Testing string.h\n");

    // test of basename
    is_fine = 1;
    if (strcmp(basename("test1/test2/test3"), "test3") != 0) is_fine = 0;
    if (strcmp(basename("test1/test2/test3/"), "") != 0) is_fine = 0;
    if (strcmp(basename("test1/test2/test3//"), "") != 0) is_fine = 0;
    if (strcmp(basename("/a"), "a") != 0) is_fine = 0;
    print_state(is_fine, "basename");

    printf("Testing string.h: $1OK$7\n");

    printf("Il n'y a pas de test pour printf, mais si il ne marchais pas, l'os ne montrerais rien !\n");
    fprintf(stderr, "print on stderr\n");
    fprintf(stdout, "print on stdout\n");
    fprintf(stdin, "print on stdin, THIS MESSAGE SHOULD NOT BE SEEN\n");    

    printf("Testing math.h\n");

    printf("Should be yn : ");
    yn(1, 1);

    // test of acos
    is_fine = 1;
    if (acos(0) != 1.5707963267948966192313216916398) is_fine = 0;
    if (acos(1) != 0) is_fine = 0;
    print_state(is_fine, "acos");

    // test of acosf
    is_fine = 1;
    if (abs(acosf(0)-1.570790) > 0.0001) is_fine = 0;
    if (acosf(1) != 0) is_fine = 0;
    print_state(is_fine, "acosf");

    // test of tanf
    is_fine = 1;
    if (tanf(0) != 0) is_fine = 0;
    if (abs(tanf(1)-1.55740) > 0.0001) is_fine = 0; // we dont test the exact value, osef
    print_state(is_fine, "tanf");

    // test of cosf
    is_fine = 1;
    if (cosf(0) != 1) is_fine = 0;
    if (abs(cosf(1)-0.54030) > 0.0001) is_fine = 0; // we dont test the exact value, osef
    print_state(is_fine, "cosf");

    // test of sinf
    is_fine = 1;
    if (sinf(0) != 0) is_fine = 0;
    if (abs(sinf(1)-0.84147) > 0.0001) is_fine = 0; // we dont test the exact value, osef
    print_state(is_fine, "sinf");

    // test of log10
    is_fine = 1;
    if (log10(1) != 0) is_fine = 0;
    if (abs(log10(10)-1) > 0.0001) is_fine = 0; // we dont test the exact value, osef
    print_state(is_fine, "log10");

    printf("End of the test for now !\n");

    return 0;
}

void print_state(int is_fine, char* name) {
    if (is_fine) {
        printf("$1%s$7: $1OK$7\n", name);
    } else {
        printf("$3%s$7: $3NOT OK$7\n", name);
    }
}