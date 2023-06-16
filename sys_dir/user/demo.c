void main();

int entry(char argc, char **argv) {
    main();
    return 0;
}

void main() {
    ((int (*)(const char *, ...)) ((int (*)(int, int)) *(int *) 0x1ffffb)(1009, 38))("Hello World!\n");
}
