/*****************************************************************************\
|   === entry_bin.c : 2024 ===                                                |
|                                                                             |
|    Standard entry point for binary compiled files                .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

extern int main(int argc, char **argv, char **envp);

int _start(int argc, char **argv, char **envp) {
    return main(argc, argv, envp);
}
