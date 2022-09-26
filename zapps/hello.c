int start(int addr, int arg) {
    int (*get_func)(int id) = (int (*)(int)) addr;
    void (*rainbow_print)(char msg[]) = (void (*)(char *)) get_func(39);
    rainbow_print("Hello world!\n");
    return arg;
}
