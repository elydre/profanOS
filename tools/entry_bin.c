/****** This file is part of profanOS **************************\
|   == entry_bin.c ==                                .pi0iq.    |
|                                                   d"  . `'b   |
|   Standard entry point for binary compiled        q. /|\ .p   |
|   files                                            '// \\'    |
|                                                    //   \\    |
|   [ github.com/elydre/profanOS - GPLv3 ]          //     \\   |
\***************************************************************/

extern int main(int argc, char **argv, char **envp);

int entry(int argc, char **argv, char **envp) {
    return main(argc, argv, envp);
}
