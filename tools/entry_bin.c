// standard entry point for binary compiled files

extern int main(int argc, char **argv, char **envp);

int entry(int argc, char **argv, char **envp) {
    return main(argc, argv, envp);
}
