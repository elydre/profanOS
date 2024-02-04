#include <filesys.h>
#include <syscall.h>
#include <profan.h>

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

int main(int argc, char **argv) {
    /*if (argc != 3 && argc != 4) {
        puts("Usage: ubr <link> <redirection> [pid]");
        return 1;
    }

    char *pwd = getenv("PWD");

    char *link = malloc(strlen(pwd) + strlen(argv[1]) + 2);
    assemble_path(pwd, argv[1], link);

    char *redirection = malloc(strlen(pwd) + strlen(argv[2]) + 2);
    assemble_path(pwd, argv[2], redirection);

    sid_t link_sid = fu_path_to_sid(ROOT_SID, link);
    sid_t redirection_sid = fu_path_to_sid(ROOT_SID, redirection);

    int pid;
    if (argc == 4) {
        pid = atoi(argv[3]);
    } else {
        pid = c_process_get_ppid(c_process_get_pid());
    }

    if (IS_NULL_SID(link_sid) || !fu_is_link(link_sid)) {
        puts("Failed to get link sid");
    } else if (IS_NULL_SID(redirection_sid)) {
        puts("Failed to get redirection sid");
    } else if (!(fu_is_file(redirection_sid) || fu_is_fctf(redirection_sid))) {
        puts("Redirection must be a file or a fctf");
    } else if (devio_set_redirection(link_sid, redirection, pid)) {
        puts("Failed to change redirection");
    } else {
        free(redirection);
        free(link);
        return 0;
    }

    free(redirection);
    free(link);
    return 1;
    */
    puts("oui");
    return 0;
}
