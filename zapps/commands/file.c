#include <stdlib.h>
#include <string.h>
#include <stdio.h>

int main(int argc, char **argv) {
    FILE *f = fopen("/user/test.txt", "w");

    if (f == NULL) {
        printf("Error opening file!");
    }

    char *text = "This is some text to write to the file.\n";
    fwrite(text, 1, strlen(text), f);
    fclose(f);

    f = fopen("/user/test.txt", "r");
    char buffer[100];
    fread(buffer, 1, 100, f);
    printf("%s", buffer);
    fclose(f);

    return 0;
}